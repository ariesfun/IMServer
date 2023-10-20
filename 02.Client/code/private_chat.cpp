#include "private_chat.h"
#include "ui_private_chat.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QFileDialog>
#include <QCoreApplication>
#include <QFile>

PrivateChat::PrivateChat(QTcpSocket* socket, QString username, QString friendname, ChatList* chatlist, QList<ChatWidgetInfo>* privatechatWidgetList, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
    m_socket = socket;
    m_username = username;
    m_friendname = friendname;
    m_chatlist_widget = chatlist;
    m_privatechat_WidgetList = privatechatWidgetList;               // 得到当前用户打开的私聊窗口链表信息

                                                                    // 将当前的好友列表窗口与私聊窗口绑定，以执行槽函数
    connect(m_chatlist_widget, &ChatList::signal_to_friendchat_widget, this, &PrivateChat::show_friend_messge);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

void PrivateChat::on_sendMsgBtn_clicked()
{
    QString sendmessage = ui->userMsg_lineEdit->text();
    if(sendmessage.isEmpty()) {
        QMessageBox::warning(this, "私聊提示", "发送的消息不能为空！");
    }
    else {
        QJsonObject obj;                                            // 向服务器发送好友私聊请求
        obj.insert("cmd", "private_chat");
        obj.insert("user_from", m_username);
        obj.insert("user_to", m_friendname);
        obj.insert("message", sendmessage);
        QByteArray byte = QJsonDocument(obj).toJson();

        ui->userMsg_lineEdit->clear();                              // 清空输入框的数据，将消息添加至聊天框
        QString message = QString("[%1]: %2\n").arg(m_username).arg(sendmessage);
        ui->chatContent_textEdit->append(message);
        m_socket->write(byte);
    }
}

void PrivateChat::show_friend_messge(QJsonObject &obj)
{
    QString cmd = obj.value("cmd").toString();
    QString to_username = obj.value("user_to").toString();
    if(cmd == "private_chat" && to_username == m_username) {       // 处理服务器转发来的私聊消息，且要显示内容在与发送方的聊天窗口中
        QString send_friendname = obj.value("user_from").toString();
        QString sendmessage = obj.value("message").toString();     // 获得发送方的具体信息（人及消息）
        QString message = QString("[%1]: %2\n").arg(send_friendname).arg(sendmessage);
        ui->chatContent_textEdit->append(message);
        if(this->isMinimized()) {                                  // 收到消息后，该窗户应该显示在页面最前(保持活跃状态)
            this->showNormal();
        }
        this->activateWindow();
    }
}

void PrivateChat::closeEvent(QCloseEvent *event)
{
    for(int i = 0; i < m_privatechat_WidgetList->size(); i++) {
        if(m_privatechat_WidgetList->at(i).friend_name == m_friendname) {
            m_privatechat_WidgetList->removeAt(i);                  // 从打开的聊天窗口中移除当前要关闭的对象
            break;
         }
    }
    event->accept();
}

void PrivateChat::on_sendFileBtn_clicked()
{
    QString local_filepath = QFileDialog::getOpenFileName(this, "发送文件", QCoreApplication::applicationFilePath());
    if(local_filepath.isEmpty()) {
         QMessageBox::warning(this, "发送文件提示", "请选择你要发送的文件！");
         return;
    }

    QFile file(local_filepath);
    file.open(QIODevice::ReadOnly);
    int filesize = file.size();                                              // 获得文件大小

    QJsonObject obj;
    obj.insert("cmd", "send_file");
    obj.insert("user_from", m_username);
    obj.insert("user_to", m_friendname);
    obj.insert("file_pathname", local_filepath);
    obj.insert("file_size", filesize);
    QByteArray byte = QJsonDocument(obj).toJson();
    m_socket->write(byte);
}

