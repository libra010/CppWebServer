#include "./server/webserver.h"
#include "./config/config.h"

int main(int argc, char *argv[])
{
    Config config;
    config.parse_cmd_arg(argc, argv);

    char buffer[1024];
    if (getcwd(buffer, sizeof(buffer)) != nullptr)
    {
        std::string current_path(buffer);
        std::cout << "Server started on port: " << config.port << std::endl;
        std::cout << "Config timeoutMS is: " << config.timeoutMS << std::endl;
        std::cout << "Config threadNum is: " << config.threadNum << std::endl;
        std::cout << "Config logLevel is: " << config.logLevel << std::endl;
        std::cout << "Config logQueSize is: " << config.logQueSize << std::endl;
        std::cout << "work dictionary in \"" << current_path << "\"" << std::endl;
        std::cout << "Resources dictionary in \"" << config.resources_dir << "\"" << std::endl;
        std::cout << "Logs dictionary in \"" << config.logs_dir << "\"" << std::endl;
    }
    else{
        return 1;
    }
    
    WebServer server(config.port, config.timeoutMS, config.connPoolNum,
                     config.threadNum, config.openLog, config.logLevel, config.logQueSize,
                     config.resources_dir.c_str(),
                     config.logs_dir.c_str());
    server.Start();

    return 0;
}
