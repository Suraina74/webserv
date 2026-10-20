#pragma once
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
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
		int 				receiveRequest(ServerInfo &eloop);
		int 				sendResponse();
		string& 			getFullResponse();//check with http_parser branch


		void				setResponse();
		int					getClientFd();
		const ServerConfig& getClientServer();
		Request& 			getRequest();
		Response&			getResponse();
};
