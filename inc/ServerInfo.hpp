#pragma once
#include <poll.h>
#include <vector>
#include <map>
#include <iostream>
#include "ServerConfig.hpp"
#include "Client.hpp"
#define TIMEOUT 60

using namespace std;

class ServerInfo
{
	private:
		//_pfds layout: [ listen fds... | client fds... ]
		//the first _listenCount entries are listen fds, the rest are clients
		vector<pollfd>					_pfds;
		size_t							_listenCount;
		map<int, const ServerConfig*>	_listenServers;	//key = listen fd
		map<int, Client>				_clients;		//key = client fd

	public:
		ServerInfo();
		~ServerInfo();

		void					addListenFd(int fd, const ServerConfig *server);
		void					addClient(int fd, const ServerConfig *server);
		void					removeClient(size_t index);
		void					closeAllFds();
		size_t					getListenCount() const;
		vector<pollfd>&			getPfds();
		Client&					getClient(int fd);
		const ServerConfig*		getListenServer(int fd);
};
