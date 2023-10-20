#ifndef CREATE_GROUP_H
#define CREATE_GROUP_H

#include <QWidget>
#include <QTcpSocket>

namespace Ui {
class CreateGroup;
}

class CreateGroup : public QWidget {
    Q_OBJECT

public:
    explicit CreateGroup(QTcpSocket* socket, QString username, QWidget *parent = nullptr);
    ~CreateGroup();

private slots:
    void on_cancelBtn_clicked();
    void on_createGroupBtn_clicked();

private:
    Ui::CreateGroup *ui;
    QTcpSocket* m_socket;
    QString m_username;

};

#endif // CREATE_GROUP_H
