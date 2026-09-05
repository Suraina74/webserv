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
		std::string requestTillHeaders{};
		ssize_t 	headerBytes{};
		std::string partialRequest{};
		ssize_t     bytesRead{};
		std::string	requestLine{};
		std::map<std::string, std::string> headerMap{};
		ssize_t		contentLength{};
		std::string Method{};
		std::string Protocol{};
		std::string Path{};
		std::string	statusText = "200 OK";
		std::string	Body{};
		std::string fileName{};
		std::string fileContent{};
		httpStatus  statusCode = OK;

	public:
		Request(){}
		~Request(){}
		bool parseRequestLine();
		bool validateRequestLine();
		bool parseUntilHeaders(std::string hString);
		void parseHeaders();
		bool validateHeaders();
		void parseBody();
		void extractBody();
		void extractFileElements();
		void addFile();
		void cleanRequest();

		std::string setStatusText(httpStatus status);
		void setRequest(std::string request);
		void setBytesRead(ssize_t bytes);
		void setHeaderBytes(ssize_t bytes);

		ssize_t getContentLength();
		std::string getPath();
		std::string getMethod();
		httpStatus  getStatusCode();
		std::string getStatusText();
		std::string getFullRequest();
		std::string getRequestTillHeaders();
		ssize_t getHeaderBytes();
		ssize_t getBytesRead();
};


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
