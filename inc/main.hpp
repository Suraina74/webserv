#include "../src/configParser/ServerConfig.hpp"
#include "../inc/ServerInfo.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
#include "../inc/Client.hpp"
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
int server(const vector<ServerConfig> &servers);