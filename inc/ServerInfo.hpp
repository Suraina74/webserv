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
		vector<pollfd> _pfds;
		
	public:
		ServerInfo();
		~ServerInfo();
		map<int, Client> clients;
		void setPfds(vector<pollfd> setPfds);
		vector<pollfd> &getPfds();
		Client &getClient();
};
