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
    config_file = "./config.ini";
    resources_dir = "./resources";
    logs_dir = "./logs";
}

Config::~Config()
{
}

void Config::parse_cmd_arg(int argc, char *argv[])
{
    // 第一步：先从命令行中提取 -c 或 --config 指定的配置文件路径
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-c" || arg == "--config")
        {
            if (i + 1 < argc)
            {
                config_file = argv[++i];
            }
            else
            {
                std::cerr << "Error: --config requires a file path." << std::endl;
                print_usage(argv[0]);
                exit(1);
            }
        }
        else if (arg == "-h" || arg == "--help")
        {
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
    const char *str = "p:l:t:c:";
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
        case 'c':
        {
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
    auto config = parse_ini(config_file);

    if (config.count("port"))
    {
        auto value = config.find("port")->second;
        port = std::atoi(value.c_str());
    }

    if (config.count("timeoutMS"))
    {
        auto value = config.find("timeoutMS")->second;
        timeoutMS = std::atoi(value.c_str());
    }

    if (config.count("threadNum"))
    {
        auto value = config.find("threadNum")->second;
        threadNum = std::atoi(value.c_str());
    }

    if (config.count("logLevel"))
    {
        auto value = config.find("logLevel")->second;
        logLevel = std::atoi(value.c_str());
    }

    if (config.count("logQueSize"))
    {
        auto value = config.find("logQueSize")->second;
        logQueSize = std::atoi(value.c_str());
    }

    if (config.count("resources_dir"))
    {
        resources_dir = config.find("resources_dir")->second;
    }

    if (config.count("logs_dir"))
    {
        logs_dir = config.find("logs_dir")->second;
    }

    // for (const auto& [key, value] : config) {
    //     std::cout << key << " = " << value << std::endl;
    // }

    return false;
}

std::string trim(const std::string &str)
{
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    if (start == std::string::npos)
        return ""; // 全是空白
    return str.substr(start, end - start + 1);
}

std::unordered_map<std::string, std::string> Config::parse_ini(const std::string &filename)
{
    std::ifstream infile(filename);
    std::unordered_map<std::string, std::string> config;
    std::string line;

    if (!infile.is_open())
    {
        std::cerr << "Error: Failed to open Config file." << std::endl;
        return config;
    }

    if (!std::getline(infile, line))
    {
        std::cerr << "Error: File is empty." << std::endl;
        return config;
    }

    // 检查第一行是否为 [server]
    std::string trimmed_line = trim(line);
    if (trimmed_line != "[server]")
    {
        std::cerr << "Error: First line must be '[server]', found: '" << trimmed_line << "'" << std::endl;
        return config;
    }

    while (std::getline(infile, line))
    {
        trimmed_line = trim(line);
        // 跳过空行和注释行
        if (trimmed_line.empty() || trimmed_line[0] == '#')
        {
            continue;
        }
        // 查找等号
        size_t separator = trimmed_line.find('=');
        if (separator != std::string::npos)
        {
            std::string key = trim(trimmed_line.substr(0, separator));
            std::string value = trim(trimmed_line.substr(separator + 1));

            if (!key.empty() && !value.empty())
            {
                config[key] = value;
            }
        }
    }

    return config;
}
