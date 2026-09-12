#pragma once
#include <poll.h>
#include <vector>
#include <iostream>
#include "../inc/Client.hpp"
#include "../src/configParser/ServerConfig.hpp"
#define TIMEOUT 60

using namespace std;

class ServerInfo
{
private:
	vector<Client> clients;
	struct addrinfo *result;
	vector<pollfd> fds;

public:
	void setResult(struct addrinfo *setRes);
	void setClient(vector<Client> setClient);
	void setFd(vector<pollfd> setFd);

	struct addrinfo *getResult();
	vector<Client> getClients();
	vector<pollfd> &getFd();
};

int server(const vector<ServerConfig> &servers);
int eventLoop(const vector<int> &sockfds, const vector<ServerConfig> &servers);
vector<pollfd> createSockAddr(struct addrinfo *result, const vector<ServerConfig> &servers);