#include "../inc/Response.hpp"

string getFileExtension(string path){
	size_t lastDot = path.find_last_of('.');
	if (lastDot == string::npos){
		return "";
	}
	string fileExtension = path.substr(lastDot + 1, path.length() - lastDot + 1);
	for (size_t i = 0; i < fileExtension.length(); i++)
		fileExtension[i] = tolower(fileExtension[i]);
	return fileExtension;
} 

string convertExtension(string path){
	string extension = getFileExtension(path);
	if (extension == "png"){
		return "image/png";
	}
	else if (extension == "gif"){
		return "image/gif";
	}
	else if (extension == "jpg" || extension == "jpeg"){
		return "image/jpeg";
	}
	else if (extension == "ico"){
		return "image/x-icon";
	}
	else if (extension == "html" || extension == "htm"){
		return "text/html";
	}
	else if (extension == "css"){
		return "text/css";
	}
	else if (extension == "txt"){
		return "text/plain";
	}
	else if (extension == "js"){
		return "text/javascript";
	}
	else if (extension == "pdf"){
		return "application/pdf";
	}
	else if (extension == "doc"){
		return "application/msword";
	}
	else if (extension == "json"){
		return "application/json";
	}
	return "text/html";
}

string setContentType(string path, string method){
	if (method == "GET"){
		return convertExtension(path);
	}
	return "text/html";
}

int Response::composeResponse(string redirection, string routePath, const ServerConfig& server)
{
	if (request.getStatusCode() == OK && !routePath.empty()){
		ifstream file(routePath, std::ios::binary);
		if (!file.is_open()){
			request.setStatus(PageNotFound);
			request.setStatusText(PageNotFound);
		}
		file.close();
	}
	auto it = server.getErrPages().find(request.getStatusCode());
	if (it != server.getErrPages().end())
	{
		routePath = server.getRoot() + it->second;
		ifstream file(routePath, std::ios::binary);
		string line;
		if (file.is_open())
		{
			while(getline(file, line))
			{
				body += line + '\n';
			}
		}
		file.close();
	}
	statusLine += request.getStatusText() + "\r\n";
	if (body.empty() && !(request.getStatusCode() == NoContent || request.getStatusCode() == MovedPermanently))
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
	}
	string sizeOfBody;
	stringstream ss;
	ss << body.size();
	ss >> sizeOfBody;
	contentLength += sizeOfBody + "\r\n";
	contentType += setContentType(routePath, request.getMethod()) + "\r\n";
	string connection = "Connection: close\r\n\r\n";
	if (request.getStatusCode() == MovedPermanently){
		location += redirection + "\r\n";
		fullResponse = statusLine + location + contentLength + connection;
		return 0;
	}
	else if (request.getStatusCode() == NoContent){
		fullResponse = statusLine + contentLength + connection;
		return 0;
	}
	fullResponse = statusLine + contentType + contentLength + connection + body;
	return 0;
}

void Response::resetResponseVars()
{
	request = {};
	statusLine = "HTTP/1.1 ";
	contentType = "Content-Type: ";
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

void Response::setBody(string b)
{
	body = b;
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
