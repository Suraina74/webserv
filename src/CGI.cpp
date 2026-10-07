#include "../inc/CGI.hpp"

CGI::CGI():_env("",""), _interpreter(""), _scriptPath(""), _requestBody(""), _bodySent(0), _output(""), _clientFd(-1), _inFd(-1), _outFd(-1), _pid(-1) {}

CGI::CGI(Request &request, const ServerConfig &server, int clientFd)
{
	CGI();
	vector<Location> allLocations = server.getLocations();
	for (int i = 0; i < allLocations.size(); i++)
	{
		//if found cgi, get interpreter val
		//else all destructor
	}
	_scriptPath = request.getPath();
	_requestBody = request.getBody();
	_clientFd = clientFd;
}

void CGI::populateEnv(Request &request, const ServerConfig &server)
{
	
}