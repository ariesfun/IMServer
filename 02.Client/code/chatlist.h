#ifndef CHATLIST_H
#define CHATLIST_H

class PrivateChat;              // 头文件重复包含的声明
#include "private_chat.h"
class GroupChat;
#include "group_chat.h"
#include <QWidget>
#include <QTcpSocket>
#include <QJsonObject>
#include <QList>
#include <QCloseEvent>

namespace Ui {
class ChatList;
}

struct ChatWidgetInfo
{
    PrivateChat* prichat_widget;
    QString friend_name;
};

struct GroupChatWidgetInfo
{
    GroupChat* groupchat_widget;
    QString group_name;
};

class ChatList : public QWidget {
    Q_OBJECT

public:
    explicit ChatList(QTcpSocket* socket, QString username, QString friends, QString groups, QWidget* parent = nullptr);
    ~ChatList();

public:
    void closeEvent(QCloseEvent* event);

private slots:
    void server_reply();
    void on_addFriendBtn_clicked();
    void on_createGroupBtn_clicked();
    void on_addGroupBtn_clicked();
    void on_friendList_double_clicked();
    void on_groupList_double_clicked();

signals:
    void signal_to_friendchat_widget(QJsonObject &obj);
    void signal_to_groupchat_getmember_widget(QJsonObject &obj);
    void signal_to_groupchat_sendmsg_widget(QJsonObject &obj);
    void signal_to_recvfile_working_func();

private:
    void client_login_notice_reply(QString friendname);
    void client_add_friend_reply(QJsonObject &obj);
    void client_create_group_reply(QJsonObject &obj);
    void client_add_group_reply(QJsonObject &obj);
    void client_private_chat_reply(QString res);
    void client_chat_reply(QJsonObject &obj);
    void client_get_groupmember_reply(QJsonObject &obj);
    void client_groupchat_reply(QJsonObject &obj);
    void client_sendfile_reply(QString res);
    void client_sendfile_port_reply(QJsonObject &obj);
    void client_recvfile_port_reply(QJsonObject &obj);
    void client_friend_offline_reply(QString username);

private:
    Ui::ChatList* ui;
    QTcpSocket* m_socket;
    QString m_username;
    QList<ChatWidgetInfo> privatechatWidgetList;            // 保存当前用户的所有私聊窗口信息链表
    QList<GroupChatWidgetInfo> groupchatWidgetList;         // 保存当前用户的所有群聊窗口信息链表

};

#endif // CHATLIST_H
