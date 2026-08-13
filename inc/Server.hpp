#pragma once 
#include "Config.hpp"
#include "ServerConfig.hpp"

class Server 
{
	private:
		int				_listenFd;
		ServerConfig	_server;

	public:
		Server();
		~Server();

		void start();
		void stop();

};

int server();
int	eventLoop(int *sockfd);
int	createSockAddr(int *sockfd, struct addrinfo *result);