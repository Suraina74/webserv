#pragma once 
#include <iostream>
#include <string>
#include <sstream>

class Request {
	private:
		std::string Input{};
		std::string	requestLine{};
		std::string Method{};
		std::string Path{};
		int			statusCode{};
		std::string	Content{};
		// std::string contentLengthHeader{};
		// ssize_t 	contentLength{};
	public:
		Request(std::string input) : Input(input){}
		// Request(const Request& other);
		// Request& operator=(const Request& other);
		~Request(){}
		void extractElements();
		// Extract body -> extractBody()
};

ssize_t 	getContentlength(std::string message);
ssize_t 	getBytesUntilHeaders(std::string message);