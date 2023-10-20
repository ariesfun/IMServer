#include "chatlist.h"
#include "add_friend.h"
#include "create_group.h"
#include "add_group.h"
#include "sendfile_thread.h"
// #include "recvfile_thread.h"
#include "recvfile.h"
#include "ui_chatlist.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QThread>

ChatList::ChatList(QTcpSocket* socket, QString username, QString friends, QString groups, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatList)
{
    ui->setupUi(this);
    m_socket = socket;
    m_username = username;

    QStringList friendlist = friends.split('|');                                                                // 当打开好友列表后，将收到的好友及群组信息显示在列表上
    for(int i = 0; i < friendlist.size(); i++) {
        if(friendlist.at(i) != "") {
            ui->FriendList->addItem(friendlist.at(i));
        }
    }
    QStringList grouplist = groups.split('|');
    for(int i = 0; i < grouplist.size(); i++) {
        if(grouplist.at(i) != "") {
            ui->GroupList->addItem(grouplist.at(i));
        }
    }

    connect(m_socket, &QTcpSocket::readyRead, this, &ChatList::server_reply);                                   // 建立信号与槽，时刻准备接收服务器发来的消息
    connect(ui->FriendList, &QListWidget::itemDoubleClicked, this, ChatList::on_friendList_double_clicked);     // 用户双击好友列表事件，进行私聊
    connect(ui->GroupList, &QListWidget::itemDoubleClicked, this, ChatList::on_groupList_double_clicked);       // 用户双击群聊列表事件，打开群聊
}

ChatList::~ChatList()
{
    delete ui;
}

void ChatList::closeEvent(QCloseEvent* event)                               // 处理客户端下线
{
    QJsonObject obj;
    obj.insert("cmd", "offline");
    obj.insert("user", m_username);
    QByteArray byte = QJsonDocument(obj).toJson();
    m_socket->write(byte);
    m_socket->flush();
    event->accept();
}

void ChatList::server_reply()                                               // 客户端统一接收所有服务器的回复
{
    QByteArray byte = m_socket->readAll();
    QJsonObject obj = QJsonDocument::fromJson(byte).object();
    QString cmd = obj.value("cmd").toString();
    if(cmd == "friend_online_reply") {                                      // 好友上线提示
        QString friendname = obj.value("friend").toString();
        client_login_notice_reply(friendname);
    }
    else if(cmd == "add_friend_reply") {
        client_add_friend_reply(obj);                                       // 传递JSON数据对象
    }
    else if(cmd == "create_group_reply") {
        client_create_group_reply(obj);
    }
    else if(cmd == "add_group_reply") {
        client_add_group_reply(obj);
    }
    else if(cmd == "private_chat_reply") {
        QString res = obj.value("result").toString();
        client_private_chat_reply(res);
    }
    else if(cmd == "private_chat") {                                        // 处理服务器转发私聊消息给接收方
        client_chat_reply(obj);
    }
    else if(cmd == "get_groupmember_reply") {
        client_get_groupmember_reply(obj);
    }
    else if(cmd == "group_chat") {
        client_groupchat_reply(obj);                                        // 处理服务器转发群聊消息给接收方
    }
    else if(cmd == "send_file_reply") {
        QString res = obj.value("result").toString();
        client_sendfile_reply(res);
    }
    else if(cmd == "sendfile_port_reply") {                                 // 主要用于接受服务器返回的文件信息及发送的端口
        client_sendfile_port_reply(obj);                                    // 发送启动线程，发送文件
    }
    else if(cmd == "recvfile_port_reply") {
        client_recvfile_port_reply(obj);                                    // 接收方启动线程，接收文件
    }
    else if(cmd == "friend_offline_reply") {                                // 好友下线提醒
        QString username = obj.value("user").toString();
        client_friend_offline_reply(username);
    }

}

void ChatList::client_login_notice_reply(QString friendname)
{
    QString str = QString("好友%1，上线了！").arg(friendname);
    QMessageBox::information(this, "好友上线提示", str);
}

