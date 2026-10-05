#include "ServerConfig.hpp"
#include "ServerInfo.hpp"
#include "Config.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Client.hpp"
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

int sendResponse(int clientFd, Response &response);
int receiveRequest(int clientFd, Request &request);
int eventLoop(ServerInfo &eloop);
void createSockAddr(const vector<ServerConfig> &server, ServerInfo &eloop);
int server(const vector<ServerConfig> &servers);

