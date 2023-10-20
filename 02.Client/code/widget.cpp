#include "widget.h"
#include "ui_widget.h"
#include "chatlist.h"
#include <QHostAddress>
#include <QMessageBox>
#include <QJsonObject>      // 创建JSON对象
#include <QJsonDocument>    // 解析JSON对象
#include <QNetworkProxy>


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    socket = new QTcpSocket;
    socket->setProxy(QNetworkProxy::NoProxy);                                   // 默认不使用系统的代理模式

    connect(socket, &QTcpSocket::errorOccurred, this, &Widget::connect_error);  // 处理网络连接错误的信号
    connect(socket, &QTcpSocket::connected, this, &Widget::connect_success);    // 执行槽函数
    connect(socket, &QTcpSocket::readyRead, this, &Widget::server_reply);

    socket->connectToHost(QHostAddress("47.113.226.70"), 1111);                // 这里是公网IP地址
}

Widget::~Widget()
{
    delete ui;
}

void Widget::connect_error(QAbstractSocket::SocketError error)
{
    QString errorString = socket->errorString();                                // 获取错误原因
    if (error == QAbstractSocket::HostNotFoundError) {                          // 根据错误类型显示不同的错误消息
        QMessageBox::critical(this, "连接错误", "主机未找到，请检查服务器地址和端口！");
    } else if (error == QAbstractSocket::ConnectionRefusedError) {
        QMessageBox::critical(this, "连接错误", "连接被服务器拒绝，请确保服务器正常运行！");
    } else {
        QMessageBox::critical(this, "连接错误", "连接出错： " + errorString);
    }
}

void Widget::connect_success()
{
    QMessageBox::information(this, "连接提示", "连接服务器成功！");
}

void Widget::on_registerBtn_clicked()
{
    QString username = ui->user_lineEdit->text();
    QString password = ui->password_lineEdit->text();
    if(username.isEmpty() || password.isEmpty()) {
        QMessageBox::information(this, "注册提示", "用户名和密码不能为空！");
    }

    if(!username.isEmpty() && !password.isEmpty()) {
        QJsonObject obj;
        obj.insert("cmd", "register");
        obj.insert("user", username);
        obj.insert("password", password);
        QByteArray byte = QJsonDocument(obj).toJson();
        socket->write(byte);
    }
}

void Widget::on_loginBtn_clicked()
{
    QString username = ui->user_lineEdit->text();
    QString password = ui->password_lineEdit->text();
    if(username.isEmpty() || password.isEmpty()) {
        QMessageBox::information(this, "登录提示", "用户名和密码不能为空！");
    }

    if(!username.isEmpty() && !password.isEmpty()) {
        QJsonObject obj;
        obj.insert("cmd", "login");
        obj.insert("user", username);
        obj.insert("password", password);
        m_username = username;                          // 记录当前登录的用户名
        QByteArray byte = QJsonDocument(obj).toJson();
        socket->write(byte);
    }
}

void Widget::server_reply()
{
    QByteArray byte = socket->readAll();
    QJsonObject obj = QJsonDocument::fromJson(byte).object();
    QString cmd = obj.value("cmd").toString();
    QString res = obj.value("result").toString();
    if(cmd == "register_reply") {
        client_register_handler(res);
    }
    else if(cmd == "login_reply") {
        QString friends = obj.value("friends").toString();
        QString groups = obj.value("groups").toString();
        client_login_handler(res, friends, groups);
    }

}

void Widget::client_register_handler(QString res)
{
    if(res == "success") {
        QMessageBox::warning(this, "注册提示", "注册成功！");
    }
    else if(res == "failure") {
        QMessageBox::warning(this, "注册提示", "注册失败，该用户已经存在！");
    }
}

void Widget::client_login_handler(QString res, QString friends, QString groups)
{
    if(res == "user_not_exist") {
        QMessageBox::warning(this, "登录提示", "用户不存在！");
    }
    else if(res == "password_error") {
        QMessageBox::warning(this, "登录提示", "用户密码错误，请重试！");
    }
    else if(res == "success") {      // 登录成功，跳转到新界面
        this->hide();                // 关闭当前界面，并取消server_reply的槽函数绑定
        socket->disconnect(SIGNAL(readyRead()));
        ChatList* chatlist = new ChatList(socket, m_username, friends, groups);
        QString str = QString("当前用户: %1").arg(m_username);
        chatlist->setWindowTitle(str);
        chatlist->show();
    }
}
