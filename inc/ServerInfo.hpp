#pragma once
#include <poll.h>
#include <vector>
#include <iostream>
#include "ServerConfig.hpp"
#include "Client.hpp"
#define TIMEOUT 60

using namespace std;

class ServerInfo
{
	private:
		vector<pollfd> _fds;
		
	public:
		ServerInfo();
		~ServerInfo();
		map<int, Client> clients;
		void setFds(vector<pollfd> setFds);
		vector<pollfd> &getFds();
		Client &getClient();
};
