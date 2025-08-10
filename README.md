
# Introduction
This is a project named C++ WebServer for Learning.



# Build Method
``` bash
# 方式一: 执行便捷脚本
sh start_server.sh

# 清理操作
sh start_server.sh clean
```
``` bash
# 方式二: 手动构建
mkdir build
cd build/
cmake ..
make
# 确保resources文件夹在工作目录（当前运行程序的目录）
cd ..
./build/bin/server
```

# TODO List

[x] Config INI File Support

[x] CGI Support

[ ] Epoll Support

[ ] Write Unit Tests
