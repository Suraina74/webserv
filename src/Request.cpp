#include "../inc/Request.hpp"

// The parsed HTTP request is evaluated against the configuration after parsing.
// Per belangrijke header kijken wat mag of niet. Content Length, Transfer encoding:chunked, Content Type, Host
// Kijken of alle lines in request eindigen met /r/n. if /r see if next is /n.

string Request::setStatusText(httpStatus status){
	switch (status){
		case OK:
			return "200 OK";
		case BadRequest:
			return "400 Bad Request";
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
}

bool Request::allowedCharsInString(string input){
	string allowedChars = "!#$%&'*+-.^_`|~";
	for (size_t i = 0; i < input.length(); i++){
		if (!(input[i] >= 'a' && input[i] <= 'z') && !(input[i] >= 'A' && input[i] <= 'Z') \
		&& !(input[i] >= '0' && input[i] <= '9') && allowedChars.find(input[i]) == string::npos){
			return false;
		}
	}
	return true;
}

void Request::resetParserVars(){
	fullRequest = {};
	requestLeft = {};
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
	stateOfParser = {};
	partialRequest = {};
}

void Request::setRequest(string request){
	fullRequest = request;
}

void Request::setBytesRead(size_t bytes){
	bytesRead = bytes;
}

void Request::setHeaderBytes(size_t bytes){
	headerBytes = bytes;
}

size_t Request::getContentLength(){
	return contentLength;
}
string Request::getPath(){
	return Path;
}
string Request::getMethod(){
	return Method;
}
httpStatus Request::getStatusCode(){
	return statusCode;
}
string Request::getStatusText(){
	return statusText;
}

string Request::getFullRequest(){
	return fullRequest;
}

string Request::getRequestTillHeaders(){
	return requestTillHeaders;
}

size_t Request::getBytesRead(){
	return bytesRead;
}

size_t Request::getHeaderBytes(){
	return headerBytes;
}

bool Request::getChunked(){
	return chunked;
}
