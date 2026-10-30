#include "../inc/Request.hpp"

void Request::extractFileElements()
{
	auto itFilename = contentDisposMap.find("filename");
	if (itFilename == contentDisposMap.end())
	{
		statusCode = UnprocessableContent;
		return;
	}
	fileName = itFilename->second;
	if (fileName.empty() || fileName.find('\\') != string::npos || fileName.find('/') != string::npos)
	{
		statusCode = BadRequest;
		return;
	}
	if (contentLength > 0 || chunked == true)
	{
		size_t startOfFileContent = Body.find("\r\n\r\n");
		if (startOfFileContent == string::npos)
		{
			statusCode = BadRequest;
			return;
		}
		startOfFileContent += 4;
		size_t endOfFileContent = Body.find("\r\n" + boundary + "--");
		if (endOfFileContent == string::npos)
		{
			statusCode = BadRequest;
			return;
		}
		fileContent = Body.substr(startOfFileContent, endOfFileContent - startOfFileContent);
		if (fileContent.find(boundary) != string::npos)
		{
			statusCode = BadRequest;
			return;
		}
	}
}

void Request::addFile(string uploadPath)
{
	string uploadPlace = uploadPath + "/" + fileName;
	ofstream file(uploadPlace, ios::binary);
	if (!file.is_open()){
		statusCode = InternalServerError;
		statusText = setStatusText(statusCode);
	}
	file << fileContent;
	statusCode = Created;
	statusText = setStatusText(statusCode);
	file.close();
}

