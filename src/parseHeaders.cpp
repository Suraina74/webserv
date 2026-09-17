#include "../inc/Request.hpp"

bool Request::parseHeaders(){
	int amountLines = 0;
	for (size_t i = 0; i < requestTillHeaders.size(); i++){
		if (requestTillHeaders[i] == '\r'){
			amountLines++;
		}
	}
	amountLines -= 2;
	int startLine = requestTillHeaders.find("\r\n") + 2;
	int endLine = 0;
	for (int i = 0; i < amountLines; i++){
		endLine = requestTillHeaders.find("\r\n", startLine);
		std::string line = requestTillHeaders.substr(startLine, endLine - startLine);
		std::string key, value;
		size_t findColon = line.find(':');
		if (findColon == std::string::npos){
			statusCode = BadRequest;
			statusText = setStatusText(statusCode);
			return false;
		}
		else{
			key = line.substr(0, findColon);
			if (key.empty()){
				statusCode = BadRequest;
				statusText = setStatusText(statusCode);
				return false;
			}
			if (key.find(' ') != std::string::npos){
				statusCode = BadRequest;
				statusText = setStatusText(statusCode);
				return false;
			}
			// Normalize header name:
			for (size_t i = 0; i < key.length(); i++){
				key[i] = std::tolower(key[i]);
			}
			value = line.substr(findColon + 1, line.length() - key.length() + 1);
			size_t findNotSpace = value.find_first_not_of(' ');
			value.erase(0, findNotSpace); // delete N characters starting from pos 0.
			findNotSpace = value.find_last_not_of(' ');
			value.erase(findNotSpace + 1); // delete everything from pos findNotSpace + 1 onwards.
		}
		headerMap.insert({key, value});
		startLine = endLine + 2;
	}
	return true;
}

bool checkIfOnlyNumbers(std::string string){
	if (string.empty()){
		return false;
	}
	for (size_t i = 0; i < string.size(); i++){
		if (!isdigit(string[i])){
			return false;
		}
	}
	return true;
}

bool Request::checkContentType(){
	auto itCt = headerMap.find("content-type");
	if (itCt != headerMap.end()){
		std::string str = itCt->second;
		if (str.find("multipart/form-data") == std::string::npos){
			statusCode = UnsupportedMediaType;
			statusText = setStatusText(statusCode);
			return false;
		}
		size_t begin = str.find("boundary=") + 9;
		boundary = str.substr(begin, (str.length() - begin));
		boundary = "--" + boundary;
	}
	else if (itCt == headerMap.end()){
		statusCode = BadRequest;
		statusText = setStatusText(statusCode);
		return false;
	}
	return true;
}

bool Request::checkPostHeaders(){
	auto itCl = headerMap.find("content-length"); // if the key is not present, it returns end().
	auto itTe = headerMap.find("transfer-encoding");
	if (itCl != headerMap.end() && itTe != headerMap.end()){
		statusCode = BadRequest;
		statusText = setStatusText(statusCode);
		return false;
	}
	if (itCl != headerMap.end()){  //An iterator is a pointer-like object that allows traversing through the elements of a map.
		std::string contentLenStr = itCl->second; // first = key, second = value of a map.
		if (checkIfOnlyNumbers(contentLenStr) == false){
			statusCode = BadRequest;
			statusText = setStatusText(statusCode);
			return false;
		}
		std::stringstream ss(contentLenStr);
		ss >> contentLength;
		if (contentLength < 0){
			statusCode = BadRequest;
			statusText = setStatusText(statusCode);
			return false;
		}
	}
	// if (contentLength > allowed body size in config file){
	//	statusCode = RequestHeaderFieldsTooLarge;
	// }
	else if (itTe != headerMap.end()){
		if (itTe->second == "chunked"){
			chunked = true;
		}
		else{
			statusCode = NotImplemented;
			statusText = setStatusText(statusCode);
			return false;
		}
	}
	else{
		statusCode = BadRequest;
		statusText = setStatusText(statusCode);
		return false;
	}
	if (checkContentType() == false){
		return false;
	}
	return true;
}

bool Request::validateHeaders(){

	// for (auto it = headerMap.begin(); it != headerMap.end(); it++){
	// 	std::cout << it->first << it->second << std::endl;
	// }

	// Check headers that can't appear multiple times. Content length en host for example.
	// Headers can only be made up of certain characters. Can be anything voor de rest?
	// Host, connection, content length, content type. Laatste twee alleen bij Post method.
	auto itHost = headerMap.find("host");
	if (itHost == headerMap.end()){
		statusCode = BadRequest;
		statusText = setStatusText(statusCode);
		return false;
	}
	// Check for connection header?
	if (Method == "POST"){
		if (checkPostHeaders() == false)
			return false;
	}
	return true;
}

bool Request::parseUntilHeaders(std::string string){
	int endHeaders = string.find("\r\n\r\n") + 4;
	requestTillHeaders = string.substr(0, endHeaders);
	if (parseRequestLine() == false){
		return false;
	}
	if (parseHeaders() == false){
		return false;
	}
	if (validateHeaders() == false){
		return false;
	}
	return true;
}