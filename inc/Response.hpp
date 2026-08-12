#pragma once 
#include "Server.hpp"

class Response 
{
	private:
		std::string	statusLine;
		int			statusCode;
		std::string statusText;
		std::string contentType;
		std::string contentLength;
		std::string	body; // The html page
	public:
		Response();
		// Response(const Response& other);
		// Response operator=(const Response& other);
		~Response();
};


// HTTP/1.1 200 OK\r\n
// Content-Type: text/html\r\n
// Content-Length: 13\r\n
// \r\n
// text in html page


// String samenstellen voor response zonder body t/m \r\n\r\n
// en dan die string + html page