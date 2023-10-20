#ifndef GROUP_CHAT_H
#define GROUP_CHAT_H

class ChatList;             // 头文件重复包含（互相包含）的声明
struct GroupChatWidgetInfo;
#include "chatlist.h"
#include <QWidget>
#include <QTcpSocket>
#include <QJsonObject>
#include <QList>
#include <QCloseEvent>

namespace Ui {
class GroupChat;
}

class GroupChat : public QWidget {
    Q_OBJECT

public:
    explicit GroupChat(QTcpSocket* socket, QString groupname, QString username, ChatList* chatlist, QList<GroupChatWidgetInfo>* groupchatWidgetList, QWidget *parent = nullptr);
    ~GroupChat();

public:
    void closeEvent(QCloseEvent* event);                // 处理关闭窗口事件

private slots:
    void show_group_member(QJsonObject &obj);
    void show_group_message(QJsonObject &obj);
    void on_sendMsgButton_clicked();

private:
    Ui::GroupChat* ui;
    QTcpSocket* m_socket;
    QString m_groupname;
    QString m_username;                                 // 当前用户
    ChatList* m_chatlist_widget;                        // 当前用户聊天列表的主窗口
    QList<GroupChatWidgetInfo>* m_groupchatWidgetList;  // 当前用户打开的所有群聊窗口列表

};

#endif // GROUP_CHAT_H
