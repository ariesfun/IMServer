#ifndef SENDFILETHREAD_H
#define SENDFILETHREAD_H

#include <QThread>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTcpSocket>
#include <QHostAddress>
#include <QFile>

class SendFileThread : public QThread {
public:
    SendFileThread(QJsonObject &obj);

public:
    void run();             // 线程执行时，运行的就是run（）函数

private:
    QString m_file_pathname;
    int m_filesize;
    int m_port;

};

#endif // SENDFILETHREAD_H
