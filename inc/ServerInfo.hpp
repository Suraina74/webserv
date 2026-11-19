#pragma once
#include <poll.h>
#include <vector>
#include <map>
#include <iostream>
#include <memory>
#include "CGI.hpp"

using namespace std;

class Client;
class ServerConfig;
class CGI;

class ServerInfo
{
	private:
		//_pfds layout: [ listen fds... | client fds... ]
		//the first _listenCount entries are listen fds, the rest are clients
		vector<pollfd>					_pfds;
		size_t							_listenCount;
		map<int, const ServerConfig*>	_listenServers;	//key = listen fd
		map<int, Client>				_clients;		//key = client fd
		map<int, CGI>					_cgis;
		map<int, int>					_cgiFdOwners;

	public:
		ServerInfo();
		~ServerInfo();

		void					addListenFd(int fd, const ServerConfig *server);
		void					addClient(int fd, const ServerConfig *server, string& clientIp);
		void					removeClient(size_t index);
		void					closeAllFds();
		size_t					getListenCount() const;
		vector<pollfd>&			getPfds();
		Client&					getClient(int fd);
		const ServerConfig*		getListenServer(int fd);
		
		void					populateCgi(int clientFd, CGI& Cgi);
		CGI&					getClientCgi(int clientFd);
		void					registerCgiPipes(int clientFd);
		void					unregisterCgiPipe(size_t i);
		int						findCgiOwner(int fd);
};
