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

using std::cout;
using std::cerr;
using std::cin;

using std::endl;
using std::string;

using std::vector;
using std::map;
using std::set;

using std::ifstream;
using std::ofstream;
using std::stringstream;


class Server 
{
	private:
		int _listenFd;

	public:
		Server();
		~Server();

		void start();
		void stop();
};