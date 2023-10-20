#include "chat_db.h"
#include <iostream>
#include <cstring>

ChatDB::ChatDB() {}

ChatDB::~ChatDB()
{
    mysql_close(m_mysql);
}

void ChatDB::db_connect(std::string db_name)
{
    m_mysql = mysql_init(nullptr);
    m_mysql = mysql_real_connect(m_mysql, "127.0.0.1", "root", "root", db_name.c_str(), 0, nullptr, 0);
    if(m_mysql == nullptr) {
        std::cout << "Connect database failure!" << std::endl;
        return;
    }
    if(mysql_query(m_mysql, "set names utf8;") != 0) {                          // 设置中文字符集编码，防止查询出现的是中文群名有乱码的情况
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }
}

int ChatDB::db_get_groupname(std::string* group_name)
{
    if(mysql_query(m_mysql, "show tables;") != 0) {                             // 查询当前数据库中的所有表格
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }

    MYSQL_RES* res = mysql_store_result(m_mysql);                               // 获得查询结果
    if(res == nullptr) {
        std::cout << "Execute mysql_store_result() error!" << std::endl;
        return 0;
    }

    int cnt = 0;
    MYSQL_ROW row;
    while(row = mysql_fetch_row(res)) {                                         // 存储群组名
        group_name[cnt] += row[0];
        cnt++;
    }
    return cnt;
}

void ChatDB::db_get_groupmember(std::string tb_name, std::string &member_list)
{
    char sql[1024] = {0};
    sprintf(sql, "select member from %s;", tb_name.c_str());
    if(mysql_query(m_mysql, sql) != 0) {
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }

    MYSQL_RES* res = mysql_store_result(m_mysql);                               // 获得查询结果
    if(res == nullptr) {
        std::cout << "Execute mysql_store_result() error!" << std::endl;
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);                                       // 一个群组对应一张表，获取一行记录就行，成员信息只是其中一行记录
    if(row[0] == nullptr) {                                                     // 当前群组没有成员
        return;
    }
    member_list += row[0];
}

void ChatDB::db_disconnect()
{
    mysql_close(m_mysql);
}

bool ChatDB::verify_user_isexist(std::string username)
{
    char sql[128] = {0};
    sprintf(sql, "show tables like '%s';", username.c_str());
    if(mysql_query(m_mysql, sql) != 0) {
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }

    MYSQL_RES* res = mysql_store_result(m_mysql);                               // 获得查询结果
    if(res == nullptr) {
        std::cout << "Execute mysql_store_result() error!" << std::endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res); 
    if(row != nullptr) {                                                        // 查询结果不空，说明用户已经存在
        return true;
    }
    else {
        return false;
    }
}

void ChatDB::new_user_register(std::string username, std::string password)
{
    char sql[128] = {0};
    sprintf(sql, "create table %s (password varchar(16), friends varchar(4096), chatgroups varchar(4096));", username.c_str()); // 先创建用户表
    if(mysql_query(m_mysql, sql) != 0) {
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }

    memset(sql, 0, sizeof(sql));
    sprintf(sql, "insert into %s (password) values ('%s');", username.c_str(), password.c_str());   // 初始化用户数据信息，好友为空
    if(mysql_query(m_mysql, sql) != 0) {
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }
}

bool ChatDB::verify_user_password(std::string username, std::string password)
{
    char sql[128] = {0};
    sprintf(sql, "select password from %s;", username.c_str());                                     // 获得该用户对应的密码
    if(mysql_query(m_mysql, sql) != 0) {
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }

    MYSQL_RES* res = mysql_store_result(m_mysql);                                                   // 获得查询结果
    if(res == nullptr) {
        std::cout << "Execute mysql_store_result() error!" << std::endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res); 
    if(row[0] == password) {                                                                        // 校验密码是否正确
        return true;
    }
    else {
        return false;
    }
}

std::string ChatDB::db_get_friendslist(std::string username)
{
    char sql[128] = {0};
    sprintf(sql, "select friends from %s;", username.c_str());                                     // 获得该用户的好友列表
    if(mysql_query(m_mysql, sql) != 0) {
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }

    std::string result;
    MYSQL_RES* res = mysql_store_result(m_mysql);                                                  // 获得查询结果
    if(res == nullptr) {
        std::cout << "Execute mysql_store_result() error!" << std::endl;
        return "";
    }
    MYSQL_ROW row = mysql_fetch_row(res); 
    if(row[0] == nullptr) {
        mysql_free_result(res);   // TODO 需要free()释放查询的结果集，可设为类成员变量                                                                        
        return "";
    }
    else {
        result.append(row[0]);
        mysql_free_result(res);
        return result;
    }
}

std::string ChatDB::db_get_groupslist(std::string username)
{
    char sql[128] = {0};
    sprintf(sql, "select chatgroups from %s;", username.c_str());                                     // 获得该用户的群组列表
    if(mysql_query(m_mysql, sql) != 0) {
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }

    std::string result;
    MYSQL_RES* res = mysql_store_result(m_mysql);                                                  // 获得查询结果
    if(res == nullptr) {
        std::cout << "Execute mysql_store_result() error!" << std::endl;
        return "";
    }
    MYSQL_ROW row = mysql_fetch_row(res); 
    if(row[0] == nullptr) {           
        mysql_free_result(res);                                                             
        return "";
    }
    else {
        result.append(row[0]);
        mysql_free_result(res);   
        return result;
    }
}

