#pragma once
#include "Request.hpp"
#include <sstream>

class Response
{
	private:
		Request 	request{};
		std::string	statusLine = "HTTP/1.1";
		std::string contentType = "Content-Type: text/html\r\n";
		std::string contentLength = "Content-Length: ";
		std::string body{};
		std::string fullResponse{};
		int         lenResponse{};
		const char* cFullResponse{};
		int         bytesSent{};
	public:
		Response(){}
		~Response(){}
		int composeResponse();
		void cleanResponse();

		void setRequest(Request r);
		void setLenResponse(int length);
		void setCString(const char* cString);
		void setBytesSent(int bytes);

		std::string getFullResponse();
		const char* getCFullResponse();
		int getLenResponse();
		int getBytesSent();
};


// HTTP/1.1 200 OK\r\n
// Content-Type: text/html\r\n
// Content-Length: 13\r\n
// \r\n
// text in html page


// String samenstellen voor response zonder body t/m \r\n\r\n
// en dan die string + html page