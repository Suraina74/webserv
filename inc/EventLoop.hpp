#pragma once
#include <poll.h>
#include <vector>
#include <iostream>
#include "../src/configParser/ServerConfig.hpp"
#define TIMEOUT 60

using namespace std;

class EventLoop
{
	public:
		struct addrinfo *result;
		vector<pollfd>	fds;
		vector<size_t>	configIndexes;
		vector<int>		listenFds;
};

int server(const vector<ServerConfig> &servers);
int eventLoop(const vector<int> &sockfds, const vector<ServerConfig> &servers);
vector<int> createSockAddr(struct addrinfo *result, const vector<ServerConfig> &servers);