#include "../inc/Request.hpp"

bool Request::validateRequestLine(){
	// Nog wel kijken welke html pages allowed zijn volgens config file.
	if (Path != "www/index.html" && Path != "www/uploads.html"){
		Path = "www/404.html";
		statusCode = PageNotFound;
	}
	// Nog wel kijken welke methods allowed zijn volgens config file.
	// Methods mogen alleen bestaan uit bepaalde karakters. A-Z a-z 0-9 ! # $ % & ' * + - . ^ _ ` | ~
	// Method must contain at least one character.
	if (Method != "GET" && Method != "DELETE" && Method != "POST"){
		statusCode = MethodNotAllowed;
	}
	if (Protocol != "HTTP/1.1"){
		statusCode = HTTPVersionNotSupported;
	}
	if (statusCode != OK){
		statusText = setStatusText(statusCode);
		return false;
	}
	return true;
}

bool Request::parseRequestLine(){
	size_t endOfRequestLine = requestTillHeaders.find("\r\n"); // CRLF is: carriage return(\r) line feed (\n)
	if (endOfRequestLine == std::string::npos){
		statusCode = BadRequest;
		setStatusText(statusCode);
		return false;
	}
	requestLine = requestTillHeaders.substr(0, endOfRequestLine);
	// Check of er 3 woorden in RL zitten.
	std::stringstream ss(requestLine);
	std::string word;
	int amountWords = 0;
	while (ss >> word){
		amountWords++;
	}
	if (amountWords != 3){
		statusCode = BadRequest;
		setStatusText(statusCode);
		return false;
	}
	// Check of er maar 1 space tussen de elements in request line en ook geen andere soorten whitespace in RL;
	int spaces = 0;
	for (size_t i = 0; i < requestLine.size(); i++){
		if(isspace(requestLine[i])){
			if (requestLine[i] != ' '){
				statusCode = BadRequest;
				setStatusText(statusCode);
				return false;
			}
			spaces++;
		}
	}
	if (spaces != 2){
		statusCode = BadRequest;
		setStatusText(statusCode);
		return false;
	}
	ss.clear();
	ss.str(requestLine);
	ss >> Method >> Path >> Protocol;
	Path = "www" + Path;
	if (Path == "www/"){
		Path = "www/index.html";
	}
	// Iets doen voor favicon.
	if (validateRequestLine() == false){
		return false;
	}
	return true;
}