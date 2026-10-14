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
#include <cerrno>

using namespace std;

enum RouteAction
{
	R_CGI,
	R_ERROR,
	R_STATIC,//R_Static means: 1.Read the file at route.fsPath 2.Put its contents into the HTTP response body 3.Set headers such as Content-Type: image/jpeg and Content-Length
	R_UPLOAD,
	R_REDIRECT,
	R_AUTOINDEX,//The autoindex handler builds an HTML response with clickable links to the files and subdirectories.
	R_DELETE
};

struct Route
{
	RouteAction		action = R_ERROR;
	httpStatus		status = OK;
	const Location*	loc = NULL;
	string			fsPath{};       // resolved path on disk
	string			interpreter{};  // R_CGI only, e.g. /usr/bin/python3
	string			redirectTo{};   // R_REDIRECT only
};
//src/serverProcess/router.cpp 
int processRoute(Client& client, ServerInfo&, Route& route);

int sendResponse(int clientFd, Response &response);
int receiveRequest(int clientFd, Request &request);
int eventLoop(ServerInfo &eloop);
void createSockAddr(const vector<ServerConfig> &server, ServerInfo &eloop);
int server(const vector<ServerConfig> &servers);

