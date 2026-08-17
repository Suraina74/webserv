#pragma once
#include "Location.hpp"

using namespace std;

class ServerConfig
{
	private:
		int		_listen;
		string	_root;
		string	_host;
		string	_index;
		string	_serverName;
		size_t  _maxBodySize;
		map<int, string>  _errPages;
		vector<Location> _locations;
		
	public:
		ServerConfig();
		~ServerConfig();

		void setPort(const int port);
		void setHost(const string &host);
		void setRoot(const string &root);
		void setIndex(const string &index);
		void setBodySize(const size_t &bodySize);
		void setServerName(const string &name);
		void setErrPage(const vector<int>& errCodes, const string& path);
		void addLocation(const Location& loc);

		int						getPort() const;
		const string			&getHost() const;
		const string			&getRoot() const;
		const string			&getIndex() const;
		const string			&getServerName() const;
		const size_t			&getBodySize() const;
		const map<int, string>	&getErrPages() const;
};
