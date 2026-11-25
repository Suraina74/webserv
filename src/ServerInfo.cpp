#include "../inc/ServerInfo.hpp"
#include "../inc/Client.hpp"
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

void ServerInfo::addClient(int fd, const ServerConfig *server, string& clientIp)
{
	pollfd p;
	p.fd = fd;
	p.events = POLLIN;
	p.revents = 0;
	_pfds.push_back(p);
	Client newClient = Client(fd, server, Request(), Response(), clientIp);
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

void ServerInfo::populateCgi(int clientFd, CGI& Cgi)
{
	_cgis[clientFd] = Cgi;
}

CGI& ServerInfo::getClientCgi(int clientFd)
{
	return _cgis.at(clientFd);
}

int ServerInfo::findCgiOwner(int fd)
{
	auto it = _cgiFdOwners.find(fd);
	if (it == _cgiFdOwners.end())
		return -1;
	return it->second;
}

void ServerInfo::registerCgiPipes(int clientFd)
{
	CGI& curCgi = getClientCgi(clientFd);
	int inFd = curCgi.getInFd();
	int outFd = curCgi.getOutFd();

	if (inFd != -1)
	{
		_pfds.push_back(pollfd{inFd, POLLOUT, 0});
		_cgiFdOwners[inFd] = clientFd;
	}
	_pfds.push_back(pollfd{outFd, POLLIN, 0});
	_cgiFdOwners[outFd] = clientFd;
}

void ServerInfo::unregisterCgiPipe(size_t i)
{
    int fd = _pfds[i].fd;
    _cgiFdOwners.erase(fd);
    _pfds.erase(_pfds.begin() + i);
}