#pragma once
#include <mysql/mysql.h>
#include <string>

class ChatDB {
public:
    ChatDB();
    ~ChatDB();
    
public: // 数据库相关操作的API
    void db_connect(std::string db_name);                                       // 连接数据库
    int db_get_groupname(std::string* group_name);                              // 字符串数组，保存群名信息及返回群组个数
    void db_get_groupmember(std::string tb_name, std::string &member_list);     // 获得具体群组中的所有群成员
    void db_disconnect();                                                       // 断开数据库连接

    bool verify_user_isexist(std::string username);                             // 检验用户是否已经存在
    bool verify_user_password(std::string username, std::string password);      // 检验用户密码是否正确
    void new_user_register(std::string username, std::string password);         // 执行新用户注册操作

    std::string db_get_friendslist(std::string username);                       // 获得当前好友列表信息
    std::string db_get_groupslist(std::string username);                        // 获得当前好友群组信息
    bool verify_is_friends(std::string username, std::string new_friend);       // 验证两人是否为好友好关系
    void new_friend_add(std::string username, std::string new_add_friend);      // 当前用户添加新好友

    bool verify_group_isexist(std::string groupname);                           // 检验群组是否已经存在
    void new_group_create(std::string groupname, std::string username);         // 当前用户创建新群组
    void user_add_groupinfo(std::string username, std::string groupname);       // 当前用户添加群组
    void group_add_memberinfo(std::string groupname, std::string membername);   // 更新群组的成员信息

private:
    MYSQL* m_mysql;
};