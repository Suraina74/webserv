#pragma once 
#include "../src/configParser/ServerConfig.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>

class Request {
	private:
		std::string Input{};
		std::string	requestLine{};
		std::string Method{};
		std::string Path{};
		std::string	statusCode{};
		std::string	statusText{};
		std::string	Content{};
	public:
		Request(std::string input) : Input(input){}
		~Request(){}
		void extractElements(const ServerConfig &server);
		// std::string getMethod();
		std::string getPath();
		std::string getStatusCode();
		std::string getStatusText();
		// Extract body -> extractBody()
};

ssize_t 	getContentlength(std::string message);
ssize_t 	getBytesUntilHeaders(std::string message);


// Request ontvangen, Extract information, 
// Wat nodig voor response: Method, Path, statusCode, contentLength, Body
// Validating http request.