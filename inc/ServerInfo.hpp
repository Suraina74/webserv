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
		
	public:
		vector<Client> clients;
		struct addrinfo *result;
		vector<pollfd> fds;
		void setResult(struct addrinfo *res);
		void setClient(Client client);
		void setFd(pollfd fd);

		struct addrinfo getResult();
		vector<Client> getClients();
		vector<pollfd> getFds();
};

int server(const vector<ServerConfig> &servers);
int eventLoop(const vector<int> &sockfds, const vector<ServerConfig> &servers);
vector<pollfd> createSockAddr(struct addrinfo *result, const vector<ServerConfig> &servers);