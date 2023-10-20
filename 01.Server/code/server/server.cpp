#include "server.h"
#include <sys/types.h>           
#include <sys/socket.h>         // socket()
#include <netinet/in.h>
#include <arpa/inet.h>          // inet_addr()
#include <event2/listener.h>    // _new_bind()
#include <unistd.h>
#include <cstring>
#include <thread>
#include <iostream>

char Server::read_buffer[READ_BUF_MAXSIZE] = {0};   
ChatDB* Server::m_db = new ChatDB;
ChatInfo* Server::chatlist = new ChatInfo;

Server::Server(const char* ip, int serverPort)
{
    // Test, 创建链表对象
    // chatlist = new ChatInfo;

    // 创建事件集合
    m_base = event_base_new();

    // 绑定本地服务器的地址信息
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(serverPort);

    // 创建监听器对象, 并且会加入待事件集合中
    m_listener = evconnlistener_new_bind(m_base, listener_cb, nullptr, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 
                                         10, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(m_listener == nullptr) {
        std::cout << "Execute evconnlistener_new_bind() error!" << std::endl;
    }
    std::cout << "\nThe server information is initialized successfully! \nStarts listening to the client connection!" << std::endl;
    event_base_dispatch(m_base);                // 监听集合
}

Server::~Server() 
{
    event_base_free(m_base);                    // 释放集合
}

// 当有客户端发起连接的时候，会触发该回调函数
void Server::listener_cb(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* addr, int socklen, void* arg)
{
    std::cout << "\nA new client connection was received, the fd_info: " << fd << std::endl;
    std::thread client_th(client_handler, fd);  // 创建工作线程来处理客户端
    client_th.detach();                         // 挂在后台，线程运行结束后会自动释放资源
}

// 每个线程处理客户端的工作函数
void Server::client_handler(int fd)
{
    // 创建集合，每个线程有一个单独的base
    struct event_base* base = event_base_new();

    // 创建bufferevent对象
    struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);  // Tcp关闭时会自动释放内存, bev就会释放掉
    if(bev == nullptr) {
        std::cout << "Execute bufferevent_socket_new() error!" << std::endl;
    }

    // 给bufferevent设置回调函数
    bufferevent_setcb(bev, read_cb, nullptr, event_cb, nullptr);

    // 是能回调函数
    bufferevent_enable(bev, EV_READ);

    event_base_dispatch(base);                                                          // 死循环，监听集合（监听客户端是否有数据发送过来）
    event_base_free(base);      
    std::cout << "The current thread has exited. Release the collection with thread for base!" << std::endl; 
}

void Server::read_cb(struct bufferevent* bev, void* ctx)
{
    int size = bufferevent_read(bev, read_buffer, sizeof(read_buffer));                 // 读取数据
    if(size < 0) {
        std::cout << "Execute bufferevent_read() error!" << std::endl; 
    }
    std::cout << "\nRead one json with string data:\n" << read_buffer << '\n' << std::endl;
    // 静态成员函数，只能调用静态的
    parse_json_data(bev);                                                               // 解析读取到的字符串数据
}

void Server::event_cb(struct bufferevent* bev, short what, void* ctx)
{

}

void Server::parse_json_data(struct bufferevent* bev)                                   // 解析JSON,处理不同的业务逻辑
{
    Json::Reader reader;                                                                // 解析JSON数据对象
    Json::Value val;                                                                    // 创建一个JSON数据对象（6大类型）
    if(!reader.parse(read_buffer, val)) {                                               // 将读到的字符串转换成JSON对象
        std::cout << "\nThe Server parse json data failure!" << std::endl; 
    }
    std::string cmd = val["cmd"].asString();                                            // 根据解析到的不同事件进行相应处理
    std::cout << "Execute parse_json_data(), the command: " << cmd << std::endl;

    if(cmd == "register") {                                                             // 处理客户端的各种请求
        user_register(bev, val);
    }
    else if(cmd == "login") {
        user_login(bev, val);
    }
    else if(cmd == "add_friend") {
        user_add_friend(bev, val);
    }
    else if(cmd == "create_group") {
        user_create_group(bev, val);
    }
    else if(cmd == "add_group") {
        user_add_group(bev, val);
    }
    else if(cmd == "private_chat") {
        user_private_chat(bev, val);
    }
    else if(cmd == "group_chat") {
        user_group_chat(bev, val);
    }
    else if(cmd == "get_groupmember") {
        user_get_groupmember(bev, val);
    }
    else if(cmd == "send_file") {
        user_send_file(bev, val);
    }
    else if(cmd == "offline") {
        user_offline(bev, val);
    }
    
}

