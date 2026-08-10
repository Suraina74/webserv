#include "../inc/ServerConfig.hpp"

ServerConfig::ServerConfig():_listen(0), _host(""), _serverName(""), _root(""), _errPage(""), _index("") {}

ServerConfig::~ServerConfig(){}

void ServerConfig::setPort(int port)
{
	_listen = port;
}

void ServerConfig::setHost(const string &host)
{
	_host = host;
}

void ServerConfig::setRoot(const string &root)
{
	_root = root;
}

void ServerConfig::setIndex(const string &index)
{
	_index = index;
}