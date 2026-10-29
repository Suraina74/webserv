#include "Client.hpp"
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>

using namespace std;

int eventLoop(ServerInfo &eloop);
void createSockAddr(const vector<ServerConfig> &server, ServerInfo &eloop);
int server(const vector<ServerConfig> &servers);