void Server::user_register(struct bufferevent* bev, Json::Value value)
{
    m_db->db_connect("funcloud_userinfo_db");
    std::string usrname = value["user"].asString();
    bool flag = m_db->verify_user_isexist(usrname);
    if(flag) {                                                                           // 用户存在，封装数据返回失败给客户端
        Json::Value val;
        val["cmd"] = "register_reply";
        val["result"] = "failure";
        val["message"] = "user_already_exist";
        
        std::string data = Json::FastWriter().write(val);                               // 封装的JSON数据对象
        std::cout << "\nTo send user json data:\n" << data << '\n' << std::endl;
        int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));     // 向缓冲事件写入数据
        if(write_ret < 0) {
            std::cout << "Execute bufferevent_write failure!" << std::endl;
        }
    }
    else {                                                                              // 用户不存在，可以执行注册操作
        Json::Value val;
        val["cmd"] = "register_reply";
        val["result"] = "success";
        
        std::string data = Json::FastWriter().write(val);
        std::cout << "\nTo send user json data:\n" << data << '\n' << std::endl;       
        int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));     
        if(write_ret < 0) {
            std::cout << "Execute bufferevent_write failure!" << std::endl;
        }

        std::string usrpwd = value["password"].asString();
        m_db->new_user_register(usrname, usrpwd);                                       // 执行注册操作，向数据库中写入数据
    }

    m_db->db_disconnect();
}

void Server::user_login(struct bufferevent* bev, Json::Value value)
{
    m_db->db_connect("funcloud_userinfo_db");
    std::string usrname = value["user"].asString();
    bool name_ret = m_db->verify_user_isexist(usrname);                                     // 校验用户名 
    if(!name_ret) {                                                                         // 用户名不存在                                                                   
        Json::Value val;
        val["cmd"] = "login_reply";
        val["result"] = "user_not_exist";
        
        std::string data = Json::FastWriter().write(val);                       
        std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
        int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
        if(write_ret < 0) {
            std::cout << "Execute bufferevent_write failure!" << std::endl;
        }
        return;
    }

    std::string usrpwd = value["password"].asString();
    bool pwd_ret = m_db->verify_user_password(usrname, usrpwd);                            // 校验用户密码
    if(!pwd_ret) {                                                                         // 密码错误
        Json::Value val;
        val["cmd"] = "login_reply";
        val["result"] = "password_error";
        
        std::string data = Json::FastWriter().write(val);                       
        std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
        int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
        if(write_ret < 0) {
            std::cout << "Execute bufferevent_write failure!" << std::endl;
        }
        return;
    }
    
    std::string friends_info = m_db->db_get_friendslist(usrname);                         // 获得好友及群组列表并返回
    std::string groups_info = m_db->db_get_groupslist(usrname);   
    Json::Value val;
    val["cmd"] = "login_reply";
    val["result"] = "success";
    val["friends"] = friends_info;
    val["groups"] = groups_info;
    
    std::string data = Json::FastWriter().write(val);                       
    std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
    int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
    if(write_ret < 0) {
        std::cout << "Execute bufferevent_write failure!" << std::endl;
    } 

    User u = {usrname, bev};    
    chatlist->online_user->push_back(u);                                                 // 向在线用户链表中添加当前用户

    std::string fri_name;                                                                // 向该用户的所有在线的好友发送上线提醒
    int start = 0, end = 0;
    bool flag = true;
    while(flag) {
        end = friends_info.find('|', start);                                             // 每次截取一个好友成员子串
        if(end == -1) {
            fri_name = friends_info.substr(start, friends_info.size() - start);          // 提取最后一个好友，无'|'分隔
            flag = false;
        }
        else {
            fri_name = friends_info.substr(start, end - start);                          // 获得一个好友姓名
        }
        for(std::list<User>::iterator it = chatlist->online_user->begin();               // 遍历当前所有在线的用户
            it != chatlist->online_user->end(); it++) {
            if(it->username == fri_name) {                                               // 找到了一个在线好友
                Json::Value val;
                val["cmd"] = "friend_online_reply";
                val["friend"] = usrname;
                
                std::string data = Json::FastWriter().write(val);                       
                std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
                int write_ret = bufferevent_write(it->bev, data.c_str(), strlen(data.c_str()));
                if(write_ret < 0) {
                    std::cout << "Execute bufferevent_write failure!" << std::endl;
                }
            }
        }
        start = end + 1;
    }

    // Test
    for(std::list<User>::iterator it = chatlist->online_user->begin();               // 遍历当前所有在线的用户
        it != chatlist->online_user->end(); it++) {
        printf("Current all login user: %s\n", (it->username).c_str());
    }
    puts("");

    m_db->db_disconnect();
}

