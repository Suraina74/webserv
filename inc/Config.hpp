#pragma once
#include "Server.hpp"
#include "Location.hpp"
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
		void parseErr(string& val, string& extra, ServerConfig& server, int lineNum);
		void parseServerName(string& val, ServerConfig& server, int lineNum);
};

void cleanLine(string& line);
void verifyNum(string sub, int lineNum);
void emptyValCheck(string& val, int lineNum);