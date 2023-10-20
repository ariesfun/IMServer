#include "add_friend.h"
#include "ui_add_friend.h"
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

AddFriend::AddFriend(QTcpSocket* socket, QString username, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddFriend)
{
    ui->setupUi(this);
    m_socket = socket;
    m_username = username;
}

AddFriend::~AddFriend()
{
    delete ui;
}

void AddFriend::on_cancelBtn_clicked()
{
    this->close();          // 关掉该窗口
}


void AddFriend::on_addFriendBtn_clicked()
{
    QString friendname = ui->search_lineEdit->text();
    if(friendname.isEmpty()) {
        QMessageBox::warning(this, "添加好友提示", "好友名不能为空！");
    }
    else {
        QJsonObject obj;                        // 向服务器发送添加好友请求
        obj.insert("cmd", "add_friend");
        obj.insert("user", m_username);
        obj.insert("friend", friendname);
        QByteArray byte = QJsonDocument(obj).toJson();
        m_socket->write(byte);
        this->close();
    }
}

