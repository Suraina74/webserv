#pragma once 
#include "../src/configParser/ServerConfig.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <cstddef>

constexpr size_t MAX_REQUEST_LINE = 8192;
using namespace std;

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
	NotImplemented = 501,
	HTTPVersionNotSupported = 505
};

class Request
{
	private:
		string fullRequest{};
		string requestTillHeaders{};
		ssize_t 	headerBytes{};
		string partialRequest{};
		ssize_t     bytesRead{};
		string	requestLine{};
		map<string, string> headerMap{};
		ssize_t		contentLength{};
		string Method{};
		string Protocol{};
		string Path{};
		string	statusText = "200 OK";
		string	Body{};
		string fileName{};
		string fileContent{};
		httpStatus  statusCode = OK;

	public:
		Request(){}
		~Request(){}
		bool parseRequestLine();
		bool validateRequestLine();
		bool parseUntilHeaders(string hString);
		bool parseHeaders();
		bool validateHeaders();
		void parseBody();
		void extractBody();
		void extractFileElements();
		void addFile();
		void cleanRequest();

		string setStatusText(httpStatus status);
		void setRequest(string request);
		void setBytesRead(ssize_t bytes);
		void setHeaderBytes(ssize_t bytes);

		ssize_t getContentLength();
		string getPath();
		string getMethod();
		httpStatus  getStatusCode();
		string getStatusText();
		string getFullRequest();
		string getRequestTillHeaders();
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
