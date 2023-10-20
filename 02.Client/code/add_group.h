#ifndef ADD_GROUP_H
#define ADD_GROUP_H

#include <QWidget>
#include <QTcpSocket>

namespace Ui {
class AddGroup;
}

class AddGroup : public QWidget {
    Q_OBJECT

public:
    explicit AddGroup(QTcpSocket* socket, QString username, QWidget *parent = nullptr);
    ~AddGroup();

private slots:
    void on_cancelBtn_clicked();

    void on_addGroupBtn_clicked();

private:
    Ui::AddGroup *ui;
    QTcpSocket* m_socket;
    QString m_username;

};

#endif // ADD_GROUP_H