void Server::user_add_friend(struct bufferevent* bev, Json::Value value)
{
    m_db->db_connect("funcloud_userinfo_db");
    std::string usrname = value["user"].asString();
    std::string to_add_friend = value["friend"].asString();
    bool flag = m_db->verify_user_isexist(to_add_friend);                    // 判断添加的好友是否存在
    if(!flag) {                                                             
        Json::Value val;
        val["cmd"] = "add_friend_reply";
        val["result"] = "user_not_exist";
        
        std::string data = Json::FastWriter().write(val);                       
        std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
        int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
        if(write_ret < 0) {
            std::cout << "Execute bufferevent_write failure!" << std::endl;
        }
        return; 
    }

    bool is_friend_ret = m_db->verify_is_friends(usrname, to_add_friend);
    if(is_friend_ret) {                                                     // 已经是好友关系
        Json::Value val;
        val["cmd"] = "add_friend_reply";
        val["result"] = "already_is_friend";
        
        std::string data = Json::FastWriter().write(val);                       
        std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
        int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
        if(write_ret < 0) {
            std::cout << "Execute bufferevent_write failure!" << std::endl;
        }
        return; 
    }

    m_db->new_friend_add(usrname, to_add_friend);                           // 还不是好友关系，可以进行添加，需要修改双方的数据库(且双方都要回复消息)
    m_db->new_friend_add(to_add_friend, usrname);
    Json::Value val;                                                        // 回复添加方
    val["cmd"] = "add_friend_reply";
    val["result"] = "success";
    val["friend"] = to_add_friend;
    
    std::string data = Json::FastWriter().write(val);                       
    std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
    int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
    if(write_ret < 0) {
        std::cout << "Execute bufferevent_write failure!" << std::endl;
    }

    for(std::list<User>::iterator it = chatlist->online_user->begin();     // 如果被添加方在线，回复他        
        it != chatlist->online_user->end(); it++) {
        if(it->username == to_add_friend) {                                                 
            val.clear();
            val["cmd"] = "add_friend_reply";
            val["result"] = "success";
            val["new_friend"] = usrname;
            
            std::string data = Json::FastWriter().write(val);                       
            std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
            int write_ret = bufferevent_write(it->bev, data.c_str(), strlen(data.c_str()));   // TODO，这里是to_bev,有问题
            if(write_ret < 0) {
                std::cout << "Execute bufferevent_write failure!" << std::endl;
            }
        }
    }

    m_db->db_disconnect();
}

