#include "../inc/Request.hpp"

void Request::extractBody(std::string input, int bodyLen){
	int startBody = input.find("\r\n\r\n");
	startBody += 4;
	Body = input.substr(startBody, bodyLen);
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
	std::string uploadPlace = "www/uploads/" + fileName;
	std::ofstream file(uploadPlace, std::ios::binary);
	file << fileContent;
	file.close();
}

void Request::extractElements(){
	int endOfRequestLine = Input.find("\r\n"); // CRLF is: carriage return(\r) line feed (\n)
	endOfRequestLine -= 2;
	requestLine = Input.substr(0, endOfRequestLine);
	std::stringstream ss(requestLine);
	ss >> Method >> Path >> Protocol;
	// Controle op protocol uitvoeren. Of het wel HTTP/1.1 is
	Path = "www" + Path;
	if (Path == "www/"){
		Path = "www/index.html";
	}
	// Kijken of andere manier is om te zien of het bestaande html pages zijn.
	if (Path == "www/index.html" || Path == "www/uploads.html" || Path == "www/gaia.html"){
		statusCode = "200";
		statusText = "OK";
	}
	// Iets doen voor favicon.
	else{
		Path = "www/404.html";
		statusCode = "404";
		statusText = "Not Found";
	}
	// Check if contentLength is not too big, anders error 413 gooien.
	int contentLength = getContentlength(Input);
	if (Method == "POST"){
		extractBody(Input, contentLength);
		extractFileElements();
		addFile();
	}
}

ssize_t getContentlength(std::string message){
	std::string contentLenStr{};
	size_t startContentLen = message.find("Content-Length:");
	if (startContentLen != std::string::npos){	
		startContentLen += 16;
		size_t endContentLen = message.find("\r\n", startContentLen); // Finds first occurence of \r\n starting at the position of startContentLen.
		contentLenStr = message.substr(startContentLen, (endContentLen - startContentLen));
	}
	ssize_t contentLength{};
	if (!contentLenStr.empty()){
		std::stringstream ss(contentLenStr);
		ss >> contentLength;
	}
	return contentLength;
}

ssize_t	getBytesUntilHeaders(std::string message){
	int posEndHeaders = message.find("\r\n\r\n");
	std::string requestUntilHeaders = message.substr(0, (posEndHeaders + 4));
	ssize_t bytesUntilHeaders = requestUntilHeaders.size();
	return bytesUntilHeaders;
}

std::string Request::getPath(){
	return Path;
}
std::string Request::getMethod(){
	return Method;
}
std::string Request::getStatusCode(){
	return statusCode;
}
std::string Request::getStatusText(){
	return statusText;
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