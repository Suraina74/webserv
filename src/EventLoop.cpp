#include "../inc/EventLoop.hpp"
#include "configParser/ServerConfig.hpp"
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int nfds = 1;
int eventLoop(int *listen_fd, const ServerConfig &servers)
{
	char index_page_txt[2048];
	string path = servers.getRoot() + '/' + servers.getIndex();
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
			pollfd client_pfd;
			client_pfd.fd = accept(*listen_fd, NULL, NULL);
			if (client_pfd.fd == -1)
			{
				::perror("accept");
				return (1);
			}
			client_pfd.events = POLLIN;
			client_pfd.revents = 0;
			poll_fds.fds.push_back(client_pfd);
			nfds++;
		}
		for (int i = 1; i < nfds; i++)
		{
			if ((poll_fds.fds[i].revents & POLLIN))
			{
				char buffer[2048];
				ssize_t received = recv(poll_fds.fds[i].fd, buffer, sizeof(buffer), 0);
				if (received <= 0)
				{
					if (received == -1)
						::perror("recv");
					close(poll_fds.fds[i].fd);
					poll_fds.fds.erase(poll_fds.fds.begin() + i);
					nfds--;
					i--;
					continue;
				}
				poll_fds.fds[i].events = POLLOUT;
			}
			if (poll_fds.fds[i].revents & POLLOUT)
			{
				int open_index = open(path.c_str(), O_RDONLY);
				if (open_index >= 0)
				{
					ssize_t n = read(open_index, index_page_txt, sizeof(index_page_txt) - 1);
					if (n > 0)
					{
						index_page_txt[n] = '\0';
						if (send(poll_fds.fds[i].fd, index_page_txt, n, MSG_NOSIGNAL) == -1)
						{
							::perror("send");
							close(poll_fds.fds[i].fd);
							poll_fds.fds.erase(poll_fds.fds.begin() + i);
							nfds--;
							i--;
							continue;
						}
						poll_fds.fds[i].events = POLLIN;
					}
					close(open_index);
				}
			}
		}
	}
	return (0);
}

int createSockAddr(int *listen_fd, struct addrinfo *result, const ServerConfig &servers)
{
	struct addrinfo info;
	struct addrinfo *ptr;

	memset(&info, 0, sizeof(info));
	info.ai_family = AF_INET;
	info.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(servers.getHost().c_str(), "8080", &info, &result) != 0)
	{
		::perror("getaddrinfo");
		return (1);
	}
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		*listen_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (*listen_fd == -1)
		{
			::perror("socket");
			continue;
		}
		if (fcntl(*listen_fd, F_SETFL, O_NONBLOCK) == -1)
		{
			::perror("fcntl");
			continue;
		}
		int on = true;
		if ((setsockopt(*listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) == -1)
		{
			::perror("setsockopt");
			continue;
		}
		if (bind(*listen_fd, result->ai_addr, result->ai_addrlen) == -1)
		{
			::perror("bind");
			continue;
		}
		break;
	}
	return (0);
}