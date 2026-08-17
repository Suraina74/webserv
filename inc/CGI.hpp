#pragma once 
#include "Server.hpp"

using namespace std;

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