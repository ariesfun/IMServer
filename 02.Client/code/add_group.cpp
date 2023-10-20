#include "add_group.h"
#include "ui_add_group.h"
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

AddGroup::AddGroup(QTcpSocket* socket, QString username, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddGroup)
{
    ui->setupUi(this);
    m_socket = socket;
    m_username = username;
}

AddGroup::~AddGroup()
{
    delete ui;
}

void AddGroup::on_cancelBtn_clicked()
{
    this->close();
}


void AddGroup::on_addGroupBtn_clicked()
{
    QString groupname = ui->searchGroup_lineEdit->text();
    if(groupname.isEmpty()) {
        QMessageBox::warning(this, "添加群聊提示", "群聊名称不能为空！");
    }
    else {
        QJsonObject obj;                        // 向服务器发送添加群聊请求
        obj.insert("cmd", "add_group");
        obj.insert("user", m_username);
        obj.insert("group", groupname);
        QByteArray byte = QJsonDocument(obj).toJson();
        m_socket->write(byte);
        this->close();
    }
}

