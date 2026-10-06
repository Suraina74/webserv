#include "../inc/Request.hpp"

// The parsed HTTP request is evaluated against the configuration after parsing.
// Per belangrijke header kijken wat mag of niet. Content Length, Transfer encoding:chunked, Content Type, Host
// Kijken of alle lines in request eindigen met /r/n. if /r see if next is /n.

// Content type and filename in content disp. are optional.
// Kijken of filename niet leeg is..
// Ignore unknown headers in body.

void Request::extractFileElements(){
	auto itFilename = contentDisposMap.find("filename");
	if (itFilename == contentDisposMap.end()){
		statusCode = BadRequest;
		return;
	}
	fileName = itFilename->second;
	size_t startOfFileContent = Body.find("\r\n\r\n");
	if (startOfFileContent == string::npos){
		statusCode = BadRequest;
		return;
	}
	startOfFileContent += 4;
	size_t endOfFileContent = Body.find("\r\n" + boundary + "--");
	if (endOfFileContent == string::npos){
		statusCode = BadRequest;
		return;
	}
	fileContent = Body.substr(startOfFileContent, endOfFileContent - startOfFileContent);
	if (fileContent.find(boundary) != string::npos){
		statusCode = BadRequest;
		return;
	}
}

void Request::addFile(){
	// Vanuit config halen waar files moeten worden opgeslagen.
	string uploadPlace = "www/uploads/" + fileName;
	ofstream file(uploadPlace, ios::binary);
	file << fileContent;
	file.close();
}

void Request::postAndDelete(){
	if (Method == "POST" && statusCode == OK){
		extractFileElements();
		statusText = setStatusText(statusCode);
		if (statusCode == OK){
			addFile();
		}
	}
	if (Method == "DELETE" && statusCode == OK){ //  curl -X DELETE localhost:8080/uploads/cat.png;
		string uploadPlace = Path;
		const char *cUploadPlace = uploadPlace.c_str();
		int status = remove(cUploadPlace);
		if (status != 0) {
        	statusCode = BadRequest;
			statusText = setStatusText(statusCode);
		}
	}
}

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
