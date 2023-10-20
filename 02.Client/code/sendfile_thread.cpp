#include "sendfile_thread.h"
#include <QMessageBox>
#include <QDebug>

SendFileThread::SendFileThread(QJsonObject &obj)
{
    QString filepath = obj.value("file_pathname").toString();
    int filesize = obj.value("file_size").toInt();
    int port = obj.value("port").toInt();
    m_file_pathname = filepath;
    m_filesize = filesize;
    m_port = port;
}

void SendFileThread::run()                              // 这里实际上是子线程在处理
{
    QTcpSocket sendsocket;
    sendsocket.connectToHost(QHostAddress("47.113.226.70"), m_port);
    if(!sendsocket.waitForConnected(10 * 1000)) {       // 10s后还没连接上，提示服务器连接失败，线程退出，
        // QMessageBox::warning(this, "发送文件提示", "连接文件服务器失败，超时连接！");
        qDebug() << "发送方，连接文件服务器失败，超时连接！" << "\n";
        this->quit();
    }
    else {
        QFile file(m_file_pathname);
        file.open(QIODevice::ReadOnly);
        while(true) {
            QByteArray byte = file.read(1024 * 1024);   // 每次读1MB
            if(byte.size() == 0) {
                break;
            }
            sendsocket.write(byte);                     // 发送方，循环发送数据
            sendsocket.flush();
            qDebug() << "发送方，已发送完数据： " << byte.size() << " B, 共 " << byte.size()/1024/1024 << " MB";
            sleep(1);                                   // 休眠1s
        }
        qDebug() << "发送方，文件发送完成！ ";
        if(sendsocket.waitForDisconnected()) {          // 当服务器完成发送完成，断开连接时，关闭资源
            qDebug() << "文件服务器已经断开连接！ ";
            file.close();
            sendsocket.close();
            this->quit();                               // 发送完成，关闭资源线程退出
        }
    }
}
