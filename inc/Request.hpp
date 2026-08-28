#pragma once 
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <map>

enum httpStatus{
	OK = 200,
	BadRequest = 400,
	PageNotFound = 404,
	MethodNotAllowed = 405,
	RequestTimeout = 408,
	ContentTooLarge = 413,
	URITooLong = 414,
	RequestHeaderFieldsTooLarge = 431,
 	InternalServerError = 500,
	HTTPVersionNotSupported = 505
};

class Request {
	private:
		std::string fullRequest{};
		std::string	requestLine{};
		std::string Method{};
		std::string Protocol{};
		std::string Path{};
		std::string	statusText{};
		std::string	Body{};
		std::string fileName{};
		std::string fileContent{};
		httpStatus  statusCode;
		std::map<std::string, std::string> headers;

	public:
		Request(std::string fRequest) : fullRequest(fRequest){}
		~Request(){}
		void extractMethodPathProtocol();
		void extractHeaders();
		void parse();
		void extractBody(std::string input, int bodyLen);
		void extractFileElements();
		void addFile();
		std::string setStatusText(httpStatus status);
		// std::string getMethod();
		std::string getPath();
		std::string getMethod();
		httpStatus  getStatusCode();
		std::string getStatusText();
		// Extract body -> extractBody()
};

ssize_t 	getContentlength(std::string message);
ssize_t 	getBytesUntilHeaders(std::string message);


// Request ontvangen, headers apart opslaan, body opslaan,
// Extract information, 
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