void Server::user_create_group(struct bufferevent* bev, Json::Value value)
{
    m_db->db_connect("funcloud_chatgroup_db");                       // 更新群组数据库信息
    std::string groupname = value["group"].asString();      
    bool flag = m_db->verify_group_isexist(groupname);               // 判断群组是否存在
    if(flag) {                                                             
        Json::Value val;
        val["cmd"] = "create_group_reply";
        val["result"] = "group_already_exist";
        
        std::string data = Json::FastWriter().write(val);                       
        std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
        int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
        if(write_ret < 0) {
            std::cout << "Execute bufferevent_write failure!" << std::endl;
        }
        return; 
    }

    std::string username = value["user"].asString();                 // 不存在时，需要修改数据库信息(群组名+群主)，之后回复创建成功
    m_db->new_group_create(groupname, username);                     
    m_db->db_disconnect();

    m_db->db_connect("funcloud_userinfo_db");                        // 更新用户数据库信息
    m_db->user_add_groupinfo(username, groupname);                   // 用户更新自己创建的群组信息

    chatlist->add_new_groupinfo(groupname, username);                          // 修改群组链表信息
    
    Json::Value val;
    val["cmd"] = "create_group_reply";
    val["result"] = "success";
    val["new_group"] = groupname;
    
    std::string data = Json::FastWriter().write(val);                       
    std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
    int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
    if(write_ret < 0) {
        std::cout << "Execute bufferevent_write failure!" << std::endl;
    }

    m_db->db_disconnect();
}

void Server::user_add_group(struct bufferevent* bev, Json::Value value)
{
    std::string groupname = value["group"].asString();
    bool flag = chatlist->verify_groupinfo_isexist(groupname);                      // 通过群组信息链表，判断群组是否存在 (遍历链表会比查询数据库高效)
    if(!flag) {                                                                     // 群组不存在
        Json::Value val;
        val["cmd"] = "add_group_reply";
        val["result"] = "group_not_exist";
        
        std::string data = Json::FastWriter().write(val);                       
        std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
        int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
        if(write_ret < 0) {
            std::cout << "Execute bufferevent_write failure!" << std::endl;
        }
        return;
    }
    
    std::string username = value["user"].asString();
    bool is_groupmember = chatlist->verify_is_groupmember(groupname, username);     // 判断用户是否为群成员
    if(is_groupmember) {                                                            // 已经是群成员了
        Json::Value val;
        val["cmd"] = "add_group_reply";
        val["result"] = "user_in_group";
        
        std::string data = Json::FastWriter().write(val);                       
        std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
        int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
        if(write_ret < 0) {
            std::cout << "Execute bufferevent_write failure!" << std::endl;
        }
        return;
    }
    m_db->db_connect("funcloud_userinfo_db");                                       // 修改数据库(用户表、群组表)
    m_db->user_add_groupinfo(username, groupname);                                  // 用户更新添加的群组信息
    m_db->db_disconnect();

    m_db->db_connect("funcloud_chatgroup_db");      
    m_db->group_add_memberinfo(groupname, username);                                // 群组更新添加的成员信息
    m_db->db_disconnect();

    chatlist->update_user_groupinfo(groupname, username);                           // 修改群组链表
    Json::Value val;
    val["cmd"] = "add_group_reply";
    val["result"] = "success";
    val["added_group"] = groupname;
    
    std::string data = Json::FastWriter().write(val);                       
    std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
    int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
    if(write_ret < 0) {
        std::cout << "Execute bufferevent_write failure!" << std::endl;
    }
}

void Server::user_private_chat(struct bufferevent* bev, Json::Value value)
{
    std::string user_to_username = value["user_to"].asString();
    struct bufferevent* to_bev = chatlist->get_onlineuser_bev(user_to_username);
    if(to_bev == nullptr) {                                                 // 对方不在线
        Json::Value val;
        val["cmd"] = "private_chat_reply";
        val["result"] = "user_to_offline";
        
        std::string data = Json::FastWriter().write(val);                       
        std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
        int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
        if(write_ret < 0) {
            std::cout << "Execute bufferevent_write failure!" << std::endl;
        }
        return;
    }

    std::string data = Json::FastWriter().write(value);                     // 对方在线将收到的信息直接转发给对方  
    std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
    int write_ret = bufferevent_write(to_bev, data.c_str(), strlen(data.c_str()));
    if(write_ret < 0) {
        std::cout << "Execute bufferevent_write failure!" << std::endl;
    }

    Json::Value val;                                                        // 回复消息发送成功
    val["cmd"] = "private_chat_reply";
    val["result"] = "success";
    
    data = Json::FastWriter().write(val);                       
    std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
    write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
    if(write_ret < 0) {
        std::cout << "Execute bufferevent_write failure!" << std::endl;
    }
}

