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
		//useful data for running CGI and eventloop
		string								Path{};
		string								Method{};
		string								requestLine{};
		string								Protocol{};
		unordered_multimap<string, string>	headerMap{};
		string								Body{};
		string								queryString{};//Change by Wenxuan
		
		//used by serverManager.cpp
		httpStatus 							statusCode = OK;
		size_t								contentLength{};
		size_t								headerBytes{};
		bool								chunked = false;
		string								statusText = "200 OK";
		
		string								fullRequest{};
		string								requestTillHeaders{};
		string								partialRequest{};
		size_t								bytesRead{};
		
		string								boundary{};
		map<string, string> 				partHeaderMap{};
		unordered_multimap<string, string>	contentDisposMap{};
		string								fileName{};
		string								fileContent{};

	public:
		Request(){}
		~Request(){}
		bool	parseRequestLine();
		bool	validateRequestLine();
		bool	parseUntilHeaders(string hString);
		bool	parseHeaders();
		bool	validateHeaders();
		bool	actionsOnKey(string& key);
		bool	allowedCharsInKey(string key);
		bool	actionsOnValue(string& value);
		bool	checkPostHeaders();
		bool	checkContentType();
		void	parseBody();
		void	extractBody();
		void	extractChunkedBody();
		void	validateBody();
		void	postAndDelete();
		void	extractFileElements();
		void	addFile();
		string	setStatusText(httpStatus status);
		void	setStatus(httpStatus status);
		void	setRequest(string request);
		void	setBytesRead(size_t bytes);
		void	setHeaderBytes(size_t bytes);
		bool	getChunked() const;
		size_t	getHeaderBytes() const;
		size_t	getBytesRead() const;
		string	getRequestTillHeaders() const;
		string	getFullRequest() const;

		size_t		getContentLength() const;//length of the request body
		string		getPath() const;//path before '?'
		string		getBody() const;//requestion body content
		string		getQuery() const;//Change by Wenxuan
		string		getMethod() const;
		httpStatus  getStatusCode() const;
		string 		getStatusText() const;
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
