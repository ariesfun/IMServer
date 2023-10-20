## IMServer 简易的即时通讯系统

### 服务器环境配置
Linux 服务器环境：`Ubuntu 20.04`

程序依赖的环境：`libevent`，`libjsoncpp`

```shell
# 如果没有以上软件环境，需要先安装
sudo apt update
sudo apt-get install libevent-dev
sudo apt-get install libjsoncpp-dev

# 验证环境是否已经安装
dpkg -l | grep libevent 
dpkg -l | grep libjsoncpp
```

### 运行服务器
```shell
mkdir build
cd build && cmake .. && make
./main
```

### 客户端运行示例

<img src="https://imgbed-funblog.oss-cn-chengdu.aliyuncs.com/img/image-20231020090414569.png" alt="image-20231020090414569" style="zoom: 67%;" />


<img src="https://imgbed-funblog.oss-cn-chengdu.aliyuncs.com/img/image-20231020090331453.png" alt="image-20231020090331453" style="zoom: 50%;" />


<img src="https://imgbed-funblog.oss-cn-chengdu.aliyuncs.com/img/image-20231020090503273.png" alt="image-20231020090503273" style="zoom: 58%;" />