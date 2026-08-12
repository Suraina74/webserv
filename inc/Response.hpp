#pragma once 
#include "Server.hpp"
#include "Request.hpp"

class Response 
{
	private:
		Request 	httpRequest;
		std::string	statusLine = "HTTP/1.1";
		std::string statusText;
		std::string contentType;
		std::string contentLength;
	public:
		Response(Request request) : httpRequest(request){}
		// Response(const Response& other);
		// Response operator=(const Response& other);
		void composeResponse();
		~Response(){}
};


// HTTP/1.1 200 OK\r\n
// Content-Type: text/html\r\n
// Content-Length: 13\r\n
// \r\n
// text in html page


// String samenstellen voor response zonder body t/m \r\n\r\n
// en dan die string + html page