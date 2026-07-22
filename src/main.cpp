#include "../inc/Server.hpp"

int	main(int ac, char **av)
{
	(void)av;
	if (ac != 2)
	{
		cout << "usage: ./webserv <filename>.config" << std::endl;
		return (1);
	}

	struct addrinfo *result = NULL;//initialize it to NULL for safety practice
	struct addrinfo info;
	memset(&info, 0, sizeof(info));
	info.ai_family = AF_INET; // IPV4
	// info.ai_flags = AI_PASSIVE; // Choose IP for me
	info.ai_socktype = SOCK_STREAM; // TCP communication style
	
	// Build me a list of usable socket addresses:
	// Not all can be reached. Find one that is usable.
	if (getaddrinfo("127.0.0.1", "8080", &info, &result) != 0)
	{
		//getaddrinfo sets result to a linked list that 
		//includes as many candidate addresses as match the ai_family and ai_socktype you set value to
		cout << "getaddrinfo failed\n";
		return 1;
	}

	// ai_protocol gaat op basis van socktype.
	// What kind of socket you want:
	struct addrinfo *ptr;
	int sockfd = 0;

	//this for loop runs through the linked list returned by getaddrinfo until it
	//finds one that successfully bind with the sockfd
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (sockfd == -1)
		{
			cout << "socket failure\n";
			// freeaddrinfo(result);
			continue;
		}
		//setsockopt is a flag that bypasses the default cooling down time of your 
		//operating system when you restart your server to avoid error.
		int on = true;
		if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) == -1)
		{
			cout << "socket option failure\n";
			continue;
		}
		// Binding the socket to the host (server side).
		// Server says: I will listen on this port.
		cout << "connected to the host!\n";
		if (bind(sockfd, ptr->ai_addr, ptr->ai_addrlen) == -1)
		{
			cout << "bind failure\n";
			close(sockfd);
			// freeaddrinfo(result);
			continue;
		}
		cout << "connected to the host!\n";
		break ;
	}
	
	freeaddrinfo(result);
	//128 in listen(sockfd, 128) is referred to as backlog number
	//it is the maximum number of client connections that can be fully 
	//connected (finished the TCP handshake) but not yet accept()ed by your program, 
	//sitting and waiting in a kernel-managed queue. SOMAXCONN returns the actual max queue
	//the os can handle, however 128 is more of a universally acknowledged reasonable value.
	if (listen(sockfd, 128) != 0){
		cout << "listen failure\n";
		return 1;
	}

	struct pollfd mypoll;

	memset(&mypoll, 0, sizeof(mypoll));
	mypoll.fd = 0;
	mypoll.events = POLLIN;
	
	while (1)
	{
		//first you have to accept a connection
		struct sockaddr_storage client_address;
		socklen_t address_size;
		address_size = sizeof(client_address);
		cout << "LOOP\n";
		int new_fd = accept(sockfd, (struct sockaddr *)&client_address, &address_size);
		if (new_fd == -1){
			cout << "accept failure\n";
			continue;
		}

		//then we read or write data over a network socket
		poll(&mypoll, 1, 100);
		char index_page_txt[2048];	
		// if (mypoll.revents & POLLIN)
		// {
			int open_index = open("www/index.html", O_RDONLY);
			ssize_t n = read(open_index, index_page_txt, 2048);
			index_page_txt[n] = '\0';
			cout << index_page_txt << std::endl;
		// }
		// else if (mypoll.revents & POLLOUT) {
			send(new_fd, index_page_txt, 2048, 0);
		// }
		
		// show index.html
		// char buffer[1024];
		// ssize_t n = recv(new_fd, buffer, sizeof(buffer), 0);
		// buffer[n] = '\0';
		// cout << buffer;
	

	}

	// Alleen result free-en met freeaddrinfo(res), info is alleen input.


	return (0);
}