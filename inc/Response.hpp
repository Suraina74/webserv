#pragma once
#include "Request.hpp"
#include <sstream>

class Response
{
	private:
		Request 	request;
		std::string	statusLine = "HTTP/1.1";
		std::string contentType = "Content-Type: text/html\r\n";
		std::string contentLength = "Content-Length: ";
		std::string body{};
		std::string fullResponse{};
	public:
		Response(Request httpRequest) : request(httpRequest){}
		int composeResponse();
		std::string getFullResponse();
		~Response(){}
};


// HTTP/1.1 200 OK\r\n
// Content-Type: text/html\r\n
// Content-Length: 13\r\n
// \r\n
// text in html page


// String samenstellen voor response zonder body t/m \r\n\r\n
// en dan die string + html page