void Server::user_group_chat(struct bufferevent* bev, Json::Value value)
{
    std::string username = value["user"].asString();
    std::string groupname = value["group"].asString();
    for(std::list<Group>::iterator it = chatlist->group_info->begin();      // 遍历群组信息链表
        it != chatlist->group_info->end(); it++) {
        if(it->group_name == groupname) {
            for(std::list<GroupUser>::iterator it_user = it->group_list->begin(); 
                it_user != it->group_list->end(); it_user++) {              // 遍历该群的所有用户
                struct bufferevent* to_bev = chatlist->get_onlineuser_bev(it_user->group_username);
                if(to_bev != nullptr) {                                      // 对在线的群成员转发群消息
                    std::string data = Json::FastWriter().write(value);                       
                    std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
                    int write_ret = bufferevent_write(to_bev, data.c_str(), strlen(data.c_str()));
                    if(write_ret < 0) {
                        std::cout << "Execute bufferevent_write failure!" << std::endl;
                    }
                }
            }
        }
    }

    Json::Value val;                                                        // 回复消息发送成功
    val["cmd"] = "group_chat_reply";
    val["result"] = "success";
    
    std::string data = Json::FastWriter().write(val);                       
    std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
    int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
    if(write_ret < 0) {
        std::cout << "Execute bufferevent_write failure!" << std::endl;
    }
}

void Server::user_get_groupmember(struct bufferevent* bev, Json::Value value)
{
    std::string groupname = value["group"].asString();
    std::string group_memberlist = chatlist->get_group_memberinfo(groupname);
    Json::Value val;                                                        // 回复消息发送成功
    val["cmd"] = "get_groupmember_reply";
    val["group"] = groupname;
    val["result"] = group_memberlist;
    
    std::string data = Json::FastWriter().write(val);                       
    std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
    int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
    if(write_ret < 0) {
        std::cout << "Execute bufferevent_write failure!" << std::endl;
    }
}

