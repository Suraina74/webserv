#include "../inc/CGI.hpp"

CGI::CGI():_inFd(-1), _outFd(-1), _clientFd(-1){}

CGI::CGI(Request &request, const ServerConfig &server, int clientFd)
{
	_scriptPath = request.getPath();
}

void CGI::populateEnv(Request &request, const ServerConfig &server)
{
	_env[] = 
}