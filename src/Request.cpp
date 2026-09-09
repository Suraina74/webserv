#include "../inc/Request.hpp"


// 	// if (Method == "DELETE"){
// 	// 	std::string uploadPlace = "www/uploads/" + fileName;
// 	// 	const char *cUploadPlace = uploadPlace.c_str();
// 	// 	int status = remove(cUploadPlace);
// 	// 	if (status != 0) {
//     //     	std::cout << "Error deleting file" << std::endl;
// 	// 	}
// 	// }


void Request::extractBody(){
	int startBody = fullRequest.find("\r\n\r\n");
	startBody += 4;
	Body = fullRequest.substr(startBody, contentLength);
}

void Request::extractFileElements(){
	size_t startFilename = Body.find("filename=\"");
	startFilename += 10;
	size_t endFilename = Body.find('\"', startFilename);
	fileName = Body.substr(startFilename, (endFilename - startFilename));

	size_t startOfFileContent = Body.find("\r\n\r\n");
	startOfFileContent += 4;
	size_t endOfFileContent = Body.find("\r\n------WebKit");
	fileContent = Body.substr(startOfFileContent, endOfFileContent - startOfFileContent);
}

void Request::addFile(){
	// Vanuit config halen waar files moeten worden opgeslagen.
	std::string uploadPlace = "www/uploads/" + fileName;
	std::ofstream file(uploadPlace, std::ios::binary);
	file << fileContent;
	file.close();
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
		case HTTPVersionNotSupported:
			return "505 HTTP Version Not Supported";
	}
}

void Request::parseBody(){
	// Als het gaat om chuncked transfer, dan is er alleen geen contentLength, maar kan nog steeds wel een body zijn.
	if (contentLength && statusCode == OK){
		extractBody();
		extractFileElements();
		addFile();
		statusText = setStatusText(statusCode);
	}
}

void Request::cleanRequest(){
	fullRequest = {};
	requestTillHeaders = {};
	headerBytes = {};
	partialRequest = {};
	bytesRead = {};
	requestLine = {};
	headerMap = {};
	contentLength = {};
	Method = {};
	Protocol = {};
	Path = {};
	statusText = "200 OK";
	Body = {};
	fileName = {};
	fileContent = {};
	statusCode = OK;
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