void Server::sendfile_handler(int port, int filesize, int &from_fd, int &to_fd, struct bufferevent* bev)
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);                                       // 使用TCP Socket搭建文件服务器
    if(listen_fd == -1) {
        int saved_errno = errno;
        printf("%s \nThe check_error detail: %s", "create listen sockfd failed!", strerror(saved_errno));
    }

    int reuse = 1;                                                                         // 设置允许端口复用
    int ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if(ret == -1) {
        printf("setsockopt() oprate error!");
    }

    int recvBuffer = BUFFER_MAXSIZE;                                                       // 在传输大文件时，发送/接收缓冲区大小设置为1MB
    int recv_ret = setsockopt(listen_fd, SOL_SOCKET, SO_RCVBUF, (const char*)&recvBuffer, sizeof(int));
    if(recv_ret == -1) {
        printf("Recv setsockopt() oprate error!");
    }
    int sendBuffer = BUFFER_MAXSIZE;                                                      
    int send_ret = setsockopt(listen_fd, SOL_SOCKET, SO_SNDBUF, (const char*)&sendBuffer, sizeof(int));
    if(send_ret == -1) {
        printf("Recv setsockopt() oprate error!");
    }

    struct sockaddr_in server_addr;                                                        // 初始化本地服务器的地址信息
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP);
    server_addr.sin_port = htons(port);

    int bind_ret = bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));  // 绑定本地地址信息
    if(bind_ret == -1) {
        int saved_errno = errno;
        printf("%s \nThe check_error detail: %s", "bind listen sockfd failed!", strerror(saved_errno));
    }
    
    int listen_ret = listen(listen_fd, 10);                                                 // 开始监听
        if(listen_ret == -1) {
        int saved_errno = errno;
        printf("%s \nThe check_error detail: %s", "start listen sockfd failed!", strerror(saved_errno));
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);                                        
    from_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);        // 接收发送客户端的连接，解引用将连接的fd赋值给发送方
    int m_from_fd = from_fd;
    to_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    int m_to_fd = to_fd;
    
    char file_buffer[BUFFER_MAXSIZE] = {0};
    size_t cur_size, recv_sum = 0;
    while(true) {                                                                          // 文件服务器负责变接收边转发文件数据
        cur_size = recv(m_from_fd, file_buffer, BUFFER_MAXSIZE, 0);
        // if(cur_size <= 0 || cur_size > BUFFER_MAXSIZE) {                                   // 发送客户端异常退出或数据超过缓冲区限制
        //     break;
        // }
        if(cur_size < 0) {
            perror("recv()");
            break;
        }
        else if(cur_size == 0) {
            std::cout << "发送客户端已经断开连接" << std::endl;
            break;
        }

        std::cout << "循环一次读取数据：" << cur_size << " B" << std::endl;
        recv_sum += cur_size;
        std::cout << "服务器当前收到的总数据大小：" << recv_sum << " B, 共 " << recv_sum/1024/1024 << "MB" << std::endl;
        send(m_to_fd, file_buffer, cur_size, 0);
        std::cout << "正在发送数据数据：" << cur_size << " B" << std::endl;                                             
        if(recv_sum >= filesize) {                                                         // 服务器文件接收完成，退出循环
            std::cout << "文件已经发送完成！" << std::endl;
            Json::Value val;                                                               // 回复发送方文件发送成功
            val["cmd"] = "send_file_reply";
            val["result"] = "success";
            
            std::string data = Json::FastWriter().write(val);                       
            std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
            int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
            if(write_ret < 0) {
                std::cout << "Execute bufferevent_write failure!" << std::endl;
            }
            break;
        }
        memset(file_buffer, 0, BUFFER_MAXSIZE);
    }

    close(m_from_fd);                                                                                // 释放资源
    close(m_to_fd);
    close(listen_fd);
}

