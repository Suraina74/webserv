#include "../inc/Request.hpp"

// Validation required!!

void Request::parseRequestLine(){
	size_t endOfRequestLine = partialRequest.find("\r\n"); // CRLF is: carriage return(\r) line feed (\n)
	if (endOfRequestLine == std::string::npos){
		statusCode = BadRequest;
		setStatusText(statusCode);
		return ;
	}
	requestLine = partialRequest.substr(0, endOfRequestLine);
	std::stringstream ss(requestLine);
	ss >> Method >> Path >> Protocol;
	// if (Method.empty() || Path.empty() || Protocol.empty()){
	// }
	// Controle op protocol uitvoeren. Of het wel HTTP/1.1 is
	Path = "www" + Path;
	if (Path == "www/"){
		Path = "www/index.html";
	}
	// Kijken of andere manier is om te zien of het bestaande html pages zijn.
	if (Path != "www/index.html" && Path != "www/uploads.html"){
		Path = "www/404.html";
		statusCode = PageNotFound;
	}
	statusText = setStatusText(statusCode);
	// Iets doen voor favicon.
}