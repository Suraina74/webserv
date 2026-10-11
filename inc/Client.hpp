#pragma once
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
#include <poll.h>

using namespace std;

class Client
{
	private:
		int					_fd;
		const ServerConfig*	_server;
		Request				_request;
		Response			_response;

	public:
		Client();
		Client(int fd, const ServerConfig *server, Request request, Response response);
		~Client();
		int					getClientFd();
		const ServerConfig& getClientServer();
		Request& 			getRequest();
		Response&			getResponse();
};