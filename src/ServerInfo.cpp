#include "../inc/ServerInfo.hpp"

ServerInfo::ServerInfo(){}

ServerInfo::~ServerInfo(){}

void ServerInfo::setFd(vector<pollfd> setFds)
{
	_fds = setFds;
}

vector<pollfd> &ServerInfo::getFds()
{
	return (_fds);
}
