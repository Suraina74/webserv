#include "../inc/Response.hpp"

int Response::composeResponse()
{
	statusLine = statusLine + " " + request.getStatusText() + "\r\n";
	ssize_t bytesRead = 0;
	if (request.getStatusCode() != OK && request.getStatusCode() != PageNotFound){
		body = R"(
			<!DOCTYPE html>
			<html lang="en">
			<head>
				<meta charset="UTF-8">
				<meta name="viewport" content="width=device-width, initial-scale=1.0">
				<title>Document</title>
			</head>
			<body>
				<h1>)" + request.getStatusText() + R"( ^⩊^</h1>
			</body>
			</html>
			<style>
				body {
					box-sizing: border-box;
					margin: 0;
					font-family: Arial, Helvetica, sans-serif;
					background-color: rgb(255, 233, 244);
					color: hotpink;
					text-align: center;
				}
					h1 {font-size: 7em;}
			</style>
			)";
			bytesRead = body.size();
		}
	else{
		std::string path = request.getPath();
		const char *cPath = path.c_str();
		int fd = open(cPath, O_RDONLY);
		char buffer[2048];
		if (fd >= 0)
		{
			bytesRead = read(fd, buffer, sizeof(buffer));
			if (bytesRead > 0)
			{
				std::string htmlPage(buffer, bytesRead);
				body = htmlPage;
			}
			else
			{
				std::cout << "error" << std::endl;
				return 1;
			}
			close(fd);
		}
	}
	std::string sizeOfBody;
	std::stringstream ss;
	ss << bytesRead;
	ss >> sizeOfBody;
	contentLength = contentLength + sizeOfBody + "\r\n";
	std::string connection = "Connection: close\r\n\r\n";
	fullResponse = statusLine + contentType + contentLength + connection + body;
	return 0;
}

void Response::cleanResponse(){
	request = {};
	statusLine = "HTTP/1.1";
	contentType = "Content-Type: text/html\r\n";
	contentLength = "Content-Length: ";
	body = {};
	fullResponse = {};
	lenResponse = {};
	cFullResponse = {};
	bytesSent = {};
}

void Response::setRequest(Request r){
	request = r;
}

void Response::setCString(const char* cString){
	cFullResponse = cString;
}

void Response::setLenResponse(int length){
	lenResponse = length;
}

void Response::setBytesSent(int bytes){
	bytesSent = bytes;
}

std::string Response::getFullResponse()
{
	return fullResponse;
}

const char* Response::getCFullResponse(){
	return cFullResponse;
}

int Response::getLenResponse()
{
	return lenResponse;
}

int Response::getBytesSent(){
	return bytesSent;
}