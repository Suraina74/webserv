#pragma once 
#include <iostream>
#include <string>

class Request {
	private:
		std::string Input;
		std::string	requestLine;
		std::string Method;
		std::string Path;
		// std::string	*body;
		//hashtable var voor headers
	public:
		Request(std::string input) : Input(input){}
		// Request(const Request& other);
		// Request& operator=(const Request& other);
		~Request(){}

		// Read request into string
		void extractReqLine();
		// Extract headers -> extractHeaders()
		// Extract body -> extractBody()
};