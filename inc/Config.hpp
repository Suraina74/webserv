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
using std::runtime_error;

using std::ifstream;
using std::ofstream;
using std::istringstream;

class Config 
{
	private:
		vector<ServerConfig> _servers;

	public:
		Config();
		~Config();

		void cleanLine(string& line);
		void parseLine(string& line, ServerConfig& server, int lineNum);
		void parse(const std::string& filename);
		const std::vector<ServerConfig>& getServers() const;
		void parseHost(string& line, ServerConfig& server, int lineNum);
		void parseRoot(string& line, ServerConfig& server, int lineNum);
		void parseIndex(string& line, ServerConfig& server, int lineNum);
		void parsePort(string& line, ServerConfig& server, int lineNum);
		void parseServer(ifstream& configFile, int& lineNum);
};

class ServerConfig 
{
	private:
		int		_listen;
		string	_host;
		string	_serverName;
		string	_root;
		string	_errPage;
		string	_index;
		bool	_foundServer = false;
		
	public:
		ServerConfig();
		~ServerConfig();

		void setPort(int port);
		void setHost(const std::string &host);
		void setRoot(const std::string &root);
		void setIndex(const std::string &index);

		int							getPort() const;
		const std::string			&getHost() const;
		const std::string			&getRoot() const;
		const std::string			&getIndex() const;
};
