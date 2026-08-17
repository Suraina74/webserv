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

		//main parser
		void parse(const string& filename);
		//server block parser
		void parseServer(ifstream& configFile, int& lineNum, bool& foundServer);
		//server directive parser
		void parseLine(string& line, ServerConfig& server, int lineNum);
		void parseHost(string& val, ServerConfig& server, int lineNum);
		void parseRoot(string& val, ServerConfig& server, int lineNum);
		void parseIndex(string& val, ServerConfig& server, int lineNum);
		void parsePort(string& val, ServerConfig& server, int lineNum);
		void parseBodySize(string& val, ServerConfig& server, int lineNum);
		void parseErr(string& val, ServerConfig& server, int lineNum);
		void parseServerName(string& val, ServerConfig& server, int lineNum);
		//location block parser
		void parseLocation(ifstream& configFile, string& line, ServerConfig& server, int lineNum);
		//location directive parser
		void parseLocLine(string& line, Location& loc, int lineNum);
		void parseMethods(string& val, Location& loc, int lineNum);
		void parseCGI(string& val, Location& loc, int lineNum);
		void parseLocPath(string& val, Location& loc, int lineNum);
    	void parseLocRoot(string& val, Location& loc, int lineNum);
    	void parseLocIndex(string& val, Location& loc, int lineNum);
    	void parseUploadPath(string& val, Location& loc, int lineNum);
    	void parseUpload(string& val, Location& loc, int lineNum);
    	void parseRedirect(string& val, Location& loc, int lineNum);
    	void parseDirectoryListing(string& val, Location& loc, int lineNum);
		//accessor to server
		const vector<ServerConfig>& getServers() const;
};

void cleanLine(string& line);
void verifyNum(string sub, int lineNum);
void emptyValCheck(string& val, int lineNum);
void verifyLabel(string& val, int lineNum);
void verifyErrPath(string& path, int lineNum);