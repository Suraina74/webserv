#include "../../inc/main.hpp"
#include "../../inc/CGI.hpp"

void runCGI(Request &request, const ServerConfig &server, int clientFd)
{
    CGI newCGI = CGI(request, server, clientFd);
    
}

void runUpload(Request &request, Route &route)
{
	if (request.getContentLength() > 0 || request.getChunked() == true){
			request.extractFileElements();
			request.setStatusText(request.getStatusCode());
			if (request.getStatusCode() == OK)
			{
				request.addFile(route.fsPath);
			}
		}
}

void runRedirect(Request &request, const ServerConfig &server, int clientFd)
{
	(void)request;
	(void)server;
	(void)clientFd;

}

void runAutoIndex(Request &request, const ServerConfig &server, int clientFd)
{
	(void)request;
	(void)server;
	(void)clientFd;

}

void runDelete(Request &request, const ServerConfig &server, int clientFd)
{
	(void)request;
	(void)server;
	(void)clientFd;

}

void runStatic(Request &request, const ServerConfig &server, int clientFd)
{
	(void)request;
	(void)server;
	(void)clientFd;

}