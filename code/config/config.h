#ifndef CONFIG_H
#define CONFIG_H

#include <unistd.h>
#include <cstdlib>
#include <iostream>

class Config
{
public:
    Config();
    ~Config();

    // 解析命令行参数
    void parse_cmd_arg(int argc, char *argv[]);

    // 端口
    int port;
    // 超时时间
    int timeoutMS;
    // 连接池数量
    int connPoolNum;
    // 线程池数量
    int threadNum;
    // 日志开关
    bool openLog;
    // 日志等级
    int logLevel;
    // 日志异步队列容量
    int logQueSize;

    // 配置文件（可以从命令行参数指定）
    std::string config_file;

    // 静态资源目录
    std::string resources_dir;

    // 日志目录
    std::string logs_dir;

private:
    // 从ini配置文件读取配置
    bool load_from_ini();
    // 检查参数配置
    bool check_config() const;

    void parse_cmd_args(int argc, char *argv[]);

    void print_usage(const char* progName);
};

#endif // CONFIG_H