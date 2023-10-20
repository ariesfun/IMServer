#ifndef RECVFILETHREAD_H
#define RECVFILETHREAD_H

#include <QThread>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTcpSocket>
#include <QHostAddress>
#include <QFile>

class RecvFileThread : public QThread {
    Q_OBJECT                // 使用了信号与槽
public:
    // explicit RecvFileThread(QJsonObject &obj, QObject *parent = nullptr);

    RecvFileThread(QJsonObject &obj);

public:
    void run();

private slots:
    void recv_file();

private:
    QString m_real_filename;
    int m_filesize;
    int m_cur_recvsize;
    int m_port;

    QTcpSocket* m_recvsocket;
    QFile* m_file;
};

#endif // RECVFILETHREAD_H
