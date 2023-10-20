#include "recvfile.h"
#include <QMessageBox>
#include <QDebug>

RecvFile::RecvFile(QJsonObject &obj, QObject *parent)
    : QObject{parent}
{
    QString filepath = obj.value("file_pathname").toString();
    QStringList pathlist = filepath.split('/');
    QString real_filename = pathlist.at(pathlist.size() - 1);                         // 截取到真正的文件名信息（获得最后一个以'/'分隔的信息，就是文件名）

    int filesize = obj.value("file_size").toInt();
    int port = obj.value("port").toInt();
    m_real_filename = real_filename;
    m_filesize = filesize;
    m_cur_recvsize = 0;
    m_port = port;
}

void RecvFile::working()                                    // 处理用户接收、发送文件
{
    m_file = new QFile(m_real_filename);
    m_file->open(QIODevice::WriteOnly);                     // 打开一个文件即接收文件的存储路径，在客户端执行程序的上一级目录下

    m_recvsocket = new QTcpSocket;                          // 在子线程里初始化连接的socket
                                                            // 哪个线程发出的信号，就在该线程中处理，将槽函数recv_file在子线程中执行
    connect(m_recvsocket, &QTcpSocket::readyRead, this, &RecvFile::recv_file);

    m_recvsocket->connectToHost(QHostAddress("47.113.226.70"), m_port);
    qDebug() << "m_port:" << m_port << "\n";
    if(!m_recvsocket->waitForConnected(10 * 1000)) {        // 10s后还没连接上，提示服务器连接失败，线程退出，
        qDebug() << "接收方，连接文件服务器失败，超时连接！" << "\n";
        emit recv_connect_timeout();                        // 连接超时，通知主线程
    }

}

void RecvFile::recv_file()
{
    QByteArray byte = m_recvsocket->readAll();
    qDebug() << "当前接收到了数据： " << byte.size() << " B";
    m_cur_recvsize += byte.size();
    m_file->write(byte);                                    // 将收到的文件数据写入到文件中
    qDebug() << "当前接收到文件大小： " << m_cur_recvsize << " B, 共 " << m_cur_recvsize/1024/1024 << " MB";

    if(m_cur_recvsize >= m_filesize) {                      // 文件接收完成
        m_file->close();
        m_recvsocket->close();
        delete m_file;                                      // 删除堆上开辟的内存
        delete m_recvsocket;
        qDebug() << "接收文件成功！";
        emit recv_file_success();
    }
}

