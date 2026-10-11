#include "../inc/Client.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"

Client::Client():_fd(-1), _server(NULL){}

Client::Client(int fd, const ServerConfig *server, Request request, Response response):_fd(fd), _server(server), _request(request),_response(response){}

Client::~Client(){}

int Client::getClientFd()
{
	return _fd;
}

Request& Client::getRequest()
{
	return _request;
}

const ServerConfig&	Client::getClientServer()
{
	return *_server;
}

Response& Client::getResponse()
{
	return _response;
}