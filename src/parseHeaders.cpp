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
			// Delete spaces in front and back of value:
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

// The header name (the key) is case insensitive. So we want to normalize it. So that you can find it.
// Cause you could have: Host, host, hOst, HOST and other variations. If you normalize to only lowercase, you can find it easier.
// Otherwise you don't know what to look for. As a header could be in any form.

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

bool Request::validateHeaders(){

	// for (auto it = headerMap.begin(); it != headerMap.end(); it++){
	// 	std::cout << it->first << it->second << std::endl;
	// }

	auto it = headerMap.find("content-length"); // if the key is not present, it returns end().
	if (it != headerMap.end()){  //An iterator is a pointer-like object that allows traversing through the elements of a map.
		std::string contentLenStr = it->second; // first = key, second = value of a map.
		if (checkIfOnlyNumbers(contentLenStr) == false){
			statusCode = BadRequest;
			statusText = setStatusText(statusCode);
			return false;
		}
		std::stringstream ss(contentLenStr);
		ss >> contentLength;
	}
	if (contentLength < 0){
		statusCode = BadRequest;
	}
	// if (contentLength > body size in config file){
	//	statusCode = RequestHeaderFieldsTooLarge;
	// }
	if (statusCode != OK){
		statusText = setStatusText(statusCode);
		return false;
	}
	return true;
}

bool Request::parseUntilHeaders(std::string hString){
	requestTillHeaders = hString;
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