#include "../inc/EventLoop.hpp"
#include "configParser/ServerConfig.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

std::string receiveRequest(int clientFd){
	char buffer[2048];
	ssize_t n = recv(clientFd, buffer, sizeof(buffer), 0);
	if (n == -1)
	{
		::perror("recv");
		return ("");
	}
	else if (n == 0)
	{
		return ("");
	}
	ssize_t bytesRead = n;
	std::string messageUntillHeaders(buffer, n);
	while (messageUntillHeaders.find("\r\n\r\n") == std::string::npos){
		ssize_t n = recv(clientFd, buffer, sizeof(buffer), 0);
		if (n == -1)
		{
			::perror("recv");
			return ("");
		}
		else if (n == 0)
		{
			return ("");
		}
		bytesRead = bytesRead + n;
		std::string newMessage(buffer, n);
		messageUntillHeaders = messageUntillHeaders + newMessage;
	}
	ssize_t contentLength = getContentlength(messageUntillHeaders);
	// Check on contentLength of het niet een -getal is of een heel groot getal.
	ssize_t headerBytes = getBytesUntilHeaders(messageUntillHeaders);
	std::string fullRequest = messageUntillHeaders;
	if (contentLength){
		while (bytesRead < (headerBytes + contentLength)){
			ssize_t n = recv(clientFd, buffer, sizeof(buffer), 0);
			if (n == -1)
			{
				::perror("recv");
				return ("");
			}
			else if (n == 0)
			{
				return ("");
			}
			bytesRead = bytesRead + n;
			std::string newMessage(buffer, n);
			fullRequest = fullRequest + newMessage;
		}
	}
	return fullRequest;
}

int eventLoop(int *listen_fd, const ServerConfig &servers)
{
	// string path = servers.getRoot() + '/' + servers.getIndex();
	(void)servers;
	EventLoop poll_fds;
	pollfd pfd;

	pfd.fd = *listen_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	poll_fds.fds.push_back(pfd);
	std::string fullRequest{};
	while (1)
	{
		pollfd client_pfd;

		int nfds = poll_fds.fds.size();
		int ready = poll(poll_fds.fds.data(), nfds, 100);
		if (ready == -1)
		{
			::perror("poll");
			return (1);
		}
		if (poll_fds.fds[0].revents & POLLIN)
		{
			client_pfd.fd = accept(*listen_fd, NULL, NULL);
			if (client_pfd.fd == -1)
			{
				::perror("accept");
				return (1);
			}
			if (fcntl(*listen_fd, F_SETFL, O_NONBLOCK) == -1)
			{
				::perror("fcntl");
				continue;
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
				fullRequest = receiveRequest(poll_fds.fds[i].fd);
				if (fullRequest.empty()){
					close(poll_fds.fds[i].fd);
					poll_fds.fds.erase(poll_fds.fds.begin() + i);
					nfds--;
					i--;
					continue;
				}
				poll_fds.fds[i].events = POLLOUT;
			}
			else if (poll_fds.fds[i].revents & POLLOUT)
			{
				Request request(fullRequest);
				request.extractElements();
				Response response(request);
				response.composeResponse();
				std::string fullResponse = response.getFullResponse();
				int lenResponse = fullResponse.length();
				const char *cFullResponse = fullResponse.c_str();
				int n = send(poll_fds.fds[i].fd, cFullResponse, lenResponse, 0);
				if (n == -1 || n == 0){
					::perror("send");
					close(poll_fds.fds[i].fd);
					poll_fds.fds.erase(poll_fds.fds.begin() + i);
					nfds--;
					i--;
					continue;
				}
				int totalSent = n;
				while (totalSent < lenResponse)
				{
					n = send(poll_fds.fds[i].fd, cFullResponse + totalSent, lenResponse - totalSent, 0);
					if (n == -1 || n == 0){
						::perror("send");
						close(poll_fds.fds[i].fd);
						poll_fds.fds.erase(poll_fds.fds.begin() + i);
						nfds--;
						i--;
						continue;
					}
					totalSent = totalSent + n;
				}
				close(poll_fds.fds[i].fd);
				poll_fds.fds.erase(poll_fds.fds.begin() + i);
				nfds--;
				i--;
			}
		}
	}
	return (0);
}

int createSockAddr(int *listen_fd, struct addrinfo *result, const ServerConfig &servers)
{
	struct addrinfo info;
	struct addrinfo *ptr;

	int port_int = servers.getPort();
	string p = to_string(port_int);
	const char *port = p.c_str();

	memset(&info, 0, sizeof(info));
	info.ai_family = AF_INET;
	info.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(servers.getHost().c_str(), port, &info, &result) != 0)
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

int	server(const ServerConfig &servers)
{
	struct addrinfo *result = nullptr;
	int listen_fd = 0;

	if (createSockAddr(&listen_fd, result, servers) != 0)
		return (1);
	freeaddrinfo(result);
	if (listen(listen_fd, 10) != 0)
	{
		perror("listen");
		return (1);
	}
	if (eventLoop(&listen_fd, servers))
		return (1);
	return (0);
}