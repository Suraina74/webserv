#pragma once
#include <poll.h>
#include <vector>
#include <iostream>
#include "../src/configParser/ServerConfig.hpp"
#define TIMEOUT 60

using namespace std;

class ServerInfo
{
private:
	struct addrinfo *_result;
	vector<pollfd> _fds;
public:
	void setResult(struct addrinfo *setRes);
	void setFd(vector<pollfd> setFd);

	struct addrinfo *getResult();
	vector<pollfd> &getFd();
};

int server(const vector<ServerConfig> &servers);
int eventLoop(const vector<int> &sockfds, const vector<ServerConfig> &servers);
vector<pollfd> createSockAddr(struct addrinfo *result, const vector<ServerConfig> &servers);