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

enum RouteAction
{
	R_CGI,
	R_Error,
	R_Static,
	R_Upload,
	R_REDIRECT,
	R_AUTOINDEX,
	R_Delete
};

struct Route
{
	RouteAction		action = R_Error;
	httpStatus		status = OK;
	const Location*	loc = NULL;
	string			fsPath;       // resolved path on disk
	string			interpreter;  // R_CGI only, e.g. /usr/bin/python3
	string			redirectTo;   // R_REDIRECT only
};

const Location*	findLocation(const ServerConfig& server, const Request &request);
Route			routing(const ServerConfig& server, const Request &request);
bool			validateRoute(const ServerConfig& server, const Request& request, Route& route);

int sendResponse(int clientFd, Response &response);
int receiveRequest(int clientFd, Request &request);
int eventLoop(ServerInfo &eloop);
void createSockAddr(const vector<ServerConfig> &server, ServerInfo &eloop);
int server(const vector<ServerConfig> &servers);