void Server::user_send_file(struct bufferevent* bev, Json::Value value)
{
    std::string to_username = value["user_to"].asString();
    struct bufferevent* to_bev = chatlist->get_onlineuser_bev(to_username);                         // 对方在线时，就获取对方的bev
    if(to_bev == nullptr) {                                                                         // 对方不在线
        Json::Value val;
        val["cmd"] = "send_file_reply";
        val["result"] = "user_to_offline";
        
        std::string data = Json::FastWriter().write(val);                       
        std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
        int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
        if(write_ret < 0) {
            std::cout << "Execute bufferevent_write failure!" << std::endl;
        }
        return;
    }

    std::string filepath = value["file_pathname"].asString();
    int filesize = value["file_size"].asInt();
    int port = 8000;                                                                                // 这里应该是系统随机分配一个未被占用的端口号
    int from_fd, to_fd = 0;
    std::thread sendfile_th(sendfile_handler, port, filesize, std::ref(from_fd), std::ref(to_fd), bev);               // 启动新线程，创建文件服务器
    sendfile_th.detach();                                                                           // 设置线程分离，挂在后台

    Json::Value val;                                                                                // 分配完端口号后，回复发送方，发送文件的端口(确保该端口号是开放的)
    val["cmd"] = "sendfile_port_reply";
    val["port"] = port;
    val["file_pathname"] = filepath;
    val["file_size"] = filesize;
    
    std::string data = Json::FastWriter().write(val);                       
    std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
    // 这里的bufferevent_write(bev, data.c_str(), strlen(data.c_str())) 会将数组存入到缓冲区中并不会立即发送消息，改用系统函数write可以解决
    // write()会将数据写入到套接字的描述符中
    int write_ret = send(bev->ev_read.ev_fd, data.c_str(), strlen(data.c_str()), 0);                // 发送消息给发送方
    if(write_ret < 0) {
        std::cout << "Execute write() failure!" << std::endl;
    }

    int cnt = 0;
    while(from_fd <= 0) {                                                                           // 发送方还没有开始连接，设置超时等待，阻塞住
        cnt++;
        usleep(1000 * 100);                                                                         // 睡眠100ms
        if(cnt == 100) {                                                                            // 经过了10s，还是没有连接，回复客户端连接超时
            pthread_cancel(sendfile_th.native_handle());                                            // 取消线程
            val.clear();
            val["cmd"] = "send_file_reply";
            val["result"] = "timeout";
            
            data = Json::FastWriter().write(val);                       
            std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
            int write_ret = bufferevent_write(bev, data.c_str(), strlen(data.c_str()));
            if(write_ret < 0) {
                std::cout << "Execute bufferevent_write failure!" << std::endl;
            }
            std::cout << "文件发送方，连接超时！" << std::endl;
            return;
        }
    }

    val.clear();                                                                        // 到这里，说明发送方已经建立Tcp连接了
    val["cmd"] = "recvfile_port_reply";                                                 // 发送端口号信息给对方to_bev
    val["port"] = port;
    val["file_pathname"] = filepath;
    val["file_size"] = filesize;
    
    data = Json::FastWriter().write(val);                       
    std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;           // 当接收方收到该消息，需要创建一个线程，服务器也要等待接收方的TCP连接
    // write_ret = bufferevent_write(to_bev, data.c_str(), strlen(data.c_str()));
    write_ret = send(to_bev->ev_read.ev_fd, data.c_str(), strlen(data.c_str()), 0);     // 发送消息给接收方
    if(write_ret < 0) {
        std::cout << "Execute write() failure!" << std::endl;
    }

    cnt = 0;
    while(to_fd <= 0) {                                                                 // 接收方还没有开始连接，设置超时等待，阻塞住
        cnt++;
        usleep(1000 * 100);                                                             
        if(cnt == 100) {                                                                
            pthread_cancel(sendfile_th.native_handle());                                
            val.clear();
            val["cmd"] = "send_file_reply";
            val["result"] = "timeout";
            
            data = Json::FastWriter().write(val);                       
            std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
            int write_ret = bufferevent_write(to_bev, data.c_str(), strlen(data.c_str()));
            if(write_ret < 0) {
                std::cout << "Execute bufferevent_write failure!" << std::endl;
            }
            std::cout << "文件接收方，连接超时！" << std::endl;
            return;
        }
    }
}

void Server::user_offline(struct bufferevent* bev, Json::Value value)
{
    m_db->db_connect("funcloud_userinfo_db");
    std::string usernmae = value["user"].asString();
    for(std::list<User>::iterator it = chatlist->online_user->begin();             
        it != chatlist->online_user->end(); it++) {                                      // 向在线用户链表中删除当前用户
        if(it->username == usernmae) {
            chatlist->online_user->erase(it);
            break;
        }
    }   
    std::string friends_info = m_db->db_get_friendslist(usernmae);                       // 获得好友及群组列表并返回                                               
    std::string fri_name;                                                                // 向该用户的所有在线的好友发送上线提醒
    int start = 0, end = 0;
    int flag = true;
    while(flag) {
        end = friends_info.find('|', start);                                             // 每次截取一个好友成员子串
        if(end == -1) {
            fri_name = friends_info.substr(start, friends_info.size() - start);          // 提取最后一个好友，无'|'分隔
            flag = false;
        }
        else {
            fri_name = friends_info.substr(start, end - start);                          // 获得一个好友姓名
        }
        
        for(std::list<User>::iterator it = chatlist->online_user->begin();               // 遍历当前所有在线的用户
            it != chatlist->online_user->end(); it++) {
            if(it->username == fri_name) {                                               // 找到了一个在线好友
                Json::Value val;
                val["cmd"] = "friend_offline_reply";
                val["user"] = usernmae;
                
                std::string data = Json::FastWriter().write(val);                       
                std::cout << "\nTo send user json data: \n" << data << '\n' << std::endl;
                int write_ret = bufferevent_write(it->bev, data.c_str(), strlen(data.c_str()));
                if(write_ret < 0) {
                    std::cout << "Execute bufferevent_write failure!" << std::endl;
                }
            }
        }
        start = end + 1;
    }
    m_db->db_disconnect();
}