void ChatList::client_add_friend_reply(QJsonObject &obj)
{
    QString res = obj.value("result").toString();
    if(res == "user_not_exist") {
        QMessageBox::warning(this, "添加好友提示", "该用户不存在");
    }
    else if(res == "already_is_friend") {
        QMessageBox::warning(this, "添加好友提示", "已经是好友了");
    }
    else if(res == "success") {                                                 // 添加好友成功双方都需要回复
        QString to_add_friendname = obj.value("friend").toString();             // 更新用户好友列表信息(添加方)
        if(!to_add_friendname.isEmpty()) {
            ui->FriendList->addItem(to_add_friendname);
            QString str = QString("好友%1添加成功!").arg(to_add_friendname);
            QMessageBox::information(this, "添加好友提示", str);
        }
        QString new_friendname = obj.value("new_friend").toString();            // 更新用户好友列表信息(被添加方)
        if(!new_friendname.isEmpty()) {
            ui->FriendList->addItem(new_friendname);
            QString str = QString("%1已经添加你为好友!").arg(new_friendname);
            QMessageBox::information(this, "添加好友提示", str);
        }
    }
}

void ChatList::client_create_group_reply(QJsonObject &obj)
{
    QString res = obj.value("result").toString();
    if(res == "group_already_exist") {
        QMessageBox::warning(this, "创建群聊提示", "该群聊已经存在了！");
    }
    else if(res == "success") {                                                 // 添加好友成功双方都需要回复
        QString new_groupname = obj.value("new_group").toString();              // 更新用户群聊信息(添加方)
        ui->GroupList->addItem(new_groupname);
        QString str = QString("群聊: %1，创建成功!").arg(new_groupname);
        QMessageBox::information(this, "创建群聊提示", str);

    }
}

void ChatList::client_add_group_reply(QJsonObject &obj)
{
    QString res = obj.value("result").toString();
    if(res == "group_not_exist") {
        QMessageBox::warning(this, "添加群聊提示", "该群聊不存在！");
    }
    else if(res == "user_in_group") {
        QMessageBox::warning(this, "添加群聊提示", "你已经是该群聊成员了！");
    }
    else if(res == "success") {                                                 // 添加好友成功双方都需要回复
        QString added_groupname = obj.value("added_group").toString();          // 更新用户群聊信息(添加方)
        ui->GroupList->addItem(added_groupname);
        QString str = QString("群聊: %1，添加成功!").arg(added_groupname);
        QMessageBox::information(this, "添加群聊提示", str);

    }
}

void ChatList::client_private_chat_reply(QString res)
{
    if(res == "user_to_offline") {
        QMessageBox::warning(this, "好友私聊提示", "好友当前不在线！");
    }
}

void ChatList::client_chat_reply(QJsonObject &obj)
{
    QString friendname = obj.value("user_from").toString();
    bool flag = false;
    for(int i = 0; i < privatechatWidgetList.size(); i++) {                 // 遍历当前用户打开的私聊窗口，判断是否需要打开与发送方的窗口
        if(privatechatWidgetList.at(i).friend_name == friendname) {
            flag = true;
            break;
        }
    }
    if(!flag) {                                                             // 没打开时，就需要打开一个聊天的窗口
        PrivateChat* privateChatWidget = new PrivateChat(m_socket, m_username, friendname, this, &privatechatWidgetList);
        QString str = QString("好友: %1").arg(friendname);
        privateChatWidget->setWindowTitle(str);
        privateChatWidget->show();
        ChatWidgetInfo chat_widget = {privateChatWidget, friendname};
        privatechatWidgetList.push_back(chat_widget);
    }

    emit signal_to_friendchat_widget(obj);                                  // 接着接收方要处理显示接收到的消息，在私聊窗口里显示
}

void ChatList::client_get_groupmember_reply(QJsonObject &obj)
{
    emit signal_to_groupchat_getmember_widget(obj);                         // 群聊窗口双击时就打开了(发送了请求)，将收到的获取群成员消息传递到群聊窗口
}

void ChatList::client_groupchat_reply(QJsonObject &obj)
{
    QString groupname = obj.value("group").toString();
    bool flag = false;
    for(int i = 0; i < groupchatWidgetList.size(); i++) {                   // 遍历当前用户打开的群聊窗口，判断是否需要打开群聊窗口
        if(groupchatWidgetList.at(i).group_name == groupname) {
            flag = true;
            break;
        }
    }
    if(!flag) {                                                             // 没打开时，就需要打开一个聊天的窗口
        GroupChat* groupChatWidget = new GroupChat(m_socket, groupname, m_username, this, &groupchatWidgetList);
        QString str = QString("群聊: %1").arg(groupname);
        groupChatWidget->setWindowTitle(str);
        groupChatWidget->show();

        GroupChatWidgetInfo groupchat_widget = {groupChatWidget, groupname};
        groupchatWidgetList.push_back(groupchat_widget);
    }

    emit signal_to_groupchat_sendmsg_widget(obj);
}

