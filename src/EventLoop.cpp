#include "../inc/EventLoop.hpp"
#include "../inc/Request.hpp"

int	eventLoop(int *sockfd)
{
	EventLoop	poll_fds;
	char		index_page_txt[2048];

	char index_path[] = "www/uploads.html";
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
				int open_index = open(index_path, O_RDONLY);
				if (open_index >= 0)
				{
					ssize_t n = read(open_index, index_page_txt, sizeof(index_page_txt) - 1);
					if (n > 0)
					{
						index_page_txt[n] = '\0';
						send(new_fd, index_page_txt, n, 0);
					}
					close(open_index);
				}
			}
			char buffer[2048];
			ssize_t n = recv(new_fd, buffer, sizeof(buffer), 0);
			ssize_t bytesRead = n;
			std::string partRead(buffer, n); // "Create a new std::string containing the first n characters/bytes starting at the first byte of buffer."
			while (partRead.find("\r\n\r\n") == std::string::npos){
				ssize_t n = recv(new_fd, buffer, sizeof(buffer), 0);
				bytesRead = bytesRead + n;
				std::string newPart(buffer, n);
				partRead = partRead + newPart;
			}
			// Until now only read until \r\n\r\n or a bit after.
			Request request(partRead);
			request.extractElements();
			ssize_t bodyLength = request.getContentLength();
			

			// if (bodyLength){
			// 	while (bytesRead < (request.getBytesUntilHeaders() + bodyLength)){
			// 		ssize_t n = recv(new_fd, buffer, sizeof(buffer), 0);
			// 		bytesRead = bytesRead + n;
			// 		std::string newPart(buffer, n);
			// 		partRead = partRead + newPart;
			// 	}
			// }
			// std::cout << partRead << std::endl;
			// std::cout << partRead.size() << std::endl;
			std::cout << bytesRead << std::endl;
			std::cout << bodyLength << std::endl;
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