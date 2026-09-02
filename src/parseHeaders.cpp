#include "../inc/Request.hpp"

void Request::parseHeaders(){
	int amountLines = 0;
	for (size_t i = 0; i < partialRequest.size(); i++){
		if (partialRequest[i] == '\r'){
			amountLines++;
		}
	}
	amountLines -= 2;
	int startLine = partialRequest.find("\r\n") + 2;
	int endLine = 0;
	for (int i = 0; i < amountLines; i++){
		endLine = partialRequest.find("\r\n", startLine);
		std::string line = partialRequest.substr(startLine, endLine - startLine);
		std::stringstream ss(line);
		std::string key, value;
		ss >> key;
		key.pop_back();
		getline(ss, value);
		value.erase(0, 1);
		headerMap.insert({key, value});
		ss.str("");
		ss.clear();
		startLine = endLine + 2;
	}
	auto it = headerMap.find("Content-Length");
	if (it != headerMap.end()){  //An iterator is a pointer-like object that allows traversing through the elements of a map.
		std::string contentLenStr = it->second; // first = key, second = value of a map.
		std::stringstream ss(contentLenStr);
		ss >> contentLength;
	}
}

bool Request::validateHeaders(){
	if (contentLength < 0){
		std::cout << "Invalid content length" << std::endl;
		return false;
	}
	statusText = setStatusText(statusCode);
	return true;
}

bool Request::parseUntilHeaders(std::string hString){
	partialRequest = hString;
	if (parseRequestLine() == false){
		return false;
	}
	parseHeaders();
	if (validateHeaders() == false){
		return false;
	}
	return true;
}