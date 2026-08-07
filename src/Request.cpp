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
	ss.clear();
	size_t startContentLen = Input.find("Content-Length");
	if (startContentLen != std::string::npos){
		size_t endContentLen = Input.find("\r\n", startContentLen);
		contentLengthHeader = Input.substr(startContentLen, (endContentLen - startContentLen));
	}
	if (!contentLengthHeader.empty()){
		std::string _;
		ss.str(contentLengthHeader);
		ss >> _ >> contentLength;
	}

	// Find how many bytes the part is until the header.
	// Als bytes read kleiner is dan bytes until header + bodylen, dan moet je nog lezen t/m until header + bodylen.
	int posEndHeaders = Input.find("\r\n\r\n");
	std::string untilHeaders = Input.substr(0, (posEndHeaders + 4));
	bytesUntilHeaders = untilHeaders.size();
	// Body = Input.substr()
}

ssize_t Request::getContentLength(){
	return contentLength;
}
ssize_t Request::getBytesUntilHeaders(){
	return bytesUntilHeaders;
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