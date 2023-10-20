#ifndef RECVFILE_H
#define RECVFILE_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTcpSocket>
#include <QHostAddress>
#include <QFile>

class RecvFile : public QObject
{
    Q_OBJECT
public:
    explicit RecvFile(QJsonObject &obj, QObject *parent = nullptr);

public:
    void working();

public slots:
    void recv_file();

signals:
    void recv_connect_timeout();
    void recv_file_success();

private:
    QString m_real_filename;
    int m_filesize;
    int m_cur_recvsize;
    int m_port;

    QTcpSocket* m_recvsocket;
    QFile* m_file;

};

#endif // RECVFILE_H
