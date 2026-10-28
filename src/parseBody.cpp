#include "../inc/Request.hpp"

// Check if chunk size exceeds body size limit.

bool checkIfDoubles(unordered_multimap<string, string> map)
{
	for (auto it = map.begin(); it != map.end(); it++)
	{
		string key = it->first;
		int count = 0;
		for (auto i = map.begin(); i != map.end(); i++)
		{
			if (i->first == key)
			{
				count++;
			}
		}
		if (count > 1)
		{
			return false;
		}
	}
	return true;
}

bool Request::makeMapOfHeader(unordered_multimap<string, string>& map, string word, string headerValue)
{
	string part{};
	vector<string> vector{};
	stringstream ss(headerValue);
	while (getline(ss, part, ';'))
	{
		size_t findNotSpace;
		if (part[0] == ' ')
		{
			findNotSpace = part.find_first_not_of(' ');
			part.erase(0, findNotSpace);
		}
		if (part[part.length() - 1] == ' ')
		{
			findNotSpace = part.find_last_not_of(' ');
			part.erase(findNotSpace + 1);
		}
		vector.push_back(part);
	}
	ss.str("");
	ss.clear();
	string firstElement = vector[0];
	for (size_t i = 0; i < firstElement.length(); i++)
	{
		firstElement[i] = tolower(firstElement[i]);
	}
	if (firstElement != word)
	{
		statusCode = UnsupportedMediaType;
		return false;
	}
	string key, value;
	for (size_t it = 0; it < vector.size(); it++)
	{
		part = vector[it];
		if (part.find('=') == string::npos)
		{
			map.insert({part, ""});
		}
		else
		{
			size_t equalSign = part.find('=');
			key = part.substr(0, equalSign);
			for (size_t i = 0; i < key.length(); i++)
			{
				key[i] = tolower(key[i]);
			}
			size_t startValue = equalSign + 1;
			value = part.substr(startValue, part.size() - startValue);
			if (value.front() == '"' && value.back() == '"')
			{
				value.erase(0, 1);
				value.pop_back();
			}
			else if ((value.front() != '"' && value.back() == '"') || (value.front() == '"' && value.back() != '"'))
			{
				return false;
			}
			else if (value.find(' ') != string::npos)
			{
				return false;
			}
			map.insert({key, value});
		}
	}
	return true;
}

bool Request::checkContentDisposition(string valueCD)
{
	if (makeMapOfHeader(contentDisposMap, "form-data", valueCD) == false)
	{
		statusCode = BadRequest;
		return false;
	}
	auto itName = contentDisposMap.find("name");
	if (itName == contentDisposMap.end())
	{
		statusCode = BadRequest;
		return false;
	}
	if (checkIfDoubles(contentDisposMap) == false)
	{
		statusCode = BadRequest;
		return false;
	}
	return true;
}

void Request::validateBody()
{
	size_t pos = Body.find(boundary + "\r\n");
	if (pos == string::npos || pos != 0)
	{
		statusCode = BadRequest;
		return;
	}
	size_t headerStart = boundary.length() + 2;
	size_t blankLine = Body.find("\r\n\r\n");
	if (blankLine == string::npos)
	{
		statusCode = BadRequest;
		return;
	}
	string key, value;
	while (1)
	{
		size_t headerEnd = Body.find("\r\n", headerStart);
		if (headerEnd == string::npos)
		{
			statusCode = BadRequest;
			return;
		}
		string bodyHeader = Body.substr(headerStart, headerEnd - headerStart);
		size_t colon = bodyHeader.find(':');
		if (colon == string::npos)
		{
			statusCode = BadRequest;
			return;
		}
		key = bodyHeader.substr(0, colon);
		for (size_t i = 0; i < key.length(); i++)
		{
			key[i] = tolower(key[i]);
		}
		if (actionsOnKey(key) == false)
		{
			return;
		}
		value = bodyHeader.substr(colon + 1, bodyHeader.length() - key.length() + 1);
		if (actionsOnValue(value) == false)
		{
			return;
		}
		bodyHeaderMap.insert({key, value});
		headerStart = headerEnd + 2;
		if (headerEnd == blankLine)
		{
			break;
		}
	}
	auto it = bodyHeaderMap.find("content-disposition");
	if (it == bodyHeaderMap.end())
	{
		statusCode = BadRequest;
		return;
	}
	string valueCD = it->second;
	if (checkContentDisposition(valueCD) == false)
	{
		return;
	}
	if (checkIfDoubles(bodyHeaderMap) == false)
	{
		statusCode = BadRequest;
		return;
	}
	// Misschien wordt niet alles in een keer ontvangen met recv, dan eindigt het niet met boundary + -- + \r\n
	size_t endBodyPos = Body.length();
	size_t endBoundary = Body.find("\r\n" + boundary + "--");
	if (endBoundary == string::npos)
	{
		statusCode = BadRequest;
		return;
	}
	endBoundary += (2 + boundary.length() + 4);
	if (endBoundary != endBodyPos)
	{
		statusCode = BadRequest;
		return;
	}
}

bool checkIfHex(string chunkSize)
{
	for (size_t i = 0; i < chunkSize.length(); i++)
	{
		if (!isxdigit(chunkSize[i]))
		{
			return false;
		}
	}
	return true;
}

void Request::extractChunkedBody(){
	size_t startBody = fullRequest.find("\r\n\r\n");
	if (startBody == string::npos){
		statusCode = BadRequest;
        return;
	}
	startBody += 4;
	size_t endBody = fullRequest.find("0\r\n\r\n");
	if (endBody == string::npos){
		statusCode = BadRequest;
        return;
	}
	endBody += 2;
	string chunkedBody = fullRequest.substr(startBody, endBody - startBody);
	int start = 0;
	string chunk{};
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
		if (chunkSize != 0){
			// chunk is bigger than what is left in body.
			size_t lenBodyWithoutChunkSize = chunkedBody.length() - (end + 2);
			if (chunkSize > lenBodyWithoutChunkSize){
				stateOfParser = INCOMPLETE;
				break;
			}
			size_t endOfChunk = end + 2 + chunkSize;
			// check if chunk does ends with /r/n, which it should
			if (chunkedBody[endOfChunk + 1] != '\r' && chunkedBody[endOfChunk + 2] != '\n'){
				statusCode = BadRequest;
				break;
			}
			chunk = chunkedBody.substr((end + 2), chunkSize);
			Body += chunk;
			start = end + 2 + chunkSize + 2;
		}
		else{
			stateOfParser = COMPLETE;
			break;
		}
	}
}

void Request::extractBody()
{
	if (Method != "POST")
	{
		statusCode = BadRequest;
		return;
	}
	size_t startBody = fullRequest.find("\r\n\r\n" + boundary + "\r\n");
	if (startBody == string::npos)
	{
		statusCode = BadRequest;
		return;
	}
	startBody += 4;
	Body = fullRequest.substr(startBody, contentLength);
}

void Request::parseBody()
{
	cout << getFullRequest() << endl;
	if (contentLength && statusCode == OK)
	{
		extractBody();
		statusText = setStatusText(statusCode);
		if (statusCode == OK)
		{
			validateBody();
			statusText = setStatusText(statusCode);
		}
	}
	else if (chunked == true && statusCode == OK)
	{
		extractChunkedBody();
		statusText = setStatusText(statusCode);
		if (statusCode == OK && stateOfParser == COMPLETE)
		{
			validateBody();
			statusText = setStatusText(statusCode);
		}
	}
}
