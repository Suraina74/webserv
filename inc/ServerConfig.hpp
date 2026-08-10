#pragma once
#include "Server.hpp"
#include "Config.hpp"
#include "Location.hpp"

class ServerConfig 
{
	private:
		int		_listen;
		string	_root;
		string	_host;
		string	_index;
		string	_serverName;
		string	_errPage;
		size_t  _clientMaxBodySize;
		bool	_foundServer = false;
		//map<int, string>  _errorPages;
		vector<Location> _locations;
		
	public:
		ServerConfig();
		~ServerConfig();

		void setPort(int port);
		void setHost(const string &host);
		void setRoot(const string &root);
		void setIndex(const string &index);

		int						getPort() const;
		const string			&getHost() const;
		const string			&getRoot() const;
		const string			&getIndex() const;
};
