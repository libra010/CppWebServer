#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>  // fcntl()
#include <unistd.h> // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../log/log.h"
#include "../timer/heaptimer.h"
#include "../pool/threadpool.h"
#include "../http/httpconn.h"

class WebServer
{
public:
    WebServer(
        int port,        // 端口
        int timeoutMS,   // 超时事件
        int connPoolNum, // 连接池数量
        int threadNum,   // 线程池数量
        bool openLog,    // 日志开关
        int logLevel,    // 日志等级
        int logQueSize,  // 日志异步队列容量
        const char *srcDir,
        const char *logDir);
    ~WebServer();
    void Start();

    // void DealListen_();
    // void DealWrite_(HttpConn* client);
    // void DealRead_(HttpConn* client);

    // void SendError_(int fd, const char*info);
    // void ExtentTime_(HttpConn* client);
    // void CloseConn_(HttpConn* client);

    // void OnRead_(HttpConn* client);
    // void OnWrite_(HttpConn* client);
    // void OnProcess(HttpConn* client);

private:
    // 端口
    int port_;
    // 优雅退出
    bool openLinger_;
    // 超时时间
    int timeoutMS_;
    bool isClose_;
    int listenFd_;
    // char* srcDir_;
    const char *srcDir_;
    const char *logDir_;
    std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<ThreadPool> threadpool_;
    std::unordered_map<int, HttpConn> users_;
};

#endif // WEBSERVER_H