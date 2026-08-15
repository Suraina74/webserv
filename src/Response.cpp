#include "../inc/Response.hpp"

int Response::composeResponse(){
	statusLine = statusLine + " " + request.getStatusCode() + " " + request.getStatusText() + "\r\n";
	std::cout << "statusline is: " << statusLine.length() << " characters" << std::endl;
	std::string path = request.getPath();
	const char *cPath = path.c_str();
	int fd = open(cPath, O_RDONLY);
	char htmlPage[2048];
	ssize_t bytesRead = 0;
	if (fd >= 0)
	{
		bytesRead = read(fd, htmlPage, sizeof(htmlPage) - 1);
		if (bytesRead > 0){
			htmlPage[bytesRead] = '\0';
		}
		else{
			std::cout << "error" << std::endl;
			return 1;
		}
		close(fd);
	}
	body = htmlPage;
	std::cout << "body is: " << body.length() << " characters" << std::endl;
	std::string sizeOfBody;
	std::stringstream ss;
	ss << bytesRead;
	ss >> sizeOfBody;
	contentLength = contentLength + sizeOfBody + "\r\n\r\n";
	std::cout << "contentlength is: " << contentLength.length() << " characters" << std::endl;
	std::cout << "contenttype is: " << contentType.length() << " characters" << std::endl;
	fullResponse = statusLine + contentType + contentLength + body;
	return 0;
}

std::string Response::getFullResponse(){
	return fullResponse;
}