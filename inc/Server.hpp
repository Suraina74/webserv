#pragma once 
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <poll.h>
#include <vector>
#include <map>
#include <set>
#include <limits>
#include <sys/stat.h>
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