#include "../inc/EventLoop.hpp"

int	eventLoop(int *sockfd)
{
	EventLoop	poll_fds;
	char		index_page_txt[2048];

	char config_path[] = "www/index.html";

	memset(&poll_fds.fds, 0, sizeof(poll_fds.fds));
	poll_fds.fds.fd = *sockfd;
	poll_fds.fds.events = POLLIN;
	while (1)
	{
		int ready = poll(&poll_fds.fds, 1, 100);
		if (ready > 0 && (poll_fds.fds.revents & POLLIN))
		{
			int new_fd = accept(*sockfd, NULL, NULL);
			if (new_fd >= 0)
			{
				int open_index = open(config_path, O_RDONLY);
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
	if (getaddrinfo("127.0.0.1", "8080", &info, &result) != 0)
	{
		std::cout << "getaddrinfo failed\n";
		return (1);
	}
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		*sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (*sockfd == -1)
		{
			std::cout << "socket failure\n";
			return (1);
		}
		int on = true;
		if ((setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) == -1)
		{
			perror("setsockopt");
			return (1);
		}
		std::cout << "connected to the host!\n";
		if (bind(*sockfd, result->ai_addr, result->ai_addrlen) == -1)
		{
			perror("bind");
			return (1);
		}
		break ;
	}
	return (0);
}
