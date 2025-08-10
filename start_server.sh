#!/bin/bash

if [ "$1" = "clean" ];
then
    rm -rf build/
    rm -rf logs/
    rm -f usertable.txt
    exit 0
fi

echo "未输入操作名, 执行“运行或自动构建”流程..."

# 检查 build/bin/server 程序是否存在
if [ -f "build/bin/server" ]; then
    echo "发现 build/bin/server, 正在执行..."
    ./build/bin/server
else
    echo "未发现 build/bin/server。"
    read -p "是否进行构建? (输入1进行构建): " user_input

    if [ "$user_input" = "1" ]; then
        # 执行构建命令
        mkdir -p build
        cd build/
        cmake .. || { echo "CMake 配置失败"; exit 1; }
        make || { echo "编译失败"; exit 1; }

        # 构建cgi文件
        # cd ../cgi_code
        # make || { echo "编译失败"; exit 1; }
        
        # 切换工作目录
        cd ../

        mkdir -p resources_cgi
        g++ -std=c++14 -O2 -Wall -g ./cgi_code/auth.cpp -o ./resources_cgi/auth.cgi

        # 构建完成后，再次检查并运行程序
        if [ -f "build/bin/server" ]; then
            echo "构建完成，正在执行 server..."
            ./build/bin/server
        else
            echo "构建失败或 server 程序未生成。"
            exit 1
        fi
    else
        echo "取消构建，程序退出。"
        exit 0
    fi
fi