#include "../inc/Request.hpp"

// The parsed HTTP request is evaluated against the configuration after parsing.
// Body ook validaten!
// Per belangrijke header kijken wat mag of niet. Content Length, Transfer encoding:chunked, Content Type, Host
// Kijken of filename niet leeg is..
// Kijken of alle lines in request eindigen met /r/n. if /r see if next is /n.

bool checkIfHex(string chunkSize)
{
	for (size_t i = 0; i < chunkSize.length(); i++)
	{
		if (!isxdigit(chunkSize[i]))
			return false;
	}
	return true;
}

void Request::extractChunkedBody()
{
	size_t startBody = fullRequest.find("\r\n\r\n") + 4;
	if (startBody == string::npos)
		statusCode = BadRequest;
	size_t endBody = fullRequest.find("0\r\n\r\n");
	if (endBody == string::npos)
		statusCode = BadRequest;
	endBody += 1;
	string chunkedBody = fullRequest.substr(startBody, endBody - startBody);
	int start = 0;
	string part{};
	size_t allChunkSizes{};
	while (1){
		int end = chunkedBody.find("\r\n", start);
		string chunkSizeString = chunkedBody.substr(start, end - start);
		// Check if the chunksizestring consists of characters that are allowed as hex:
		if (checkIfHex(chunkSizeString) == false)
		{
			statusCode = BadRequest;
			return ;
		}
		stringstream ss(chunkSizeString);
		size_t chunkSize{};
		ss >> hex >> chunkSize;
		ss.str("");
		ss.clear();
		allChunkSizes += chunkSize;
		if (allChunkSizes > bytesRead)
		{
			statusCode = BadRequest;
			break ;
		}
		if (chunkSize != 0)
		{
			part = chunkedBody.substr((end + 2), chunkSize);
			Body += part;
			start = end + 2 + chunkSize + 2;
		}
		else
			break;
	}
}

void Request::extractBody()
{
	size_t startBody = fullRequest.find("\r\n\r\n");
	if (startBody == string::npos)
		statusCode = BadRequest;
	startBody += 4;
	Body = fullRequest.substr(startBody, contentLength);
}

bool checkIfDoubles(map<string, string> map)
{
	int countCD = 0;
	int countCT = 0;
	for (auto it = map.begin(); it != map.end(); it++)
	{
		if (it->first == "content-disposition")
			countCD++;
		else if (it->first == "content-type")
			countCT++;
	}
	if (countCD > 1 || countCT > 1)
		return false;
	return true;
}

void Request::validateBody(){
	size_t pos = Body.find(boundary + "\r\n");
	if (pos == string::npos || pos != 0)
	{
		statusCode = BadRequest;
		return;
	}
	size_t start = boundary.length() + 2;
	size_t blankLine = Body.find("\r\n\r\n");
	if (blankLine == string::npos)
	{
		statusCode = BadRequest;
		return;
	}
	string key, value;
	while (1)
	{
		size_t end = Body.find("\r\n", start);
		string partHeader = Body.substr(start, end - start);
		size_t colon = partHeader.find(':');
		if (colon == string::npos)
		{
			statusCode = BadRequest;
			return;
		}
		key = partHeader.substr(0, colon);
		if (actionsOnKey(key) == false)
			return;
		value = partHeader.substr(colon + 1, partHeader.length() - key.length() + 1);
		if (actionsOnValue(value) == false)
			return;
		partHeaderMap.insert({key, value});
		start = end + 2;
		if (end == blankLine)
			break;
	}
	auto it = partHeaderMap.find("content-disposition");
	if (it == partHeaderMap.end())
	{
		statusCode = BadRequest;
		return;
	}
	string valueCT = it->second;
	string part{};
	vector<string> contentDisVector{};
	stringstream ss(valueCT);
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
		contentDisVector.push_back(part);
	}
	// Kijken of form-data op 1e plek zit.
	if (contentDisVector[0] != "form-data")
	{
		statusCode = BadRequest;
		return ;
	}
	for (size_t it = 0; it < contentDisVector.size(); it++)
	{
		part = contentDisVector[it];
		if (part.find('=') == string::npos)
		{
			// Key lower case maken.
			for (size_t i = 0; i < part.length(); i++)
				part[i] = tolower(part[i]);
			contentDisposMap.insert({part, ""});
		}
		else
		{
			size_t equalSign = part.find('=');
			key = part.substr(0, equalSign);
			// Key lower case maken.
			for (size_t i = 0; i < key.length(); i++)
				key[i] = tolower(key[i]);
			if (part[equalSign + 1] == '"' && part.back() == '"')
			{
				part.erase(0, 1);
				part.pop_back();
			}
			size_t startValue = equalSign + 1;
			value = part.substr(startValue, part.size() - startValue);
			contentDisposMap.insert({key, value});
		}
	}
	auto itName = contentDisposMap.find("name");
	if (itName == contentDisposMap.end())
	{
		statusCode = BadRequest;
		return ;
	}
	// Dubbele dingen checken in content disposition.



	// form-data; name="filename"; filename="cat.jpg"
	// Alle onderdelen van content disposition in een vector zetten.
	// Must have form-data and name in it.

	// size_t locName = valueCT.find("name");
	// if (locName == string::npos){
	// 	statusCode = BadRequest;
	// 	return;	
	// }
	// size_t locSemicolon = valueCT.find(';', locName);
	// if (locSemicolon == string::npos){
	// 	statusCode = BadRequest;
	// 	return;
	// }
	// string name = valueCT.substr(locName, locSemicolon - locName);
	if (checkIfDoubles(partHeaderMap) == false){
		statusCode = BadRequest;
		return;
	}
	// for (auto it = partHeaderMap.begin(); it != partHeaderMap.end(); it++){
	// 	cout << it->first << " " << it->second << endl;
	// }
}

