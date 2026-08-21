#include "../inc/Response.hpp"

int Response::composeResponse(){
	statusLine = statusLine + " " + request.getStatusCode() + " " + request.getStatusText() + "\r\n";
	std::string path = request.getPath();
	const char *cPath = path.c_str();
	int fd = open(cPath, O_RDONLY);
	char buffer[2048];
	ssize_t bytesRead = 0;
	if (fd >= 0)
	{
		bytesRead = read(fd, buffer, sizeof(buffer));
		if (bytesRead > 0){
			std::string htmlPage(buffer, bytesRead);
			body = htmlPage;
		}
		else{
			std::cout << "error" << std::endl;
			return 1;
		}
		close(fd);
	}
	std::string sizeOfBody;
	std::stringstream ss;
	ss << bytesRead;
	ss >> sizeOfBody;
	contentLength = contentLength + sizeOfBody + "\r\n";
	std::string connection = "Connection: close\r\n\r\n";
	fullResponse = statusLine + contentType + contentLength + connection + body;
	std::cout << fullResponse;
	return 0;
}

std::string Response::getFullResponse(){
	return fullResponse;
}