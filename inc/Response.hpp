#pragma once 
#include "Server.hpp"

using namespace std;

class Response 
{
	private:
		string	status_line;
		string	*body;
		//hashtable var voor headers
	public:
		Response();
		Response(const Response& other);
		Response operator=(const Response& other);
		~Response();
};
