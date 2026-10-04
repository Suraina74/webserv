#pragma once
#include <poll.h>
#include <vector>
#include <iostream>
#include "../src/configParser/ServerConfig.hpp"
#include "Client.hpp"
#define TIMEOUT 60

using namespace std;

class ServerInfo
{
	private:
		
	public:
		map<int, Client> clients;
		vector<pollfd> fds;
		void setResult(struct addrinfo *res);
		void setClient(Client client);
		void setFd(pollfd fd);

	struct addrinfo *getResult();
	vector<pollfd> &getFd();
};

int server(const vector<ServerConfig> &servers);
int eventLoop(const vector<int> &sockfds, const vector<ServerConfig> &servers);
vector<pollfd> createSockAddr(const vector<ServerConfig> &server);