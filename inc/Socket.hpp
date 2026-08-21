#pragma once 
#include <iostream>

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
