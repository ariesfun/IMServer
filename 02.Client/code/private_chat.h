#ifndef PRIVATE_CHAT_H
#define PRIVATE_CHAT_H

class ChatList;             // 头文件重复包含（互相包含）的声明
struct ChatWidgetInfo;
#include "chatlist.h"
#include <QWidget>
#include <QTcpSocket>
#include <QJsonObject>
#include <QList>
#include <QCloseEvent>

namespace Ui {
class PrivateChat;
}

class PrivateChat : public QWidget {
    Q_OBJECT

public:
    explicit PrivateChat(QTcpSocket* socket, QString username, QString friendname, ChatList* chatlist, QList<ChatWidgetInfo>* privatechatWidgetList, QWidget *parent = nullptr);
    ~PrivateChat();

public:
    void closeEvent(QCloseEvent* event);            // 处理关闭窗口的事件

private slots:
    void on_sendMsgBtn_clicked();
    void show_friend_messge(QJsonObject &obj);

    void on_sendFileBtn_clicked();

private:
    Ui::PrivateChat *ui;
    QTcpSocket* m_socket;
    QString m_username;
    QString m_friendname;
    ChatList* m_chatlist_widget;
    QList<ChatWidgetInfo>* m_privatechat_WidgetList;

};

#endif // PRIVATE_CHAT_H
