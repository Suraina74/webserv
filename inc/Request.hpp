#pragma once
#include "ServerConfig.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <unordered_map>
#include <cstddef>
#include <cctype>

constexpr size_t MAX_REQUEST_LINE = 8192;
// Max Body size
using namespace std;

enum httpStatus
{
	OK = 200,
	NoContent = 204,
	BadRequest = 400,
	PageNotFound = 404,
	MethodNotAllowed = 405,
	RequestTimeout = 408,
	ContentTooLarge = 413,
	Forbidden = 403,
	URITooLong = 414,
	UnsupportedMediaType = 415,
	RequestHeaderFieldsTooLarge = 431,
	InternalServerError = 500,
	NotImplemented = 501,
	HTTPVersionNotSupported = 505
};

enum parserState
{
	PARSER_ERROR = -1,
	ZERO_BYTES = 0,
	INCOMPLETE_HEADERS = 1,
	BODY_INCOMPLETE = 2,
	REQUEST_READY = 3,
	RESPONSE_READY = 4
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
		string								queryString{};//Change by Wenxuan
		unordered_multimap<string, string>	headerMap{};
		unordered_multimap<string, string>	contentTypeMap{};
		size_t								contentLength{};
		bool								chunked = false;
		string								boundary{};
		string								statusText = "200 OK";
		string								Body{};
		string								requestLeft{};
		unordered_multimap<string, string>	bodyHeaderMap{};
		unordered_multimap<string, string>	contentDisposMap{};
		string								fileName{};
		string								fileContent{};
		httpStatus							statusCode = OK;
		parserState							stateOfParser{};

	public:
		Request(){}
		~Request(){}
		bool		parseRequestLine();
		bool		validateRequestLine();
		bool		parseUntilHeaders(string hString);
		bool		parseHeaders();
		bool		validateHeaders();
		bool		makeMapOfHeader(unordered_multimap<string, string>& map, string header, string value);
		bool		actionsOnKey(string& key);
		bool		allowedCharsInString(string input);
		bool		actionsOnValue(string& value);
		bool		checkOtherHeaders();
		bool		checkContentType();
		void		parseBody();
		void		extractBody();
		void		extractChunkedBody();
		void		validateBody();
		bool		checkContentDisposition(string valueCT);
		void		postAndDelete();
		void		extractFileElements();
		void		addFile();
		void		resetParserVars();

		string		setStatusText(httpStatus status);
		void		setStatus(httpStatus status);
		void		setRequest(string request);
		void		setBytesRead(size_t bytes);
		void		setHeaderBytes(size_t bytes);
		void		setParserState(parserState state);
		void		setRequestLeft(string request);

		bool		getChunked() const;
		parserState	getParserState() const;
		size_t		getHeaderBytes() const;
		size_t		getBytesRead() const;
		string		getRequestTillHeaders() const;
		string		getFullRequest() const;
		size_t		getContentLength() const;//length of the request body
		string 		getRequestLeft() const;
		string		getPath() const;//path before '?'
		string		getBody() const;//request body content
		string		getQuery() const;//Change by Wenxuan
		string		getMethod() const;
		httpStatus	getStatusCode() const;
		string		getStatusText() const;
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

// hello

// ------WebKitFormBoundarydRcwfbvAQ3EKmZuB--


// Transfer encoding chunked example:
// printf 'POST /uploads.html HTTP/1.1\r\nHost: localhost:8080\r\nContent-Type: multipart/form-data; boundary=MyBoundary123\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n8A\r\n--MyBoundary123\r\nContent-Disposition: form-data; name="file"; filename="hello.txt"\r\nContent-Type: text/plain\r\n\r\nHello\r\n--MyBoundary123--\r\n0\r\n\r\n' | nc 127.0.0.1 8080

// Request with content length:
// printf 'POST /uploads.html HTTP/1.1\r\nHost: localhost:8080\r\nContent-Type: multipart/form-data; boundary=MyBoundary123\r\nContent-Length: 138\r\nConnection: close\r\n\r\n--MyBoundary123\r\nContent-Disposition: form-data; name="file"; filename="hello.txt"\r\nContent-Type: text/plain\r\n\r\nHello\r\n--MyBoundary123--\r\n' | nc 127.0.0.1 8080