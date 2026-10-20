#include "../../inc/main.hpp"
#include "../../inc/CGI.hpp"

void runCGI(Request &request, const ServerConfig &server, int clientFd)
{
    CGI newCGI = CGI(request, server, clientFd);
    
}

void runUpload(Request &request, const ServerConfig &server, int clientFd)
{

}

void runRedirect(Request &request, const ServerConfig &server, int clientFd)
{

}

void runAutoIndex(Request &request, const ServerConfig &server, int clientFd)
{

}

void runDelete(Request &request, const ServerConfig &server, int clientFd)
{

}

void runStatic(Request &request, const ServerConfig &server, int clientFd)
{

}