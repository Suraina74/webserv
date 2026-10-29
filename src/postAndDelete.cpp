#include "../inc/Request.hpp"

// What if upload is too large? 413 Content Too Large.

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

void Request::postAndDelete()
{
	// if (Method == "POST" && statusCode == OK)
	// {
	// 	if (contentLength > 0 || chunked == true){
	// 		extractFileElements();
	// 		statusText = setStatusText(statusCode);
	// 		if (statusCode == OK)
	// 		{
	// 			addFile();
	// 		}
	// 	}
	// }
	if (Method == "DELETE" && statusCode == OK) //  curl -X DELETE localhost:8080/uploads/cat.png;
	{
		string uploadPlace = Path;
		const char *cUploadPlace = uploadPlace.c_str();
		int status = remove(cUploadPlace);
		if (status != 0)
		{
			statusCode = PageNotFound;
			statusText = setStatusText(statusCode);
		}
			statusCode = NoContent;
			statusText = setStatusText(statusCode);
	}
}
