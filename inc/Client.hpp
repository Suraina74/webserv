#pragma once
#include "../inc/Response.hpp"
#include "../inc/Config.hpp"
#include <poll.h>

using namespace std;

class ServerInfo;

class Client
{
	private:
		int					_fd;
		const ServerConfig*	_server;
		Request				_request;
		Response			_response;
		string				_responsesString{};

	public:
		Client();
		Client(int fd, const ServerConfig *server, Request request, Response response);
		~Client();

		int					getFd();
		void 				receiveRequest(ServerInfo &eloop);
		void 				handleRequestLine(ServerInfo &eloop, const ServerConfig& server);
		void 				handleHeaders(ssize_t n, size_t delimiter, ServerInfo &eloop, const ServerConfig& server);
		void 				transferEncodingRequest(ServerInfo &eloop, const ServerConfig& server);
		void 				contentLengthRequest(size_t delimiter, ssize_t n, ServerInfo &eloop, const ServerConfig& server);
		void 				actionsRequestLeft(size_t startRequest);
		int 				sendResponse();
		string& 			getFullResponse();//check with http_parser branch


		void				setResponse();
		int					getClientFd();
		const ServerConfig& getClientServer();
		Request& 			getRequest();
		Response&			getResponse();
};

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

int	processRoute(Client& client, ServerInfo& eloop, Route& route);
//src/serverProcess/router.cpp
const Location*	findLocation(const ServerConfig& server, const Request& request);
httpStatus		validateRouteStat(Request& request, const ServerConfig& server, Route& route);
bool			checkPathStat(string& pathName, Route& route, struct stat& pathStat);
void			setRouteAction(Route& route, Request& request, const ServerConfig& server);
//route action handlers called by processRoute(), each fills client.getResponse()
void			runCGI(Request &request, const ServerConfig &server, int clientFd);
void 			runUpload(Request &request, Route &route);
void			runRedirect(Request &request, const ServerConfig &server, int clientFd);
void			runAutoIndex(Request &request, const ServerConfig &server, int clientFd);
void			runDelete(Request &request, Route &route);
void			runStatic(Request &request, const ServerConfig &server, Route &route, Client &client);