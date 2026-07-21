#include "../inc/Server.hpp"
#include "../inc/EventLoop.hpp"

int	main(int ac, char **av)
{
	(void)av;
	if (ac != 2)
	{
		std::cout << "usage: ./webserv config" << std::endl;
		return (1);
	}

	struct addrinfo *result;
	struct addrinfo info;
	memset(&info, 0, sizeof(info));
	info.ai_family = AF_INET; // IPV4
	// info.ai_flags = AI_PASSIVE; // Choose IP for me
	info.ai_socktype = SOCK_STREAM; // TCP communication style
	
	// Build me a list of usable socket addresses:
	// Not all can be reached. Find one that is usable.
	if (getaddrinfo("127.0.0.1", "8080", &info, &result) != 0){
		std::cout << "getaddrinfo failed\n";
		return 1;
	}

	// ai_protocol gaat op basis van socktype.
	// What kind of socket you want:
	struct addrinfo *ptr;
	int sockfd = 0;

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (sockfd == -1){
			std::cout << "socket failure\n";
			// freeaddrinfo(result);
			return 1;
		}
		int on = true;
		if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) == -1)
		{
			std::cout << "socket option failure\n";
			return 1;
		}
		// Binding the socket to the host (server side).
		// Server says: I will listen on this port.
		std::cout << "connected to the host!\n";
		if (bind(sockfd, result->ai_addr, result->ai_addrlen) == -1){
			std::cout << "bind failure\n";
			// freeaddrinfo(result);
			return 1;
		}
		break ;
	}
	
	freeaddrinfo(result);
	if (listen(sockfd, 10) != 0){
		std::cout << "listen failure\n";
		return 1;
	}
	// Accept incoming connections.

	// struct pollfd mypoll;

	EventLoop poll_fds;

	memset(poll_fds.fds, 0, sizeof(poll_fds.fds));
	// mypoll.fd = 0;
	// mypoll.events = POLLIN;

	while (1)
	{
		//first you have to accept a connection
		struct sockaddr_storage client_address;
		socklen_t address_size;
		address_size = sizeof(client_address);
		std::cout << "LOOP\n";
		int new_fd = accept(sockfd, (struct sockaddr *)&client_address, &address_size);
		if (new_fd == -1){
			std::cout << "accept failure\n";
			continue;
		}

		//then we read or write data over a network socket
		poll(poll_fds.fds, 1, 100);
		char index_page_txt[2048];	
		// if (mypoll.revents & POLLIN)
		// {
			int open_index = open("www/index.html", O_RDONLY);
			ssize_t n = read(open_index, index_page_txt, 2048);
			index_page_txt[n] = '\0';
			std::cout << index_page_txt << std::endl;
		// }
		// else if (mypoll.revents & POLLOUT) {
			send(new_fd, index_page_txt, 2048, 0);
		// }
		
		// show index.html
		// char buffer[1024];
		// ssize_t n = recv(new_fd, buffer, sizeof(buffer), 0);
		// buffer[n] = '\0';
		// std::cout << buffer;
	

	}

	// Alleen result free-en met freeaddrinfo(res), info is alleen input.


	return (0);
}