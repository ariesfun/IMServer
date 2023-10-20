#include "chat_info_list.h"
#include <iostream>

ChatInfo::ChatInfo()
{
    online_user = new std::list<User>;
    group_info = new std::list<Group>;

    m_db = new ChatDB;
    m_db->db_connect("funcloud_chatgroup_db");                  // 获得一个数据库连接

    std::string group_name[MAX_GOROUP_NUM];
    int group_num = m_db->db_get_groupname(group_name);

    for(int i = 0; i < group_num; i++) {                        // 存储群组信息, 向group_info中添加群信息
        Group g;
        g.group_name = group_name[i];
        g.group_list = new std::list<GroupUser>;                // 保存群中所有用户
        
        group_info->push_back(g);                               // 保存一个群组成员信息

        std::string member_list;
        m_db->db_get_groupmember(group_name[i], member_list);   // 每次获得一个群组中的成员信息member_list
        if(member_list.empty()) {                               // 没有成员时，跳过这次循环
            continue;
        }
        GroupUser g_user;
        int start = 0, end = 0;
        while(true) {
            end = member_list.find('|', start);                 // 每次截取一个群成员子串
            if(end == -1) {
                break;
            }
            g_user.group_username = member_list.substr(start, end - start);
            g.group_list->push_back(g_user);                    // 保存群成员信息
            start = end + 1;
            g_user.group_username.clear();
        }
        // 提取最后一个群成员，无'|'分隔
        g_user.group_username = member_list.substr(start, member_list.size() - start);
        g.group_list->push_back(g_user);

    }
    std::cout << "\nInitialize user linked list information successfully!" << std::endl;
    m_db->db_disconnect();

    // 用户链表信息打印测试
    for(std::list<Group>::iterator it = group_info->begin(); it != group_info->end(); it++) {
        std::cout << "群组名称：" << it->group_name << std::endl;
        for(std::list<GroupUser>::iterator it_user = it->group_list->begin(); it_user != it->group_list->end(); it_user++) {
            std::cout << "群成员：" << it_user->group_username << std::endl;
        }
    }
}

ChatInfo::~ChatInfo() {}

bool ChatInfo::verify_groupinfo_isexist(std::string groupname)
{
    for(std::list<Group>::iterator it = group_info->begin(); 
        it != group_info->end(); it++) {
        if(it->group_name == groupname) {
            return true;
        }
    }
    return false;
}

bool ChatInfo::verify_is_groupmember(std::string groupname, std::string username)
{
    for(std::list<Group>::iterator it = group_info->begin(); 
        it != group_info->end(); it++) {
        if(it->group_name == groupname) {
            for(std::list<GroupUser>::iterator it_user = it->group_list->begin(); 
                it_user != it->group_list->end(); it_user++) {
                if(it_user->group_username == username) {
                    return true;
                }
            }
        }
    }
    return false;
}

void ChatInfo::update_user_groupinfo(std::string groupname, std::string username)
{
    for(std::list<Group>::iterator it = group_info->begin(); 
        it != group_info->end(); it++) {
        if(it->group_name == groupname) {               // 找到对应的群组链表对象，添加成员
            GroupUser u;
            u.group_username = username;
            it->group_list->push_back(u);
        }
    }
}

void ChatInfo::add_new_groupinfo(std::string groupname, std::string username)
{
    GroupInfo g;
    g.group_name = groupname;
    g.group_list = new std::list<GroupUser>;
    group_info->push_back(g);

    GroupUser u;
    u.group_username = username;
    g.group_list->push_back(u);
}

struct bufferevent* ChatInfo::get_onlineuser_bev(std::string to_username)
{
    for(std::list<User>::iterator it = online_user->begin();        // 遍历在线用户链表
        it != online_user->end(); it++) {
        if(it->username == to_username) {
            return it->bev;
        }
    }
    return nullptr;
}

std::string ChatInfo::get_group_memberinfo(std::string groupname)
{
    std::string memberlist;
    for(std::list<Group>::iterator it = group_info->begin(); 
        it != group_info->end(); it++) {
        if(it->group_name == groupname) {                       // 遍历要查询的群链表信息
            std::list<GroupUser>::iterator it_user = it->group_list->begin();
            while (it_user != it->group_list->end()) {
                memberlist.append(it_user->group_username);
                it_user++;                                      // 检查是否是最后一个非空的it_user
                if (it_user != it->group_list->end()) {
                    memberlist.append("|");
                }
            }
        }
    }
    return memberlist;
}
