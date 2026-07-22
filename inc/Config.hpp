#pragma once
#include "Server.hpp"

class Config 
{
	private:
		vector<ServerConfig> _servers;
	public:
		Config();
		~Config();
};