#include "../inc/Request.hpp"

// Check on overflow/Underflow of content length.
// Kijken of content-length niet groter is dan een bepaalde grootte.

bool Request::allowedCharsInKey(string key){
	string allowedChars = "!#$%&'*+-.^_`|~";
	for (size_t i = 0; i < key.length(); i++){
		if (!(key[i] >= 'a' && key[i] <= 'z') && !(key[i] >= 'A' && key[i] <= 'Z') \
		&& !(key[i] >= '0' && key[i] <= '9') && allowedChars.find(key[i]) == string::npos){
			return false;
		}
	}
	return true;
}

bool Request::actionsOnKey(string& key){
	if (key.empty()){
		statusCode = BadRequest;
		return false;
	}
	if (key.find(' ') != string::npos){
		statusCode = BadRequest;
		return false;
	}
	if (allowedCharsInKey(key) == false){
		statusCode = BadRequest;
		return false;
	}
	for (size_t i = 0; i < key.length(); i++){
		key[i] = tolower(key[i]);
	}
	return true;
}

bool Request::actionsOnValue(string& value){
	size_t findNotSpace;
	if (value[0] == ' '){
		findNotSpace = value.find_first_not_of(' ');
		value.erase(0, findNotSpace); // delete N characters starting from pos 0.
	}
	if (value[value.length() - 1] == ' '){
		findNotSpace = value.find_last_not_of(' ');
		value.erase(findNotSpace + 1); // delete everything from pos findNotSpace + 1 onwards.
	}
	if (value.find('\r') != string::npos || value.find('\n') != string::npos || value.find('\0') != string::npos){
		statusCode = BadRequest;
		return false;
	}
	return true;
}

bool Request::parseHeaders(){
	int amountLines = 0;
	for (size_t i = 0; i < requestTillHeaders.size(); i++){
		if (requestTillHeaders[i] == '\r' && requestTillHeaders[i + 1] == '\n'){
			amountLines++;
		}
	}
	amountLines -= 2;
	int startLine = requestTillHeaders.find("\r\n") + 2;
	int endLine = 0;
	for (int i = 0; i < amountLines; i++){
		endLine = requestTillHeaders.find("\r\n", startLine);
		string line = requestTillHeaders.substr(startLine, endLine - startLine);
		string key, value;
		size_t findColon = line.find(':');
		if (findColon == string::npos){
			statusCode = BadRequest;
			return false;
		}
		key = line.substr(0, findColon);
		if (actionsOnKey(key) == false){
			return false;
		}
		value = line.substr(findColon + 1, line.length() - key.length() + 1);
		if (actionsOnValue(value) == false){
			return false;
		}
		headerMap.insert({key, value});
		startLine = endLine + 2;
	}
	return true;
}

bool checkIfDoubleHeader(string headerName, unordered_multimap<string, string> map){
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

bool validateBoundary(string boundary){
	string allowedChars = "()'+_,-./:=?";
	size_t boundaryLen = boundary.length();
	if (boundary.back() == ' '){
		return false;
	}
	if (boundaryLen == 0 || boundaryLen > 70){
		return false;
	}
	for (size_t i = 0; i < boundaryLen; i++){
		if (!(boundary[i] >= 'a' && boundary[i] <= 'z') && !(boundary[i] >= 'A' && boundary[i] <= 'Z') \
		&& !(boundary[i] >= '0' && boundary[i] <= '9') && allowedChars.find(boundary[i]) == string::npos){
			return false;
		}
	}
	return true;
}

bool Request::checkContentType(){
	auto itCt = headerMap.find("content-type");
	if (itCt != headerMap.end()){
		if (checkIfDoubleHeader("content-type", headerMap) == false){
			statusCode = BadRequest;
			return false;
		}
		if (itCt->second.empty()){
			statusCode = BadRequest;
			return false;
		}
		string str = itCt->second;
		if (makeMapOfHeader(contentTypeMap, "multipart/form-data", str) == false){
			statusCode = BadRequest;
			return false;
		}
		auto itBoundary = contentTypeMap.find("boundary");
		if (itBoundary == contentTypeMap.end()){
			statusCode = BadRequest;
			return false;
		}
		if (validateBoundary(itBoundary->second) == false){
			statusCode = BadRequest;
			return false;
		}
		boundary = "--" + itBoundary->second;
	}
	else if (itCt == headerMap.end()){
		statusCode = BadRequest;
		return false;
	}
	return true;
}

bool checkIfOnlyNumbers(string string){
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

bool Request::checkPostHeaders(){
	auto itCl = headerMap.find("content-length"); // if the key is not present, it returns end().
	auto itTe = headerMap.find("transfer-encoding");
	if (itCl != headerMap.end() && itTe != headerMap.end()){
		statusCode = BadRequest;
		return false;
	}
	if (itCl != headerMap.end()){  //An iterator is a pointer-like object that allows traversing through the elements of a map.
		if (checkIfDoubleHeader("content-length", headerMap) == false){
			statusCode = BadRequest;
			return false;
		}
		string contentLenStr = itCl->second; // first = key, second = value of a map.
		if (checkIfOnlyNumbers(contentLenStr) == false){
			statusCode = BadRequest;
			return false;
		}
		stringstream ss(contentLenStr);
		ss >> contentLength;
		if (contentLength < 0){
			statusCode = BadRequest;
			return false;
		}
		// if (contentLength > allowed body size in config file){
		//	statusCode = RequestHeaderFieldsTooLarge;
		// }
	}
	else if (itTe != headerMap.end()){
		if (checkIfDoubleHeader("transfer-encoding", headerMap) == false){
			statusCode = BadRequest;
			return false;
		}
		if (itTe->second == "chunked"){
			chunked = true;
		}
		else if (itTe->second.empty()){
			statusCode = BadRequest;
			return false;
		}
		else if (!(itTe->second.empty())){
			statusCode = NotImplemented;
			return false;
		}
	}
	else{
		contentLength = 0;
		statusCode = BadRequest;
		return false;
	}
	if (checkContentType() == false){
		return false;
	}
	return true;
}

bool Request::validateHeaders(){
	// Kijken of de host een bestaande host is volgens config file.
	auto itHost = headerMap.find("host");
	if (itHost == headerMap.end()){
		statusCode = BadRequest;
		return false;
	}
	if (itHost->second.empty()){
		statusCode = BadRequest;
		return false;
	}
	if (checkIfDoubleHeader("host", headerMap) == false){
		statusCode = BadRequest;
		return false;
	}
	if (Method == "POST"){
		if (checkPostHeaders() == false)
			return false;
	}
	return true;
}

bool Request::parseUntilHeaders(string string){
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