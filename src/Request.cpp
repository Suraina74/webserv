#include "../inc/Request.hpp"

void Request::extractElements(){
	int posCRLF = Input.find("\r\n"); // Each line ends with \r\n.
	requestLine = Input.substr(0, posCRLF);

	int i = 0;
	int amountElements = 2;
	std::stringstream ss(requestLine);
	std::string word;
	while (i < amountElements){
		ss >> word;
		if (i == 0){
			Method = word;
		}
		else{
			Path = word;
		}
		i++;
	}
	if (Path == "/" || Path == "/index.html" || Path == "/uploads.html"){
		statusCode = "200";
		statusText = "OK";
	}
}

ssize_t getContentlength(std::string message){
	std::string contentLengthHeader{};
	size_t startContentLen = message.find("Content-Length");
	if (startContentLen != std::string::npos){
		size_t endContentLen = message.find("\r\n", startContentLen);
		contentLengthHeader = message.substr(startContentLen, (endContentLen - startContentLen));
	}
	std::stringstream ss(message);
	ssize_t contentLength{};
	if (!contentLengthHeader.empty()){
		std::string _;
		ss.str(contentLengthHeader);
		ss >> _ >> contentLength;
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
std::string Request::getStatusCode(){
	return statusCode;
}

// GET / HTTP/1.1 niets na /
// GET /index.html HTTP/1.1 specifieke html page na /
// GET /style.css HTTP/1.1
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