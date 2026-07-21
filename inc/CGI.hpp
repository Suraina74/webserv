#pragma once 
#include "Server.hpp"

class CGI 
{
	private:
		//serv socket
	public:
		CGI();
		CGI(const CGI& other);
		CGI& operator=(const CGI& other);
		~CGI();
};