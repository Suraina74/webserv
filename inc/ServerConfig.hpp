#pragma once
#include "Server.hpp"

using std::cout;
using std::cerr;
using std::cin;

using std::endl;
using std::string;

using std::vector;
using std::map;
using std::set;

using std::ifstream;
using std::ofstream;
using std::stringstream;

class ServerConfig 
{
	private:
		int		_listen;
		string	_host;
		string	_serverName;
		string	_root;
		string	_errPage;
		int		_index;
		
	public:
		ServerConfig();
		~ServerConfig();
};