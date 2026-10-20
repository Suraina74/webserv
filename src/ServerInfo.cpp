#include "../inc/ServerInfo.hpp"
#include <unistd.h>

ServerInfo::ServerInfo():_listenCount(0){}

ServerInfo::~ServerInfo(){}

//listen fds must all be added before any client, so they stay at the front of _pfds
void ServerInfo::addListenFd(int fd, const ServerConfig *server)
{
	pollfd p;
	p.fd = fd;
	p.events = POLLIN;
	p.revents = 0;
	_pfds.push_back(p);
	_listenServers[fd] = server;
	_listenCount++;
}

void ServerInfo::addClient(int fd, const ServerConfig *server)
{
	pollfd p;
	p.fd = fd;
	p.events = POLLIN;
	p.revents = 0;
	_pfds.push_back(p);
	Client newClient = Client(fd, server, Request(), Response());
	_clients[p.fd] = newClient;
}

//closes the client fd and removes it from both _pfds and _clients
void ServerInfo::removeClient(size_t index)
{
	int fd = _pfds[index].fd;
	close(fd);
	_clients.erase(fd);
	_pfds.erase(_pfds.begin() + index);
}

void ServerInfo::closeAllFds()
{
	for (size_t i = 0; i < _pfds.size(); ++i)
		close(_pfds[i].fd);
	_pfds.clear();
	_clients.clear();
	_listenServers.clear();
	_listenCount = 0;
}

size_t ServerInfo::getListenCount() const
{
	return (_listenCount);
}

vector<pollfd>& ServerInfo::getPfds()
{
	return (_pfds);
}

Client& ServerInfo::getClient(int fd)
{
	return (_clients.at(fd));
}

const ServerConfig* ServerInfo::getListenServer(int fd)
{
	return (_listenServers.at(fd));
}
