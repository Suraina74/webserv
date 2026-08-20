#include "../inc/EventLoop.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"

std::string receiveRequest(int clientFd){
	char buffer[2048];
	ssize_t n = recv(clientFd, buffer, sizeof(buffer), 0);
	ssize_t bytesRead = n;
	std::string messageUntillHeaders(buffer, n);
	while (messageUntillHeaders.find("\r\n\r\n") == std::string::npos){
		ssize_t n = recv(clientFd, buffer, sizeof(buffer), 0);
		bytesRead = bytesRead + n;
		std::string newMessage(buffer, n);
		messageUntillHeaders = messageUntillHeaders + newMessage;
	}
	ssize_t contentLength = getContentlength(messageUntillHeaders);
	// Check on contentLength of het niet een -getal is of een heel groot getal.
	ssize_t bytesUntilHeaders = getBytesUntilHeaders(messageUntillHeaders);
	std::string fullRequest = messageUntillHeaders;
	if (contentLength){
		while (bytesRead < (bytesUntilHeaders + contentLength)){
			ssize_t n = recv(clientFd, buffer, sizeof(buffer), 0);
			bytesRead = bytesRead + n;
			std::string newMessage(buffer, n);
			fullRequest = fullRequest + newMessage;
		}
	}
	return fullRequest;
}

int	eventLoop(int *sockfd)
{
	EventLoop	poll_fds;
	// char		index_page_txt[2048];

	// char index_path[] = "www/index.html";
	memset(&poll_fds.fds, 0, sizeof(poll_fds.fds));
	poll_fds.fds.fd = *sockfd; //this is the fd to read from
	poll_fds.fds.events = POLLIN; //the events we are intereted in
	while (1)
	{
		//first you have to accept a connection
		//then we read or write data over a network socket
		poll(&poll_fds.fds, poll_fds.nfds, 100);
		// if (ready > 0 && (poll_fds.fds.revents & POLLIN))
		{
			//accept() is the function that takes an incoming connection 
			//from your listening socket and creates a new socket for that specific client.
			int new_fd = accept(*sockfd, NULL, NULL);
			if (new_fd >= 0)
			{
				std::string fullRequest = receiveRequest(new_fd);
				std::cout << fullRequest;
				Request request(fullRequest);
				request.extractElements();
				Response response(request);
				response.composeResponse();
				std::string fullResponse = response.getFullResponse();
				int lenResponse = fullResponse.length();
				const char *cFullResponse = fullResponse.c_str();
				// send sends in parts too like read.
				int n = send(new_fd, cFullResponse, lenResponse, 0);
				int totalSent = n;
				while (totalSent < lenResponse){
					n = send(new_fd, cFullResponse + totalSent, lenResponse - totalSent, 0);
					totalSent = totalSent + n;
				}
				close (new_fd);
			}
		}
	}
	return (0);
}

int	createSockAddr(int *sockfd, struct addrinfo *result)
{
	struct addrinfo info;
	struct addrinfo *ptr;

	memset(&info, 0, sizeof(info));
	info.ai_family = AF_INET;
	info.ai_socktype = SOCK_STREAM;
	//getaddrinfo sets result to a linked list that 
 	//includes as many candidate addresses as match 
	//the ai_family and ai_socktype you set value to
	if (getaddrinfo("127.0.0.1", "8080", &info, &result) != 0)
	{
		std::cout << "Getaddrinfo failed\n";
		return 1;
	}
	//this for loop runs through the linked list returned by getaddrinfo until it
 	//finds one that successfully bind with the sockfd
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		*sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (*sockfd == -1)
		{
			std::cout << "Socket failed.\n";
			continue;
		}
		//setsockopt is a flag that bypasses the default cooling down time of your 
 		//operating system when you restart your server to avoid error.
		int on = true;
		if ((setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) == -1)
		{
			perror("Setsockopt failed.\n");
			continue;
		}
		std::cout << "Connected to the host!\n";
		if (bind(*sockfd, result->ai_addr, result->ai_addrlen) == -1)
		{
			perror("Bind failed.\n");
			continue;
		}
		break ;
	}
	return (0);
}