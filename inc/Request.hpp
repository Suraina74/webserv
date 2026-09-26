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
#include <cctype>

constexpr std::size_t MAX_REQUEST_LINE = 8192;

enum httpStatus{
	OK = 200,
	BadRequest = 400,
	PageNotFound = 404,
	MethodNotAllowed = 405,
	RequestTimeout = 408,
	ContentTooLarge = 413,
	URITooLong = 414,
	UnsupportedMediaType = 415,
	RequestHeaderFieldsTooLarge = 431,
 	InternalServerError = 500,
	NotImplemented = 501,
	HTTPVersionNotSupported = 505
};

class Request {
	private:
		std::string fullRequest{};
		std::string requestTillHeaders{};
		size_t 	headerBytes{};
		std::string partialRequest{};
		size_t     bytesRead{};
		std::string	requestLine{};
		std::string Method{};
		std::string Protocol{};
		std::string Path{};
		std::map<std::string, std::string> headerMap{};
		size_t		contentLength{};
		bool		chunked = false;
		std::string	boundary{};
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
		bool parseHeaders();
		bool validateHeaders();
		bool checkPostHeaders();
		bool checkContentType();
		void parseBody();
		void extractBody();
		void extractChunkedBody();
		void postAndDelete();
		void extractFileElements();
		void addFile();

		std::string setStatusText(httpStatus status);
		void setRequest(std::string request);
		void setBytesRead(size_t bytes);
		void setHeaderBytes(size_t bytes);

		size_t getContentLength();
		std::string getPath();
		std::string getMethod();
		httpStatus  getStatusCode();
		std::string getStatusText();
		std::string getFullRequest();
		std::string getRequestTillHeaders();
		size_t getHeaderBytes();
		size_t getBytesRead();
		bool getChunked();
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
