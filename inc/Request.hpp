#pragma once 
#include "Server.hpp"

class Request 
{
	private:
		std::string	request_line;
		std::string	*body;
		//hashtable var voor headers
	public:
		Request();
		Request(const Request& other);
		Request& operator=(const Request& other);
		~Request();
};
