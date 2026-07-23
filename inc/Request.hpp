#pragma once 
#include <ostream>

class Request {
	private:
		std::string request;
		std::string	request_line;
		std::string	*body;
		//hashtable var voor headers
	public:
		Request(std::string input);
		// Request(const Request& other);
		// Request& operator=(const Request& other);
		~Request();

		// Read request into string
		// Extract request line -> extractReqLine();
		// Extract headers -> extractHeaders()
		// Extract body -> extractBody()
};