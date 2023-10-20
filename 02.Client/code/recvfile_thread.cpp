#include "recvfile_thread.h"
#include <QMessageBox>
#include <QDebug>

//RecvFileThread::RecvFileThread(QJsonObject &obj, QObject *parent)
//    : QThread{parent}
//{
RecvFileThread::RecvFileThread(QJsonObject &obj)
{
    QString filepath = obj.value("file_pathname").toString();
    QStringList pathlist = filepath.split('/');
    QString real_filename = pathlist.at(pathlist.size() - 1);                         // 截取到真正的文件名信息（获得最后一个以'/'分隔的信息，就是文件名）

    // int index = local_filepath.lastIndexOf("/");
    // QString filename = local_filepath.right(local_filepath.size() - 1 - index);    // 或者这样，截取发送的文件名

    int filesize = obj.value("file_size").toInt();
    int port = obj.value("port").toInt();
    m_real_filename = real_filename;
    m_filesize = filesize;
    m_cur_recvsize = 0;
    m_port = port;
}

void RecvFileThread::run()                                  // 只有run()才是子线程，接收方在子线程里，循环接收数据
{
    m_file = new QFile(m_real_filename);
    m_file->open(QIODevice::WriteOnly);                     // 打开一个文件即接收文件的存储路径，在客户端执行程序的上一级目录下

    m_recvsocket = new QTcpSocket;                          // 在子线程里初始化连接的socket
                                                            // 哪个线程发出的信号，就在该线程中处理，将槽函数recv_file在子线程中执行
    connect(m_recvsocket, &QTcpSocket::readyRead, this, &RecvFileThread::recv_file, Qt::DirectConnection);

    m_recvsocket->connectToHost(QHostAddress("192.168.184.10"), m_port);
    qDebug() << "m_port:" << m_port << "\n";
    if(!m_recvsocket->waitForConnected(10 * 1000)) {        // 10s后还没连接上，提示服务器连接失败，线程退出，
        // QMessageBox::warning(this, "发送文件提示", "连接文件服务器失败，超时连接！");
        qDebug() << "接收方，连接文件服务器失败，超时连接！" << "\n";
        this->quit();
    }

//    else {
//        QFile file(m_file_pathname);
//        file.open(QIODevice::ReadOnly)
//            while(true) {
//            QByteArray byte = file.read(1024);
//            if(byte.size() == 0) {
//                break;
//            }
//            sendsocket.write(byte);                       // 发送方，循环发送数据
//        }
//        file.close();
//        sendsocket.close();
//        this->quit();                                     // 发送完成，关闭资源线程退出
//    }
    exec();                                                 // 进行死循环，程序不会停止
}

                                                            // 该槽函数属于主线程
void RecvFileThread::recv_file()                            // 收到信号时，会不断触发该槽函数，每一次读取所有数据
{
    QByteArray byte = m_recvsocket->readAll();
    m_cur_recvsize += byte.size();
    m_file->write(byte);                                    // 将收到的文件数据写入到文件中
    qDebug() << "当亲接收到文件大小： " << byte.size() << " B, 共 " << byte.size()/1024 << " MB";

    if(m_cur_recvsize >= m_filesize) {                      // 文件接收完成
        qDebug() << "接收文件成功！";
        m_file->close();
        m_recvsocket->close();
        delete m_file;                                      // 删除堆上开辟的内存
        delete m_recvsocket;
        this->quit();
    }
}
