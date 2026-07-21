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

class Server 
{
	private:
		//serv socket
	public:
		Server();
		Server(const Server& other);
		Server& operator=(const Server& other);
		~Server();
};