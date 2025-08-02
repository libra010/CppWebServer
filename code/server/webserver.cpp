#include "webserver.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

using namespace std;

WebServer::WebServer(int port, int timeoutMS, bool OptLinger, int connPoolNum, int threadNum, bool openLog, int logLevel, int logQueSize)
    : port_(port), openLinger_(OptLinger), timeoutMS_(timeoutMS), isClose_(false),
      timer_(new HeapTimer()), threadpool_(new ThreadPool(threadNum))
{
    srcDir_ = getcwd(nullptr, 256);
    assert(srcDir_);
    strncat(srcDir_, "/resources/", 16);
    HttpConn::userCount = 0;
    HttpConn::srcDir = srcDir_;
    if (openLog)
    {
        Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
        if (isClose_)
        {
            LOG_ERROR("========== Server init error!==========");
        }
        else
        {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", port_, OptLinger ? "true" : "false");
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }
}

WebServer::~WebServer()
{
    // close(listenFd_);
    isClose_ = true;
    free(srcDir_);
}

void WebServer::Start()
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        LOG_ERROR("Create socket error!");
        return;
    }

    struct sockaddr_in bindaddr;
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindaddr.sin_port = htons(port_);
    if (bind(listenfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) == -1)
    {
        LOG_ERROR("Bind error!");
        close(listenfd);
        return;
    }

    if (listen(listenfd, SOMAXCONN) == -1)
    {
        LOG_ERROR("Listen error!");
        close(listenfd);
        return;
    }

    std::cout << "Server started on port " << port_ << std::endl;

    while (!isClose_)
    {
        struct sockaddr_in clientaddr;
        socklen_t clientaddrlen = sizeof(clientaddr);
        int clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddrlen);

        if (clientfd > 0)
        {
            // 提交到线程池处理，每个连接一个线程
            threadpool_->AddTask(
                [clientfd, clientaddr, this]()
                {
                    HttpConn client;
                    client.init(clientfd, clientaddr);
                    while (client.read(nullptr) > 0 && client.process())
                    {
                        client.write(nullptr);
                        if (!client.IsKeepAlive())
                            break;
                    }
                    close(clientfd);
                });
        }

        // if (clientfd == -1)
        // {
        //     if (errno != EINTR)
        //     { // 忽略中断
        //         LOG_ERROR("Accept error: %s", strerror(errno));
        //     }
        //     continue;
        // }

        // LOG_INFO("Client connected: %d", clientfd);

        // HttpConn client;
        // client.init(clientfd, clientaddr);

        // // 7. 同步处理请求（阻塞直到完成）
        // while (client.read(nullptr) > 0)
        // { // 读取请求
        //     if (!client.process())
        //         break; // 处理请求
        //     if (client.write(nullptr) <= 0)
        //         break; // 发送响应
        //     if (!client.IsKeepAlive())
        //         break; // 非 keep-alive 就断开
        // }

        // LOG_INFO("Client disconnected: %d", clientfd);
        // close(clientfd);

        // while (true)
        // {
        //     char buf[1024];
        //     bzero(&buf, sizeof(buf));
        //     ssize_t read_bytes = read(clientfd, buf, sizeof(buf));
        //     if (read_bytes > 0)
        //     {
        //         std::cout << "message from client fd " << clientfd << "  " << buf << std::endl;
        //         write(clientfd, buf, sizeof(buf));
        //     }
        //     else if (read_bytes == 0)
        //     {
        //         std::cout << "client fd " << clientfd << " disconnected" << std::endl;
        //         close(clientfd);
        //         break;
        //     }
        //     else if (read_bytes == -1)
        //     {
        //         close(clientfd);
        //         return;
        //     }
        // }
    }

    close(listenfd);
}
