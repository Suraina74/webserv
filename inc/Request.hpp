#pragma once 
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

class Request {
	private:
		std::string Input{};
		std::string	requestLine{};
		std::string Method{};
		std::string Path{};
		std::string	statusCode{};
		std::string	statusText{};
		std::string	Body{};
		std::string fileName{};
		std::string fileContent{};
	public:
		Request(std::string input) : Input(input){}
		~Request(){}
		void extractElements();
		void extractBody(std::string input, int bodyLen);
		void extractFileElements(int bodyLen);
		void addFile();
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


// POST /uploads.html HTTP/1.1
// Host: localhost:8080
// Connection: keep-alive
// Content-Length: 207
// \r\n\r\n
// ------WebKitFormBoundarydRcwfbvAQ3EKmZuB
// Content-Disposition: form-data; name="filename"; filename="Hello"
// Content-Type: application/octet-stream

// helloooo


// ------WebKitFormBoundarydRcwfbvAQ3EKmZuB--
