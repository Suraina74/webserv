#pragma once
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
#include <poll.h>

using namespace std;

class Client
{
	private:
		int					fd;
		const ServerConfig	*server;
		Request				request;
		Response			response;
	public:
		Client();
		Client(int fd, const ServerConfig *server);
		~Client();
		int				getFd();
		ServerConfig	&getClientServer();
		Request& 		getRequest();
		Response&		getResponse();
};