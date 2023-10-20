#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget {
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void connect_error(QAbstractSocket::SocketError error);
    void connect_success();
    void on_registerBtn_clicked();
    void on_loginBtn_clicked();

    void server_reply();            //处理服务器回复的消息

private:
    void client_register_handler(QString res);
    void client_login_handler(QString res, QString friends, QString groups);

private:
    Ui::Widget *ui;
    QTcpSocket* socket;
    QString m_username;

};
#endif // WIDGET_H
