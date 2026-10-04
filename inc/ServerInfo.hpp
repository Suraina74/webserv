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
		ServerInfo();
		~ServerInfo();
		map<int, Client> clients;
		vector<pollfd> fds;
		void setResult(struct addrinfo *res);
		void setClient(Client client);
		void setFd(pollfd fd);
		struct addrinfo *getResult();
		vector<pollfd> &getFd();
		Client &getClient();
};

