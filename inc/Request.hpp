#pragma once 
#include "ServerConfig.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <cstddef>
#include <cctype>
#include <unordered_map>

constexpr size_t MAX_REQUEST_LINE = 8192;
// Max Body size
using namespace std;
enum httpStatus
{
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

class Request
{
	private:
		string								fullRequest{};
		string								requestTillHeaders{};
		size_t								headerBytes{};
		string								partialRequest{};
		size_t								bytesRead{};
		string								requestLine{};
		string								Method{};
		string								Protocol{};
		string								Path{};
		unordered_multimap<string, string>	headerMap{};
		size_t								contentLength{};
		bool								chunked = false;
		string								boundary{};
		string								statusText = "200 OK";
		string								Body{};
		map<string, string> 				partHeaderMap{};
		unordered_multimap<string, string>	contentDisposMap{};
		string								fileName{};
		string								fileContent{};
		httpStatus 							statusCode = OK;

	public:
		Request(){}
		~Request(){}
		bool parseRequestLine();
		bool validateRequestLine();
		bool parseUntilHeaders(string hString);
		bool parseHeaders();
		bool validateHeaders();
		bool actionsOnKey(string& key);
		bool allowedCharsInKey(string key);
		bool actionsOnValue(string& value);
		bool checkPostHeaders();
		bool checkContentType();
		void parseBody();
		void extractBody();
		void extractChunkedBody();
		void validateBody();
		void postAndDelete();
		void extractFileElements();
		void addFile();

		string	setStatusText(httpStatus status);
		void	setRequest(string request);
		void	setBytesRead(size_t bytes);
		void	setHeaderBytes(size_t bytes);

		size_t getContentLength();
		string getPath();
		string getMethod();
		httpStatus  getStatusCode();
		string getStatusText();
		string getFullRequest();
		string getRequestTillHeaders();
		size_t getHeaderBytes();
		size_t getBytesRead();
		bool getChunked();
		
		string getBody();
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
