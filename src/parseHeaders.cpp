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
			return false;
		}
		else{
			key = line.substr(0, findColon);
			if (key.empty()){
				statusCode = BadRequest;
				return false;
			}
			if (key.find(' ') != std::string::npos){
				statusCode = BadRequest;
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
		if (value.find('\r') != std::string::npos || value.find('\n') != std::string::npos || value.find('\0') != std::string::npos){
			statusCode = BadRequest;
			return false;
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

bool checkIfDoubles(std::string headerName, unordered_multimap<std::string, std::string> map){
	int count = 0;
	for (auto it = map.begin(); it != map.end(); it++){
		if (it->first == headerName){
			count++;
		}
	}
	if (count > 1){
		return false;
	}
	return true;
}

bool Request::checkContentType(){
	auto itCt = headerMap.find("content-type");
	if (itCt != headerMap.end()){
		if (checkIfDoubles("content-type", headerMap) == false){
			statusCode = BadRequest;
			return false;
		}
		std::string str = itCt->second;
		if (str.find("multipart/form-data") == std::string::npos){
			statusCode = UnsupportedMediaType;
			return false;
		}
		size_t begin = str.find("boundary=");
		if (begin == std::string::npos){
			statusCode = BadRequest;
			return false;
		}
		begin += 9;
		boundary = str.substr(begin, (str.length() - begin));
		boundary = "--" + boundary;
	}
	else if (itCt == headerMap.end()){
		statusCode = BadRequest;
		return false;
	}
	return true;
}

bool Request::checkPostHeaders(){
	// Kijken of content-length niet groter is dan een bepaalde grootte.
	auto itCl = headerMap.find("content-length"); // if the key is not present, it returns end().
	auto itTe = headerMap.find("transfer-encoding");
	if (itCl != headerMap.end() && itTe != headerMap.end()){
		statusCode = BadRequest;
		return false;
	}
	if (itCl != headerMap.end()){  //An iterator is a pointer-like object that allows traversing through the elements of a map.
		if (checkIfDoubles("content-length", headerMap) == false){
			statusCode = BadRequest;
			return false;
		}
		std::string contentLenStr = itCl->second; // first = key, second = value of a map.
		if (checkIfOnlyNumbers(contentLenStr) == false){
			statusCode = BadRequest;
			return false;
		}
		std::stringstream ss(contentLenStr);
		ss >> contentLength;
		if (contentLength < 0){
			statusCode = BadRequest;
			return false;
		}
	}
	// if (contentLength > allowed body size in config file){
	//	statusCode = RequestHeaderFieldsTooLarge;
	// }
	else if (itTe != headerMap.end()){
		if (checkIfDoubles("transfer-encoding", headerMap) == false){
			statusCode = BadRequest;
			return false;
		}
		if (itTe->second == "chunked"){
			chunked = true;
		}
		else{
			statusCode = NotImplemented;
			return false;
		}
	}
	else{
		statusCode = BadRequest;
		return false;
	}
	if (checkContentType() == false){
		return false;
	}
	return true;
}

bool Request::validateHeaders(){
	// Headers can only be made up of certain characters. Can be anything voor de rest?
	// Kijken of de host een bestaande host is volgens config file.
	auto itHost = headerMap.find("host");
	if (itHost == headerMap.end()){
		statusCode = BadRequest;
		return false;
	}
	if (checkIfDoubles("host", headerMap) == false){
		statusCode = BadRequest;
		return false;
	}
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
		statusText = setStatusText(statusCode);
		return false;
	}
	if (parseHeaders() == false){
		statusText = setStatusText(statusCode);
		return false;
	}
	if (validateHeaders() == false){
		statusText = setStatusText(statusCode);
		return false;
	}
	return true;
}