#pragma once
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <poll.h>
#include <set>
#include <limits>
#include <sys/stat.h>
#include "ServerConfig.hpp"

using namespace std;

class Config 
{
	private:
		vector<ServerConfig> _servers;

	public:
		Config();
		~Config();

		const vector<ServerConfig>& getServers() const;
		void parse(const string& filename);
		void parseServer(ifstream& configFile, int& lineNum);
		void parseLine(string& line, ServerConfig& server, int lineNum);
		void parseHost(string& val, ServerConfig& server, int lineNum);
		void parseRoot(string& val, ServerConfig& server, int lineNum);
		void parseIndex(string& val, ServerConfig& server, int lineNum);
		void parsePort(string& val, ServerConfig& server, int lineNum);
		void parseBodySize(string& val, ServerConfig& server, int lineNum);
		void parseErr(string& val, ServerConfig& server, int lineNum);
		void parseServerName(string& val, ServerConfig& server, int lineNum);
};

void cleanLine(string& line);
void verifyNum(string sub, int lineNum);
void emptyValCheck(string& val, int lineNum);
void verifyLabel(string& val, int lineNum);
void verifyErrPath(string& path, int lineNum);