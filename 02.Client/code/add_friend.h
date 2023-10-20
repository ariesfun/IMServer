#ifndef ADD_FRIEND_H
#define ADD_FRIEND_H

#include <QWidget>
#include <QTcpSocket>

namespace Ui {
class AddFriend;
}

class AddFriend : public QWidget {
    Q_OBJECT

public:
    explicit AddFriend(QTcpSocket* socket, QString username, QWidget* parent = nullptr);
    ~AddFriend();

private slots:
    void on_cancelBtn_clicked();
    void on_addFriendBtn_clicked();

private:
    Ui::AddFriend* ui;
    QTcpSocket* m_socket;
    QString m_username;

};

#endif // ADD_FRIEND_H
