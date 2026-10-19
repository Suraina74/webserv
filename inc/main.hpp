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
const Location*	findLocation(const ServerConfig& server, const Request& request);
httpStatus		validateRouteStat(Request& request, const ServerConfig& server, Route& route);
bool			checkPathStat(string& pathName, Route& route, struct stat& pathStat);
void			setRouteAction(Route& route, Request& request, const ServerConfig& server);
//route action handlers called by processRoute(), each fills client.getResponse()
void			runCGI(Client& client, Route& route);
void			runUpload(Client& client, Route& route);
void			runRedirect(Client& client, Route& route);
void			runAutoIndex(Client& client, Route& route);
void			runDelete(Client& client, Route& route);
void			runStatic(Client& client, Route& route);
int				processRoute(Client& client, ServerInfo& eloop, Route& route);


int eventLoop(ServerInfo &eloop);
void createSockAddr(const vector<ServerConfig> &server, ServerInfo &eloop);
int server(const vector<ServerConfig> &servers);

