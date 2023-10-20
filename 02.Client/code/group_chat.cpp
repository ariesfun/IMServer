#include "group_chat.h"
#include "ui_group_chat.h"
#include <QJsonDocument>
#include <QMessageBox>

GroupChat::GroupChat(QTcpSocket* socket, QString groupname, QString username, ChatList* chatlist, QList<GroupChatWidgetInfo>* groupchatWidgetList, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GroupChat)
{
    ui->setupUi(this);
    m_socket = socket;
    m_groupname = groupname;
    m_username = username;
    m_chatlist_widget = chatlist;
    m_groupchatWidgetList = groupchatWidgetList;

    QJsonObject obj;
    obj.insert("cmd", "get_groupmember");
    obj.insert("user", m_username);
    obj.insert("group", m_groupname);
    QByteArray byte = QJsonDocument(obj).toJson();                          // 发送请求获取具体群聊成员
    socket->write(byte);

    connect(m_chatlist_widget, &ChatList::signal_to_groupchat_getmember_widget, this, &GroupChat::show_group_member);
    connect(m_chatlist_widget, &ChatList::signal_to_groupchat_sendmsg_widget, this, &GroupChat::show_group_message);
}

GroupChat::~GroupChat()
{
    delete ui;
}

void GroupChat::show_group_member(QJsonObject &obj)
{
    QString cmd = obj.value("cmd").toString();
    QString groupname = obj.value("group").toString();
    if(cmd == "get_groupmember_reply" && groupname == m_groupname) {       // 处理传递来消息，当时是获取群成员回复并且是当前窗口所在的群聊时，才进行处理
        QString groupMemberList = obj.value("result").toString();
        QStringList memberlist = groupMemberList.split('|');
        for(int i = 0; i < memberlist.size(); i++) {                       // 加群群成员信息
            if(memberlist.at(i) != "") {
                if(i == 0) {
                    QString groupOwner = QString("%1 (群主)").arg(memberlist.at(i));
                    ui->groupMember_listWidget->addItem(groupOwner);        // 显示群主Title,是该群的第一个成员
                }
                else {
                    ui->groupMember_listWidget->addItem(memberlist.at(i));
                }
            }
        }
    }
}

void GroupChat::show_group_message(QJsonObject &obj)
{
    QString cmd = obj.value("cmd").toString();
    QString groupname = obj.value("group").toString();
    if(cmd == "group_chat" && groupname == m_groupname) {          // 处理服务器转发来的群聊消息，并且是当前窗口所在的群聊时要显示内容在群聊窗口中
        QString send_friendname = obj.value("user").toString();
        QString sendmessage = obj.value("message").toString();     // 获得发送方的具体信息（人及消息）
        QString message = QString("[%1]: %2\n").arg(send_friendname).arg(sendmessage);
        ui->groupMsg_textEdit->append(message);
        if(this->isMinimized()) {                                  // 收到消息后，该窗户应该显示在页面最前(保持活跃状态)
            this->showNormal();
        }
        this->activateWindow();
    }
}

void GroupChat::on_sendMsgButton_clicked()
{
    QString sendmessage = ui->userMsg_lineEdit->text();
    if(sendmessage.isEmpty()) {
        QMessageBox::warning(this, "群聊提示", "发送的消息不能为空！");
    }
    else {
        QJsonObject obj;                                                    // 向服务器发送群聊消息
        obj.insert("cmd", "group_chat");
        obj.insert("user", m_username);
        obj.insert("group", m_groupname);
        obj.insert("message", sendmessage);
        QByteArray byte = QJsonDocument(obj).toJson();

        ui->userMsg_lineEdit->clear();                                      // 清空输入框的数据，将消息添加至聊天框
        QString message = QString("[%1]: %2\n").arg(m_username).arg(sendmessage);
        ui->groupMsg_textEdit->append(message);
        m_socket->write(byte);
    }
}


void GroupChat::closeEvent(QCloseEvent *event)
{
    for(int i = 0; i < m_groupchatWidgetList->size(); i++) {
        if(m_groupchatWidgetList->at(i).group_name == m_groupname) {
            m_groupchatWidgetList->removeAt(i);                         // 从打开的群聊窗口中移除当前要关闭的对象
            break;
        }
    }
    event->accept();
}