bool ChatDB::verify_is_friends(std::string username, std::string new_friend)
{
    char sql[1024] = {0};
    sprintf(sql, "select friends from %s;", username.c_str());
    if(mysql_query(m_mysql, sql) != 0) {
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }

    MYSQL_RES* res = mysql_store_result(m_mysql);                               
    if(res == nullptr) {
        std::cout << "Execute mysql_store_result() error!" << std::endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res); 
    if(row[0] == nullptr) {                                                                        // 还不是好友                                                    
        return false;
    }     
    else {                                                                                         // 相当于遍历该用户的好友列表
        // TODO,后续优化可以把登录成功后，获取到的一个所有好友姓名保存到set容器中
        std::string user_friends_list(row[0]);
        int start = 0, end = 0;
        while(true) {
            end = user_friends_list.find('|', start);
            if(end == -1) {
                break;
            }
            if(user_friends_list.substr(start, end - start) == new_friend) {
                return true;
            }
            start = end + 1;
        }
        if(user_friends_list.substr(start, user_friends_list.size() - start) == new_friend) {
            return true;
        }
    }
    return false;
}

void ChatDB::new_friend_add(std::string username, std::string new_add_friend)
{
    std::string friend_list = db_get_friendslist(username);                     // 先要获取到当前用户的好友列表
    if(friend_list.empty()) {                                                   // 空列表时，直接添加
        friend_list.append(new_add_friend);
    }
    else {
        friend_list.append("|");
        friend_list.append(new_add_friend);
    }

    char sql[1024] = {0};                                                       // 更新数据库中的信息
    sprintf(sql, "update %s set friends = '%s';", username.c_str(), friend_list.c_str());
    if(mysql_query(m_mysql, sql) != 0) {
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }
}

bool ChatDB::verify_group_isexist(std::string groupname)
{
    char sql[128] = {0};
    sprintf(sql, "show tables like '%s';", groupname.c_str());
    if(mysql_query(m_mysql, sql) != 0) {
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }

    MYSQL_RES* res = mysql_store_result(m_mysql);                               // 获得查询结果
    if(res == nullptr) {
        std::cout << "Execute mysql_store_result() error!" << std::endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res); 
    if(row != nullptr) {                                                        // 查询结果不空，说明群组已经存在
        return true;
    }
    else {
        return false;
    }
}

void ChatDB::new_group_create(std::string groupname, std::string username)
{
    char sql[128] = {0};
    sprintf(sql, "create table %s (owner varchar(32), member varchar(4096)) character set utf8;", groupname.c_str());
    if(mysql_query(m_mysql, sql) != 0) {
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }
    memset(sql, 0, sizeof(sql));                                                 // 初始化群信息，群组及群成员(群主是第一个成员)
    sprintf(sql, "insert into %s values ('%s', '%s');", groupname.c_str(), username.c_str(), username.c_str());
    if(mysql_query(m_mysql, sql) != 0) {
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }
}

void ChatDB::user_add_groupinfo(std::string username, std::string groupname)
{   
    char sql[1024] = {0};
    sprintf(sql, "select chatgroups from %s;", username.c_str());
    if(mysql_query(m_mysql, sql) != 0) {
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }
    MYSQL_RES* res = mysql_store_result(m_mysql);                               // 获得查询结果
    if(res == nullptr) {
        std::cout << "Execute mysql_store_result() error!" << std::endl;
    }
    std::string user_grouplist;
    MYSQL_ROW row = mysql_fetch_row(res); 
    if(row[0] != nullptr) {                                                     // 查询结果不空，说明已经有群组信息
        user_grouplist.append(row[0]);
        user_grouplist.append("|");
        user_grouplist.append(groupname);
    }
    else {
        user_grouplist.append(groupname);
    }
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "update %s set chatgroups = '%s';", username.c_str(), user_grouplist.c_str());
    if(mysql_query(m_mysql, sql) != 0) {
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }
}

void ChatDB::group_add_memberinfo(std::string groupname, std::string membername)
{
    char sql[1024] = {0};
    sprintf(sql, "select member from %s;", groupname.c_str());
    if(mysql_query(m_mysql, sql) != 0) {
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }
    MYSQL_RES* res = mysql_store_result(m_mysql);                               // 获得查询结果
    if(res == nullptr) {
        std::cout << "Execute mysql_store_result() error!" << std::endl;
    }
    std::string group_memberlist;
    MYSQL_ROW row = mysql_fetch_row(res); 
    if(row[0] != nullptr) {                                                     // 查询结果不空，说明已经有群组成员信息
        group_memberlist.append(row[0]);
        group_memberlist.append("|");
        group_memberlist.append(membername);
    }
    else {
        group_memberlist.append(membername);
    }
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "update %s set member = '%s';", groupname.c_str(), group_memberlist.c_str());
    if(mysql_query(m_mysql, sql) != 0) {
        printf("[ERROR] - [file: %s, func: %s, line: %d] info: ", basename(__FILE__),  __FUNCTION__, __LINE__);
        std::cout << "Execute mysql_query() error!" << std::endl;
    }
}
