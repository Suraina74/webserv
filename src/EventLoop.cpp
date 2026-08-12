#include "../inc/EventLoop.hpp"

int nfds = 1;

int eventLoop(int *listen_fd)
{
	char index_page_txt[2048];
	char index_path[] = "www/index.html";

	EventLoop poll_fds;
	pollfd pfd;

	pfd.fd = *listen_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	poll_fds.fds.push_back(pfd);
	while (1)
	{
		int ready = poll(poll_fds.fds.data(), nfds, 100);
		if (ready == -1)
		{
			::perror("poll");
			return (1);
		}
		if (poll_fds.fds[0].revents & POLLIN)
		{
			poll_fds.fds[nfds].fd = accept(*listen_fd, NULL, NULL);
			if (poll_fds.fds[nfds].fd == -1)
			{
				::perror("accept");
				return (1);
			}
			poll_fds.fds[nfds].events = POLLIN;
			nfds++;
		}
		for (int i = 1; i < nfds; i++)
		{
			if ((poll_fds.fds[i].revents & POLLIN))
			{
				char buffer[2048];
				recv(poll_fds.fds[i].fd, buffer, sizeof(buffer), 0);
				// cout << buffer << endl;
			}
			poll_fds.fds[i].events = POLLOUT;
			if (poll_fds.fds[i].revents & POLLOUT)
			{
					int open_index = open(index_path, O_RDONLY);
					if (open_index >= 0)
					{
						ssize_t n = read(open_index, index_page_txt, sizeof(index_page_txt) - 1);
						if (n > 0)
						{
							index_page_txt[n] = '\0';
							send(poll_fds.fds[i].fd, index_page_txt, n, 0);
						}
						close(open_index);
					}
					poll_fds.fds[i].events = POLLIN;
			}
		}
	}
	return (0);
}

int createSockAddr(int *listen_fd, struct addrinfo *result)
{
	struct addrinfo info;
	struct addrinfo *ptr;

	memset(&info, 0, sizeof(info));
	info.ai_family = AF_INET;
	info.ai_socktype = SOCK_STREAM;
	if (getaddrinfo("127.0.0.1", "8080", &info, &result) != 0)
	{
		std::cout << "Getaddrinfo failed\n";
		return 1;
	}
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		*listen_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (*listen_fd == -1)
		{
			std::cout << "Socket failed.\n";
			continue;
		}
		fcntl(*listen_fd, F_SETFL, O_NONBLOCK);
		int on = true;
		if ((setsockopt(*listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) == -1)
		{
			::perror("setsockopt failed.\n");
			continue;
		}
		std::cout << "Connected to the host!\n";
		if (bind(*listen_fd, result->ai_addr, result->ai_addrlen) == -1)
		{
			::perror("Bind failed.\n");
			continue;
		}
		break;
	}
	return (0);
}