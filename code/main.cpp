#include "./server/webserver.h"

using namespace std;

int main()
{
    WebServer server(
        3000,
        60000,
        false,
        12,
        6,
        true,
        1,
        1024
    );
    server.Start();
    return 0;
}
