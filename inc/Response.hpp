#pragma once
#include "Request.hpp"
#include <sstream>

using namespace std;

class Response
{
	private:
		Request		request{};
		string		statusLine = "HTTP/1.1";
		string		contentType = "Content-Type: text/html\r\n";
		string		contentLength = "Content-Length: ";
		string		body{};
		string		fullResponse{};
		int         lenResponse{};
		const char* cFullResponse{};
		int         bytesSent{};

	public:
		Response(){}
		~Response(){}
		int composeResponse();

		void setRequest(Request r);
		void setLenResponse(int length);
		void setCString(const char* cString);
		void setBytesSent(int bytes);

		string getFullResponse();
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