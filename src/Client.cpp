#include "../inc/Client.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"

Client::Client(){}

Client::Client(int fd, const ServerConfig *server, Request request, Response response):_fd(fd), _server(server), _request(request),_response(response){}

Client::~Client(){}

int Client::getFd()
{
	return _fd;
}

Request& Client::getRequest()
{
	return _request;
}

Response& Client::getResponse()
{
	return _response;
}