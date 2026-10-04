#include "../inc/ServerInfo.hpp"

ServerInfo::ServerInfo(){}

ServerInfo::~ServerInfo(){}

void ServerInfo::setPfds(vector<pollfd> setPfds)
{
	_pfds = setPfds;
}

vector<pollfd> &ServerInfo::getPfds()
{
	return (_pfds);
}