void Request::parseBody()
{
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
		if (statusCode == OK)
		{
			validateBody();
			statusText = setStatusText(statusCode);
		}
	}
}

void Request::extractFileElements()
{
	size_t startFilename = Body.find("filename=\"");
	if (startFilename == string::npos)
		statusCode = BadRequest;
	startFilename += 10;
	size_t endFilename = Body.find('\"', startFilename);
	if (endFilename == string::npos)
		statusCode = BadRequest;
	fileName = Body.substr(startFilename, (endFilename - startFilename));
	size_t startOfFileContent = Body.find("\r\n\r\n");
	if (startOfFileContent == string::npos)
		statusCode = BadRequest;
	startOfFileContent += 4;
	size_t endOfFileContent = Body.find("\r\n" + boundary + "--");
	if (endOfFileContent == string::npos)
		statusCode = BadRequest;
	fileContent = Body.substr(startOfFileContent, endOfFileContent - startOfFileContent);
}

void Request::addFile()
{
	// Vanuit config halen waar files moeten worden opgeslagen.
	string uploadPlace = "www/uploads/" + fileName;
	ofstream file(uploadPlace, ios::binary);
	file << fileContent;
	file.close();
}

void Request::postAndDelete()
{
	if (Method == "POST" && statusCode == OK)
	{
		extractFileElements();
		statusText = setStatusText(statusCode);
		if (statusCode == OK)
			addFile();
	}
	if (Method == "DELETE" && statusCode == OK)
	{ //  curl -X DELETE localhost:8080/uploads/cat.png;
		string uploadPlace = Path;
		const char *cUploadPlace = uploadPlace.c_str();
		int status = remove(cUploadPlace);
		if (status != 0) 
		{
        	statusCode = BadRequest;
			statusText = setStatusText(statusCode);
		}
	}
}

string Request::setStatusText(httpStatus status)
{
	switch (status)
	{
		case OK:
			return "200 OK";
		case BadRequest:
			return "400 Bad Request";
		case PageNotFound:
			return "404 Page Not Found";
		case MethodNotAllowed:
			return "405 Method Not Allowed";
		case RequestTimeout:
			return "408 Request Timeout";
		case ContentTooLarge:
			return "413 Content Too Large";
		case URITooLong:
			return "414 URI Too Long";
		case UnsupportedMediaType:
			return "415 Unsupported Media Type";
		case RequestHeaderFieldsTooLarge:
			return "431 Request Header Fields Too Large";
		case InternalServerError:
			return "500 Internal Server Error";
		case NotImplemented:
			return "501 Not Implemented";
		case HTTPVersionNotSupported:
			return "505 HTTP Version Not Supported";
	}
}

void Request::setRequest(string request)
{
	fullRequest = request;
}

void Request::setBytesRead(size_t bytes)
{
	bytesRead = bytes;
}

void Request::setHeaderBytes(size_t bytes)
{
	headerBytes = bytes;
}

size_t Request::getContentLength()
{
	return contentLength;
}
string Request::getPath()
{
	return Path;
}
string Request::getMethod()
{
	return Method;
}
httpStatus Request::getStatusCode()
{
	return statusCode;
}
string Request::getStatusText()
{
	return statusText;
}

string Request::getFullRequest()
{
	return fullRequest;
}

string Request::getRequestTillHeaders()
{
	return requestTillHeaders;
}

size_t Request::getBytesRead()
{
	return bytesRead;
}

size_t Request::getHeaderBytes()
{
	return headerBytes;
}

bool Request::getChunked()
{
	return chunked;
}
