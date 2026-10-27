#include "../../inc/CGI.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

CGI::CGI():_env(), _interpreter(""), _scriptPath(""), _requestBody(""), _bodySent(0), _output(""), _clientFd(-1), _inFd(-1), _outFd(-1), _pid(-1) {}

CGI::~CGI(){}

CGI::CGI(Request &request, const ServerConfig &server, int clientFd, string interpreter)
{
	CGI();
	_scriptPath = request.getPath();
	_requestBody = request.getBody();
	_clientFd = clientFd;
	_interpreter = interpreter;
	populateEnv(request, server);
}

void CGI::populateEnv(Request &request, const ServerConfig &server)
{
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["SERVER_SOFTWARE"] = "webserv/1.0";
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env["SERVER_NAME"] = server.getServerName();
	_env["SERVER_PORT"] = to_string(server.getPort());
	_env["REQUEST_METHOD"] = request.getMethod();
	_env["SCRIPT_NAME"] = request.getPath().rfind('/');
	_env["QUERY_STRING"] = request.getQuery();
	_env["CONTENT_LENGTH"] = request.getContentLength();
	
	_env["CONTENT_TYPE"] = "";
	_env["CLIENT_IP_ADDR"] = "";
}
