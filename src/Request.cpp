#include "../inc/Request.hpp"

// The parsed HTTP request is evaluated against the configuration after parsing.
// Kijken of alle lines in request eindigen met /r/n. if /r see if next is /n.

string Request::setStatusText(httpStatus status)
{
	switch (status)
	{
		case OK:
			return "200 OK";
		case NoContent:
			return "204 No Content";
		case BadRequest:
			return "400 Bad Request";
		case Forbidden:
			return "403 Forbidden";
		case PageNotFound:
			return "404 Page Not Found";
		case MethodNotAllowed:
			return "405 Method Not Allowed";
		case RequestTimeout:
			return "408 Request Timeout";
		case ContentTooLarge:
			return "413 Content Too Large";
		case URITooLong:
			return "414 URI Too Long";
		case UnsupportedMediaType:
			return "415 Unsupported Media Type";
		case RequestHeaderFieldsTooLarge:
			return "431 Request Header Fields Too Large";
		case InternalServerError:
			return "500 Internal Server Error";
		case NotImplemented:
			return "501 Not Implemented";
		case HTTPVersionNotSupported:
			return "505 HTTP Version Not Supported";
	}
	return "500 Internal Server Error";
}

bool Request::allowedCharsInString(string input)
{
	string allowedChars = "!#$%&'*+-.^_`|~";
	for (size_t i = 0; i < input.length(); i++)
	{
		if (!(input[i] >= 'a' && input[i] <= 'z') && !(input[i] >= 'A' && input[i] <= 'Z')
		&& !(input[i] >= '0' && input[i] <= '9') && allowedChars.find(input[i]) == string::npos)
			return false;
	}
	return true;
}

void Request::resetParserVars()
{
	fullRequest = {};
	requestTillHeaders = {};
	headerBytes = {};
	bytesRead = {};
	requestLine = {};
	Method = {};
	Protocol = {};
	Path = {};
	queryString = {};//Change by Wenxuan
	headerMap = {};
	contentTypeMap = {};
	contentLength = {};
	chunked = false;
	boundary = {};
	statusText = "200 OK";
	Body = {};
	bodyHeaderMap = {};
	contentDisposMap = {};
	fileName = {};
	fileContent = {};
	statusCode = OK;
	partialRequest = {};
}

void Request::setRequest(string request)
{
	fullRequest = request;
}

void Request::setBytesRead(size_t bytes)
{
	bytesRead = bytes;
}

void Request::setHeaderBytes(size_t bytes)
{
	headerBytes = bytes;
}

void Request::setStatus(httpStatus status)
{
	statusCode = status;
	statusText = setStatusText(status);
	if (status == PageNotFound)
		Path = "www/404.html";
}

void Request::setParserState(parserState state){
	stateOfParser = state;
}

void Request::setRequestLeft(string request){
	requestLeft = request;
}

size_t Request::getContentLength() const
{
	return contentLength;
}

string Request::getPath() const
{
	return Path;
}

string Request::getMethod() const
{
	return Method;
}

httpStatus Request::getStatusCode() const
{
	return statusCode;
}

string Request::getStatusText() const
{
	return statusText;
}

string Request::getFullRequest() const
{
	return fullRequest;
}

string Request::getRequestTillHeaders() const
{
	return requestTillHeaders;
}

size_t Request::getBytesRead() const
{
	return bytesRead;
}

size_t Request::getHeaderBytes() const
{
	return headerBytes;
}

bool Request::getChunked() const
{
	return chunked;
}

string Request::getRequestLeft() const{
	return requestLeft;
}

parserState Request::getParserState() const{
	return stateOfParser;
}

string Request::getBody() const
{
	return Body;
}

//Change by Wenxuan
string Request::getQuery() const
{
	return queryString;
}
