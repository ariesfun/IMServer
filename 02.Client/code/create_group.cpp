#include "create_group.h"
#include "ui_create_group.h"
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

CreateGroup::CreateGroup(QTcpSocket* socket, QString username, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CreateGroup)
{
    ui->setupUi(this);
    m_socket = socket;
    m_username = username;
}

CreateGroup::~CreateGroup()
{
    delete ui;
}

void CreateGroup::on_cancelBtn_clicked()
{
    this->close();
}


void CreateGroup::on_createGroupBtn_clicked()
{
    QString newGroupName = ui->newGroup_lineEdit->text();
    if(newGroupName.isEmpty()) {
        QMessageBox::warning(this, "创建群聊提示", "群聊名称不能为空！");
    }
    else {
        QJsonObject obj;                        // 向服务器发送创建群聊请求
        obj.insert("cmd", "create_group");
        obj.insert("user", m_username);
        obj.insert("group", newGroupName);
        QByteArray byte = QJsonDocument(obj).toJson();
        m_socket->write(byte);
        this->close();
    }
}

