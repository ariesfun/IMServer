#pragma once
#include <string>
#include <event.h>
#include <list>
#include "chat_db.h"

#define MAX_GOROUP_NUM 1024

struct UserInfo                                                                                 // 定义用户信息链表的的3个节点，单个用户信息 
{
    std::string username; 
    struct bufferevent* bev;                                                                    // 客户端与服务器成功建立连接后，才会产生bev
};

struct GroupUser                                                                                // 群组中的成员信息
{
    std::string group_username;
};

struct GroupInfo                                                                                // 一个群组的信息
{
    std::string group_name;
    std::list<GroupUser>* group_list;
};

typedef struct UserInfo User;
typedef struct GroupInfo Group;

class ChatInfo {

friend class Server;

public:
    ChatInfo();
    ~ChatInfo();

public:
    bool verify_groupinfo_isexist(std::string groupname);
    bool verify_is_groupmember(std::string groupname, std::string username);
    void update_user_groupinfo(std::string groupname, std::string username);
    void add_new_groupinfo(std::string groupname, std::string username);                       // 创建新群组后，保存用户群组链表信息
    struct bufferevent* get_onlineuser_bev(std::string to_username);
    std::string get_group_memberinfo(std::string groupname);

private:
    std::list<User>* online_user;                                                               // 保存所有在线的用户信息
    std::list<Group>* group_info;                                                               // 保存所有群组信息
    ChatDB* m_db;                                                                               // 数据库对象

};