#include "../inc/Request.hpp"

// What if content length = 0, but there is a body?

void Request::extractFileElements(){
	auto itFilename = contentDisposMap.find("filename");
	if (itFilename == contentDisposMap.end()){
		statusCode = BadRequest;
		return;
	}
	fileName = itFilename->second;
	if (fileName.empty() || fileName.find('\\') != string::npos || fileName.find('/') != string::npos){
		statusCode = BadRequest;
		return;
	}
	if (contentLength > 0 || chunked == true){
		size_t startOfFileContent = Body.find("\r\n\r\n");
		if (startOfFileContent == string::npos){
			statusCode = BadRequest;
			return;
		}
		startOfFileContent += 4;
		size_t endOfFileContent = Body.find("\r\n" + boundary + "--");
		if (endOfFileContent == string::npos){
			statusCode = BadRequest;
			return;
		}
		fileContent = Body.substr(startOfFileContent, endOfFileContent - startOfFileContent);
		if (fileContent.find(boundary) != string::npos){
			statusCode = BadRequest;
			return;
		}
	}
}

void Request::addFile(){
	// Vanuit config halen waar files moeten worden opgeslagen.
	if (contentLength > 0 || chunked == true){
		string uploadPlace = "www/uploads/" + fileName;
		ofstream file(uploadPlace, ios::binary);
		file << fileContent;
		file.close();
	}
}

void Request::postAndDelete(){
	if (Method == "POST" && statusCode == OK){
		extractFileElements();
		statusText = setStatusText(statusCode);
		if (statusCode == OK){
			addFile();
		}
	}
	if (Method == "DELETE" && statusCode == OK){ //  curl -X DELETE localhost:8080/uploads/cat.png;
		string uploadPlace = Path;
		const char *cUploadPlace = uploadPlace.c_str();
		int status = remove(cUploadPlace);
		if (status != 0) {
        	statusCode = BadRequest;
			statusText = setStatusText(statusCode);
		}
	}
}