void ChatList::client_sendfile_reply(QString res)
{
    if(res == "user_to_offline") {
        QMessageBox::warning(this, "发送文件提示", "好友不在线！");
    }
    else if(res == "timeout") {
        QMessageBox::warning(this, "发送文件提示", "连接文件服务器超时！");
    }
    else if(res == "success") {
        QMessageBox::information(this, "发送文件提示", "文件发送成功！");
    }
}

void ChatList::client_sendfile_port_reply(QJsonObject &obj)
{
    SendFileThread* sendfile_th = new SendFileThread(obj);      // 这时发送方客户端需要开启一个线程
    sendfile_th->start();                                       // 开启线程，需要重写run()函数
}

void ChatList::client_recvfile_port_reply(QJsonObject &obj)
{
    QThread* sub_th = new QThread;                                          // 创建子线程, 工作对象
    RecvFile* recvfile = new RecvFile(obj);
                                                                            // 让子线程来完成接收文件的操作
    recvfile->moveToThread(sub_th);                                         // 将工作对象移动到子线程中
    sub_th->start();                                                        // 启动线程
                                                                            // 通过信号，启动接收文件的工作函数
    connect(this, &ChatList::signal_to_recvfile_working_func, recvfile, &RecvFile::working);
    emit signal_to_recvfile_working_func();                                 // 发送信号

    connect(recvfile, &RecvFile::recv_connect_timeout, this, [=]() {        // 处理子线程的工作对象发来的信号
        QMessageBox::warning(this, "接收文件提示", "接收子线程连接文件服务器超时！");
        sub_th->quit();
        sub_th->wait();
        sub_th->deleteLater();
        recvfile->deleteLater();                                            // 释放资源
    });
    connect(recvfile, &RecvFile::recv_file_success, this, [=]() {           // 处理子线程的工作对象发来的信号
        QMessageBox::information(this, "接收文件提示", "文件接收完成！");
        sub_th->quit();
        sub_th->wait();
        sub_th->deleteLater();
        recvfile->deleteLater();
    });
}

void ChatList::client_friend_offline_reply(QString username)
{
    QString str =  QString("好友：%1下线了").arg(username);
    QMessageBox::information(this, "好友下线提醒", str);
}

void ChatList::on_addFriendBtn_clicked()
{
    AddFriend* addFriendWidget = new AddFriend(m_socket, m_username);
    addFriendWidget->setWindowTitle("添加好友");
    addFriendWidget->show();
}

void ChatList::on_createGroupBtn_clicked()
{
    CreateGroup* createGroupWidget = new CreateGroup(m_socket, m_username);
    createGroupWidget->setWindowTitle("创建群聊");
    createGroupWidget->show();
}

void ChatList::on_addGroupBtn_clicked()
{
    AddGroup* addGroupWidget = new AddGroup(m_socket, m_username);
    addGroupWidget->setWindowTitle("添加群聊");
    addGroupWidget->show();
}

void ChatList::on_friendList_double_clicked()
{
    QString to_friendname = ui->FriendList->currentItem()->text();          // 同时将当前的好友主窗口对象（chatlist_widget）, 传给私聊窗口(用于打开窗口，并显示消息)
    PrivateChat* privateChatWidget = new PrivateChat(m_socket, m_username, to_friendname, this, &privatechatWidgetList);
    QString str = QString("好友: %1").arg(to_friendname);                   // 传递当前已经打开的privatechatWidget链表对象，便于后续关掉窗口时从链表里移除
    privateChatWidget->setWindowTitle(str);
    privateChatWidget->show();

    ChatWidgetInfo chat_widget = {privateChatWidget, to_friendname};
    privatechatWidgetList.push_back(chat_widget);
}

void ChatList::on_groupList_double_clicked()
{
    QString groupname = ui->GroupList->currentItem()->text();               // 同时将当前的好友主窗口对象ChatList类，用于绑定群聊窗口，以执行相应的槽函数
    GroupChat* groupChatWidget = new GroupChat(m_socket, groupname, m_username, this, &groupchatWidgetList);
    QString str = QString("群聊: %1").arg(groupname);
    groupChatWidget->setWindowTitle(str);
    groupChatWidget->show();

    GroupChatWidgetInfo groupchat_widget = {groupChatWidget, groupname};
    groupchatWidgetList.push_back(groupchat_widget);
}

