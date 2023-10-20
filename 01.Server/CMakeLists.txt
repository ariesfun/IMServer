cmake_minimum_required(VERSION 3.10)
project(IMServer)

set(CMAKE_CXX_STANDARD 11)

# 添加头文件
include_directories(./code/db ./code/list ./code/server)

# 需编译的源文件
add_executable(IMServer main.cpp ./code/db/chat_db.cpp ./code/list/chat_info_list.cpp ./code/server/server.cpp  )

# 指定要链接的库
target_link_libraries(IMServer event jsoncpp pthread mysqlclient)