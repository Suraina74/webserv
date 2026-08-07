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
		std::string contentLengthHeader{};
		ssize_t 	contentLength{};
		ssize_t 	bytesUntilHeaders{};
		std::string	fileName{};
	public:
		Request(std::string input) : Input(input){}
		// Request(const Request& other);
		// Request& operator=(const Request& other);
		~Request(){}

		// Read request into string
		void extractElements();
		ssize_t	getContentLength();
		ssize_t getBytesUntilHeaders();
		// void getFullRequest();
		// Extract body -> extractBody()
};