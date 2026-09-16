#include "../inc/Request.hpp"

// The parsed HTTP request is evaluated against the configuration after parsing.
// Body ook validaten!
// Per belangrijke header kijken wat mag of niet.
// Boundary in extractFileElements
// Kijken of filename niet leeg is.. 

void Request::extractChunkedBody(){
	size_t startBody = fullRequest.find("\r\n\r\n") + 4;
	if (startBody == std::string::npos){
		statusCode = BadRequest;
	}
	size_t endBody = fullRequest.find("0\r\n\r\n");
	if (endBody == std::string::npos){
		statusCode = BadRequest;
	}
	std::string chunkedBody = fullRequest.substr(startBody, endBody - startBody);
	int amountLines = 0;
	for (size_t i = 0; i < chunkedBody.length(); i++){
		if (chunkedBody[i] == '\r'){
			amountLines++;
		}
	}
	int start = 0;
	for (int i = 0; i < amountLines; i++){
		int end = chunkedBody.find("\r\n", start);
		if (i % 2 == 0){
			start = end + 2;
			continue;
		}
		std::string part = chunkedBody.substr(start, end - start);
		Body += part;
		start = end + 2;
	}
}

void Request::extractBody(){
	int startBody = fullRequest.find("\r\n\r\n") + 4;
	Body = fullRequest.substr(startBody, contentLength);
}

void Request::extractFileElements(){
	size_t startFilename = Body.find("filename=\"");
	if (startFilename == std::string::npos){
		statusCode = BadRequest;
	}
	startFilename += 10;
	size_t endFilename = Body.find('\"', startFilename);
	if (endFilename == std::string::npos){
		statusCode = BadRequest;
	}
	fileName = Body.substr(startFilename, (endFilename - startFilename));

	size_t startOfFileContent = Body.find("\r\n\r\n");
	if (startOfFileContent == std::string::npos){
		statusCode = BadRequest;
	}
	startOfFileContent += 4;
	size_t endOfFileContent = Body.find("\r\n------WebKit");
	if (endOfFileContent == std::string::npos){
		statusCode = BadRequest;
	}
	fileContent = Body.substr(startOfFileContent, endOfFileContent - startOfFileContent);
	cout << fileContent;
}

void Request::addFile(){
	// Vanuit config halen waar files moeten worden opgeslagen.
	std::string uploadPlace = "www/uploads/" + fileName;
	std::ofstream file(uploadPlace, std::ios::binary);
	file << fileContent;
	file.close();
}

void Request::parseBody(){
	if (contentLength && statusCode == OK){
		extractBody();
	}
	else if (chunked == true && statusCode == OK){
		extractChunkedBody();
	}
	statusText = setStatusText(statusCode);
}

void Request::postAndDelete(){
	if (Method == "POST"){
		extractFileElements();
		addFile();
	}
	if (Method == "DELETE"){ //  curl -X DELETE localhost:8080/uploads/cat.png;
		std::string uploadPlace = Path;
		const char *cUploadPlace = uploadPlace.c_str();
		int status = remove(cUploadPlace);
		if (status != 0) {
        	statusCode = BadRequest;
			statusText = setStatusText(statusCode);
		}
	}
}

std::string Request::setStatusText(httpStatus status){
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

void Request::setRequest(std::string request){
	fullRequest = request;
}

void Request::setBytesRead(ssize_t bytes){
	bytesRead = bytes;
}

void Request::setHeaderBytes(ssize_t bytes){
	headerBytes = bytes;
}

ssize_t Request::getContentLength(){
	return contentLength;
}
std::string Request::getPath(){
	return Path;
}
std::string Request::getMethod(){
	return Method;
}
httpStatus Request::getStatusCode(){
	return statusCode;
}
std::string Request::getStatusText(){
	return statusText;
}

std::string Request::getFullRequest(){
	return fullRequest;
}

std::string Request::getRequestTillHeaders(){
	return requestTillHeaders;
}

ssize_t Request::getBytesRead(){
	return bytesRead;
}

ssize_t Request::getHeaderBytes(){
	return headerBytes;
}

bool Request::getChunked(){
	return chunked;
}

// GET / HTTP/1.1 niets na /
// GET /index.html HTTP/1.1 specifieke html page na /
// GET /favicon.ico HTTP/1.1
// POST /delete.html HTTP/1.1 met body filename=

// Content-Length: 695\r\n

//Lijst van html pages maken. Array of strings met de namen en dan kijken of the html page na / in de lijst staat.
//Als er niets na / komt, dan moet de index page worden getoond

// Met GET weet je dat je alleen de gevraagde html page als string moet meegeven aan send.
// Met POST moet er ook nog iets worden gedaan met de file (wat in de body staat).

// Met GET een string returnen met de html page waarom wordt gevraagd.

// recv can receive the request in parts, but this is not HTTP chunked transfer encoding.

// HTTP chunked transfer encoding is only used when the sender explicitly uses:
// Transfer-Encoding: chunked
// But we use content-length, this is a sign that it is not using HTTP chunked transfer encoding. The receiver should receive content-length bytes after the header section.

// Eerst volledige request krijgen met recv.
// Dan alle elementen eruithalen (parsen).
// Dan valideren.
// Dan kijken wat response moet zijn.