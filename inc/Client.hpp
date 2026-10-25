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
		int					_timeout;
		Request				_request;
		Response			_response;
		string				_responsesString{};

	public:
		Client();
		Client(int fd, const ServerConfig *server, Request request, Response response);
		~Client();

		int					getTimeout();
		int					getFd();
		void 				receiveRequest(ServerInfo &eloop);
		void 				handleRequestLine();
		void 				handleHeaders(ssize_t n, size_t delimiter);
		void 				transferEncodingRequest();
		void 				contentLengthRequest(size_t delimiter, ssize_t n);
		void 				actionsRequestLeft(size_t startRequest);
		int 				sendResponse();
		string& 			getFullResponse();//check with http_parser branch


		void				setTimeout(int newTimeout);
		void				setResponse();
		int					getClientFd();
		const ServerConfig& getClientServer();
		Request& 			getRequest();
		Response&			getResponse();
};
