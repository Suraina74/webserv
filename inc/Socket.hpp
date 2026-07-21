#pragma once 
#include "Server.hpp"

class Socket 
{
	private:
		std::string	status_line;
		std::string	*body;
		//hashtable var voor headers
	public:
		Socket();
		Socket(const Socket& other);
		Socket operator=(const Socket& other);
		~Socket();
};
