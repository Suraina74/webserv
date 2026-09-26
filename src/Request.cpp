#include "../inc/Request.hpp"

// The parsed HTTP request is evaluated against the configuration after parsing.
// Body ook validaten!
// Per belangrijke header kijken wat mag of niet. Content Length, Transfer encoding:chunked, Content Type, Host
// Validate headers in body of Post request?
// Kijken of filename niet leeg is.. 

bool checkIfHex(std::string chunkSize){
	for (size_t i = 0; i < chunkSize.length(); i++){
		if (!isxdigit(chunkSize[i])){
			return false;
		}
	}
	return true;
}

void Request::extractChunkedBody(){
	size_t startBody = fullRequest.find("\r\n\r\n") + 4;
	if (startBody == std::string::npos){
		statusCode = BadRequest;
	}
	size_t endBody = fullRequest.find("0\r\n\r\n");
	if (endBody == std::string::npos){
		statusCode = BadRequest;
	}
	endBody += 1;
	std::string chunkedBody = fullRequest.substr(startBody, endBody - startBody);
	int start = 0;
	std::string part{};
	size_t allChunkSizes{};
	while (1){
		int end = chunkedBody.find("\r\n", start);
		std::string chunkSizeString = chunkedBody.substr(start, end - start);
		// Check if the chunksizestring consists of characters that are allowed as hex:
		if (checkIfHex(chunkSizeString) == false){
			statusCode = BadRequest;
			return ;
		}
		std::stringstream ss(chunkSizeString);
		size_t chunkSize{};
		ss >> std::hex >> chunkSize;
		ss.str("");
		ss.clear();
		allChunkSizes += chunkSize;
		if (allChunkSizes > bytesRead){
			statusCode = BadRequest;
			break ;
		}
		if (chunkSize != 0){
			part = chunkedBody.substr((end + 2), chunkSize);
			Body += part;
			start = end + 2 + chunkSize + 2;
		}
		else{
			break;
		}
	}
}

void Request::extractBody(){
	size_t startBody = fullRequest.find("\r\n\r\n") + 4;
	if (startBody == std::string::npos){
		statusCode = BadRequest;
	}
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
	size_t endOfFileContent = Body.find(boundary + "--");
	if (endOfFileContent == std::string::npos){
		statusCode = BadRequest;
	}
	fileContent = Body.substr(startOfFileContent, endOfFileContent - startOfFileContent);
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
		statusText = setStatusText(statusCode);
	}
	else if (chunked == true && statusCode == OK){
		extractChunkedBody();
		statusText = setStatusText(statusCode);
	}
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

void Request::setRequest(std::string request){
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

size_t Request::getBytesRead(){
	return bytesRead;
}

size_t Request::getHeaderBytes(){
	return headerBytes;
}

bool Request::getChunked(){
	return chunked;
}
