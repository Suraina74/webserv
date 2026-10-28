#include "../inc/Response.hpp"

// Redirections
// Support redirect responses (301/302 + Location header)
// Use custom error pages from config, include server and use the getter.
// If an error html page is missing in the directory, fall back to a simple built-in error response.

int Response::composeResponse(string routePath)
{
	if (request.getStatusCode() == OK){
		ifstream file(routePath, std::ios::binary);
		if (!file.is_open()){
			request.setStatus(PageNotFound);
			request.setStatusText(PageNotFound);
		}
		file.close();
	}
	statusLine = statusLine + " " + request.getStatusText() + "\r\n";
	int bytesRead = 0;
	if (request.getStatusCode() != OK && request.getStatusCode() != PageNotFound)
	{
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
	else
	{
		ifstream file(routePath, std::ios::binary);
		string line;
		if (file.is_open())
		{
			while(getline(file, line)){
				body += line + '\n';
			}
		}
		file.close();
	}
	string sizeOfBody;
	stringstream ss;
	ss << body.size();
	ss >> sizeOfBody;
	contentLength = contentLength + sizeOfBody + "\r\n";
	string connection = "Connection: close\r\n\r\n";
	fullResponse = statusLine + contentType + contentLength + connection + body;
	return 0;
}

void Response::resetResponseVars()
{
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

void Response::setRequest(Request r)
{
	request = r;
}

void Response::setCString(const char* cString)
{
	cFullResponse = cString;
}

void Response::setLenResponse(int length)
{
	lenResponse = length;
}

void Response::setBytesSent(int bytes)
{
	bytesSent = bytes;
}

string Response::getFullResponse() const
{
	return fullResponse;
}

const char* Response::getCFullResponse() const
{
	return cFullResponse;
}

int Response::getLenResponse() const
{
	return lenResponse;
}

int Response::getBytesSent() const
{
	return bytesSent;
}
