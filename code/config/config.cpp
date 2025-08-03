#include "config.h"

Config::Config()
{
    port = 3000;
    timeoutMS = 60000;
    connPoolNum = 12;
    threadNum = 6;
    openLog = true;
    logLevel = 1;
    logQueSize = 1024;
    config_file = "";
    resources_dir = "";
    logs_dir = "";
}

Config::~Config()
{
}

void Config::parse_cmd_arg(int argc, char *argv[])
{
    // 第一步：先从命令行中提取 -c 或 --config 指定的配置文件路径
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                config_file = argv[++i];
            } else {
                std::cerr << "Error: --config requires a file path." << std::endl;
                print_usage(argv[0]);
                exit(1);
            }
        } else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            exit(0);
        }
    }

    // 从配置文件加载
    load_from_ini();

    // 解析命令行参数 覆盖配置文件参数
    parse_cmd_args(argc, argv);

    // 校验配置
    if (!check_config())
    {
        std::cerr << "Invalid configuration. Server cannot start." << std::endl;
        exit(-1);
    }
}

bool Config::check_config() const
{
    bool valid = true;

    // 检查端口
    if (port < 1 || port > 65535)
    {
        std::cerr << "[ERROR] Invalid port: " << port
                  << ". Port must be in range 1-65535." << std::endl;
        valid = false;
    }

    // 检查超时时间
    if (timeoutMS < 0)
    {
        std::cerr << "[ERROR] Invalid timeoutMS: " << timeoutMS
                  << ". Must be non-negative." << std::endl;
        valid = false;
    }

    // 检查连接池数量
    if (connPoolNum <= 0)
    {
        std::cerr << "[ERROR] Invalid connPoolNum: " << connPoolNum
                  << ". Must be positive." << std::endl;
        valid = false;
    }

    // 检查线程数
    if (threadNum <= 0)
    {
        std::cerr << "[ERROR] Invalid threadNum: " << threadNum
                  << ". Must be positive." << std::endl;
        valid = false;
    }

    // 检查日志级别
    if (logLevel < 0 || logLevel > 4)
    {
        std::cerr << "[ERROR] Invalid logLevel: " << logLevel
                  << ". Valid levels: 0(OFF), 1(ERROR), 2(WARN), 3(INFO), 4(DEBUG)." << std::endl;
        valid = false;
    }

    // 检查日志队列大小
    if (logQueSize < 0)
    {
        std::cerr << "[ERROR] Invalid logQueSize: " << logQueSize
                  << ". Must be non-negative." << std::endl;
        valid = false;
    }

    return valid;
}

void Config::parse_cmd_args(int argc, char *argv[])
{
    int opt;
    const char *str = "p:l:t:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            port = atoi(optarg);
            break;
        }
        case 'l':
        {
            logLevel = atoi(optarg);
            break;
        }
        case 't':
        {
            threadNum = atoi(optarg);
            break;
        }
        default:
            break;
        }
    }
}

void Config::print_usage(const char *progName)
{
    std::cout << "Usage: " << progName << " [options]\n"
              << "Options:\n"
              << "  -c, --config <file>     Config file path (default: config.ini)\n"
              << "  -p, --port <num>            Server port (default: 3000)\n"
              << "  -l, --loglevel <num>            log level (default: 1)\n"
              << "  -t, --threadnum <num>            thread num (default: 6)\n"
              << "  -h, --help              Show this help message\n";
}

bool Config::load_from_ini()
{
    return false;
}
