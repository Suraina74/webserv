#pragma once 
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <poll.h>
#include <vector>
#include <map>
#include <set>
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