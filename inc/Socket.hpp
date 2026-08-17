#pragma once 
#include "Server.hpp"

using namespace std;

class Socket 
{
	private:
		string	status_line;
		string	*body;
		//hashtable var voor headers
	public:
		Socket();
		Socket(const Socket& other);
		Socket operator=(const Socket& other);
		~Socket();
};
