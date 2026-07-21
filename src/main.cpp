#include "../inc/Server.hpp"
#include "../inc/EventLoop.hpp"

int	eventLoop(int *sockfd)
{
	EventLoop poll_fds;

	memset(&poll_fds.fds, 0, sizeof(poll_fds.fds));
	while (1)
	{
		struct sockaddr_storage	client_address;
		socklen_t				address_size;
	
		address_size = sizeof(client_address);
		int new_fd = accept(*sockfd, (struct sockaddr *)&client_address, &address_size);
		if (new_fd == -1)
		{
			perror("accept");
			continue;
		}
		poll(&poll_fds.fds, 1, 100);
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

	}
}

int	main(int ac, char **av)
{
	struct addrinfo *result;
	struct addrinfo info;
	struct addrinfo *ptr;
	int sockfd = 0;

	(void)av;
	if (ac != 2)
	{
		std::cout << "usage: ./webserv config" << std::endl;
		return (1);
	}
	memset(&info, 0, sizeof(info));
	info.ai_family = AF_INET;
	info.ai_socktype = SOCK_STREAM;
	if (getaddrinfo("127.0.0.1", "8080", &info, &result) != 0){
		std::cout << "getaddrinfo failed\n";
		return 1;
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (sockfd == -1){
			std::cout << "socket failure\n";
			return 1;
		}
		int on = true;
		if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) == -1)
		{
			std::cout << "socket option failure\n";
			return 1;
		}
		std::cout << "connected to the host!\n";
		if (bind(sockfd, result->ai_addr, result->ai_addrlen) == -1){
			std::cout << "bind failure\n";
			return 1;
		}
		break ;
	}
	freeaddrinfo(result);
	if (listen(sockfd, 10) != 0){
		std::cout << "listen failure\n";
		return 1;
	}
	eventLoop(&sockfd);
	return (0);
}