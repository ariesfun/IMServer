#pragma once
#include <event.h>
#include <jsoncpp/json/json.h> 
#include "chat_info_list.h"
#include "chat_db.h"

#define IP "192.168.184.10"                      // 绑定的服务器的IP地址 
#define PORT 7878
#define READ_BUF_MAXSIZE 4096
#define BUFFER_MAXSIZE 1024 * 1024

class Server {
public:
    Server(const char* ip = "127.0.0.1", int serverPort = 7878);
    ~Server();    

private:
    struct event_base* m_base;                   // 事件集合
    struct evconnlistener* m_listener;           // 监听事件
    static ChatInfo* chatlist;                   // 存放用户链表信息对象(用户+群组)

    static char read_buffer[READ_BUF_MAXSIZE];               // 服务器的读缓冲区
    static ChatDB* m_db;                         // 数据库对象

private:
    // 回调函数，需要设置为静态的; libevent 通常期望回调函数是静态的或者是全局函数
    // 因为它们需要通过函数指针来调用这些函数，而不是通过类的实例对象; 使用 void* ctx 参数，以便传递上下文信息
    static void listener_cb(struct evconnlistener* listener, evutil_socket_t fd, 
                            struct sockaddr* addr, int socklen, void* arg);
    static void client_handler(int fd);
    static void sendfile_handler(int port, int filesize, int &from_fd, int &to_fd, struct bufferevent* bev);

    static void read_cb(struct bufferevent* bev, void* ctx);
    static void event_cb(struct bufferevent* bev, short what, void* ctx);

private:
    static void parse_json_data(struct bufferevent* bev);
    static void user_register(struct bufferevent* bev, Json::Value value);
    static void user_login(struct bufferevent* bev, Json::Value value);
    static void user_add_friend(struct bufferevent* bev, Json::Value value);
    static void user_create_group(struct bufferevent* bev, Json::Value value);
    static void user_add_group(struct bufferevent* bev, Json::Value value);
    static void user_private_chat(struct bufferevent* bev, Json::Value value);
    static void user_group_chat(struct bufferevent* bev, Json::Value value);
    static void user_get_groupmember(struct bufferevent* bev, Json::Value value);
    static void user_send_file(struct bufferevent* bev, Json::Value value);
    static void user_offline(struct bufferevent* bev, Json::Value value);

};