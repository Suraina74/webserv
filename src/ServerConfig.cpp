#include "../inc/ServerConfig.hpp"

ServerConfig::ServerConfig():_listen(0), _host(""), _serverName(""), _root(""), _errPage(""), _index("") {}

ServerConfig::~ServerConfig(){}

void ServerConfig::setPort(const int port)
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

void ServerConfig::setBodySize(const size_t &bodySize)
{
	_maxBodySize = bodySize;
}

void ServerConfig::setServerName(const string &name)
{
	_serverName = name;
}

const int	ServerConfig::getPort() const
{
	return _listen;
}

const string& ServerConfig::getHost() const
{
	return _host;
}

const string& ServerConfig::getRoot() const
{
	return _root;
}

const string& ServerConfig::getIndex() const
{
	return _index;
}

const string& ServerConfig::getServerName() const
{
	return _serverName;
}

const size_t& ServerConfig::getBodySize() const
{
	return _maxBodySize;
}