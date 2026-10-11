#pragma once 
#include <iostream>
#include <unistd.h>
#include <vector>
#include <map>
#include "Request.hpp"

using namespace std;

class CGI 
{
	private:
		map<string, string>	_env; //key = "SERVER_PORT", value = "8080"
		string				_interpreter; //tells server what language to use
		string				_scriptPath; 
		string				_requestBody;
		size_t				_bodySent;
		string				_output;
		int					_clientFd;

		int					_inFd;
		int					_outFd;
		pid_t				_pid;

	public:
		CGI();
		~CGI();
		CGI(Request &request, const ServerConfig &server, int clientFd);
		void	populateEnv(Request &request, const ServerConfig &server);
		int		runScript();

		int		getInFd();
		int		getOutFd();
		int		getClientFd();
		pid_t	getPid();
		string&	getOutput();
		int		writeInput();
		int		readOutput();

		void	closeIn();	//close(_inFd); _inFd = -1;
		void	closeOut();	//close(_outFd); _outFd = -1;
};
