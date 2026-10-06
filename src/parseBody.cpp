#include "../inc/Request.hpp"

// The parsed HTTP request is evaluated against the configuration after parsing.
// Per belangrijke header kijken wat mag of niet. Content Length, Transfer encoding:chunked, Content Type, Host
// Kijken of alle lines in request eindigen met /r/n. if /r see if next is /n.

// Content type and filename in content disp. are optional.
// Kijken of filename niet leeg is..
// Ignore unknown headers in body.

bool checkIfDoubles(unordered_multimap<string, string> map){
	for (auto it = map.begin(); it != map.end(); it++){
		string key = it->first;
		int count = 0;
		for (auto i = map.begin(); i != map.end(); i++){
			if (i->first == key){
				count++;
			}
		}
		if (count > 1){
			return false;
		}
	}
	return true;
}

bool Request::checkContentDisposition(string valueCD){
	string part{};
	vector<string> contentDisVector{};
	stringstream ss(valueCD);
	while (getline(ss, part, ';')){
		size_t findNotSpace;
		if (part[0] == ' '){
			findNotSpace = part.find_first_not_of(' ');
			part.erase(0, findNotSpace);
		}
		if (part[part.length() - 1] == ' '){
			findNotSpace = part.find_last_not_of(' ');
			part.erase(findNotSpace + 1);
		}
		contentDisVector.push_back(part);
	}
	string firstElement = contentDisVector[0];
	for (size_t i = 0; i < firstElement.length(); i++){
		firstElement[i] = tolower(firstElement[i]);
	}
	if (firstElement != "form-data"){
		statusCode = BadRequest;
		return false;
	}
	string key, value;
	for (size_t it = 0; it < contentDisVector.size(); it++){
		part = contentDisVector[it];
		if (part.find('=') == string::npos){
			contentDisposMap.insert({part, ""});
		}
		else{
			size_t equalSign = part.find('=');
			key = part.substr(0, equalSign);
			for (size_t i = 0; i < key.length(); i++){
				key[i] = tolower(key[i]);
			}
			if (part[equalSign + 1] == '"' && part.back() == '"'){
				part.erase(0, 1);
				part.pop_back();
			}
			size_t startValue = equalSign + 1;
			value = part.substr(startValue, part.size() - startValue);
			contentDisposMap.insert({key, value});
		}
	}
	auto itName = contentDisposMap.find("name");
	if (itName == contentDisposMap.end()){
		statusCode = BadRequest;
		return false;
	}
	if (checkIfDoubles(contentDisposMap) == false){
		statusCode = BadRequest;
		return false;
	}
	return true;
}

void Request::validateBody(){
	size_t pos = Body.find(boundary + "\r\n");
	if (pos == string::npos || pos != 0){
		statusCode = BadRequest;
		return;
	}
	size_t start = boundary.length() + 2;
	size_t blankLine = Body.find("\r\n\r\n");
	if (blankLine == string::npos){
		statusCode = BadRequest;
		return;
	}
	string key, value;
	while (1){
		size_t end = Body.find("\r\n", start);
		string bodyHeaders = Body.substr(start, end - start);
		size_t colon = bodyHeaders.find(':');
		if (colon == string::npos){
			statusCode = BadRequest;
			return;
		}
		key = bodyHeaders.substr(0, colon);
		if (actionsOnKey(key) == false){
			return;
		}
		value = bodyHeaders.substr(colon + 1, bodyHeaders.length() - key.length() + 1);
		if (actionsOnValue(value) == false){
			return;
		}
		bodyHeaderMap.insert({key, value});
		start = end + 2;
		if (end == blankLine){
			break;
		}
	}
	auto it = bodyHeaderMap.find("content-disposition");
	if (it == bodyHeaderMap.end()){
		statusCode = BadRequest;
		return;
	}
	string valueCD = it->second;
	if (checkContentDisposition(valueCD) == false){
		return;
	}
	if (checkIfDoubles(bodyHeaderMap) == false){
		statusCode = BadRequest;
		return;
	}
	size_t endBodyPos = Body.length();
	size_t endBoundary = Body.find("\r\n" + boundary + "--");
	if (endBoundary == string::npos){
		statusCode = BadRequest;
		return;
	}
	endBoundary += (2 + boundary.length() + 2);
	if (endBoundary != endBodyPos){
		statusCode = BadRequest;
		return;
	}
}

bool checkIfHex(string chunkSize){
	for (size_t i = 0; i < chunkSize.length(); i++){
		if (!isxdigit(chunkSize[i])){
			return false;
		}
	}
	return true;
}

void Request::extractChunkedBody(){
	size_t startBody = fullRequest.find("\r\n\r\n") + 4;
	if (startBody == string::npos){
		statusCode = BadRequest;
        return;
	}
	size_t endBody = fullRequest.find("0\r\n\r\n");
	if (endBody == string::npos){
		statusCode = BadRequest;
        return;
	}
	endBody += 1;
	string chunkedBody = fullRequest.substr(startBody, endBody - startBody);
	int start = 0;
	string part{};
	size_t allChunkSizes{};
	while (1){
		int end = chunkedBody.find("\r\n", start);
		string chunkSizeString = chunkedBody.substr(start, end - start);
		if (checkIfHex(chunkSizeString) == false){
			statusCode = BadRequest;
			return;
		}
		stringstream ss(chunkSizeString);
		size_t chunkSize{};
		ss >> hex >> chunkSize;
		ss.str("");
		ss.clear();
		allChunkSizes += chunkSize;
		if (allChunkSizes > bytesRead){
			statusCode = BadRequest;
			break;
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
	size_t startBody = fullRequest.find("\r\n\r\n");
	if (startBody == string::npos){
		statusCode = BadRequest;
        return;
	}
	startBody += 4;
	Body = fullRequest.substr(startBody, (contentLength - 2));
}

void Request::parseBody(){
	if (contentLength && statusCode == OK){
		extractBody();
		statusText = setStatusText(statusCode);
		if (statusCode == OK){
			validateBody();
			statusText = setStatusText(statusCode);
		}
	}
	else if (chunked == true && statusCode == OK){
		extractChunkedBody();
		statusText = setStatusText(statusCode);
		if (statusCode == OK){
			validateBody();
			statusText = setStatusText(statusCode);
		}
	}
}