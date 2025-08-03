#include "./server/webserver.h"
#include "./config/config.h"

int main(int argc, char *argv[])
{
    Config config;
    config.parse_cmd_arg(argc, argv);

    std::cout << "Server started on port " << config.port << std::endl;

    WebServer server(config.port, config.timeoutMS, config.connPoolNum,
                     config.threadNum, config.openLog, config.logLevel, config.logQueSize,
                     "./resources",
                     "./logs");
    server.Start();

    return 0;
}
