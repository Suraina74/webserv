#pragma once 
#include <iostream>

using namespace std;

class Request 
{
	private:
		string	request_line;
		string	*body;
		//hashtable var voor headers
	public:
		Request();
		Request(const Request& other);
		Request& operator=(const Request& other);
		~Request();
};
