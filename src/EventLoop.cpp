#include "../inc/EventLoop.hpp"
#include "configParser/ServerConfig.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

std::string receiveRequest(int clientFd)
{
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
	while (messageUntillHeaders.find("\r\n\r\n") == std::string::npos)
	{
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
	if (contentLength)
	{
		while (bytesRead < (headerBytes + contentLength))
		{
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

// servers
//listenFDs komt van createsock addrs
//configIndexes
int eventLoop(const vector<int> &listenFds, const vector<ServerConfig> &servers)
{
	EventLoop poll_fds;
	vector<size_t> configIndexes;

	for (size_t i = 0; i < listenFds.size(); ++i)
	{
		pollfd pfd;
		pfd.fd = listenFds[i];
		pfd.events = POLLIN;
		pfd.revents = 0;
		poll_fds.fds.push_back(pfd);
		configIndexes.push_back(i);
	}
	std::string fullRequest{};
	while (1)
	{
		pollfd client_pfd;

		size_t nfds = poll_fds.fds.size();
		int ready = poll(poll_fds.fds.data(), nfds, 100);
		if (ready == -1)
		{
			::perror("poll");
			return (1);
		}
		for (size_t i = 0; i < nfds; i++)
		{
			if (i < listenFds.size())
			{
				if (poll_fds.fds[i].revents & POLLIN)
				{
					client_pfd.fd = accept(listenFds[i], NULL, NULL);
					if (client_pfd.fd == -1)
					{
						::perror("accept");
						return (1);
					}
					client_pfd.events = POLLIN;
					client_pfd.revents = 0;
					poll_fds.fds.push_back(client_pfd);
					configIndexes.push_back(i);
					nfds++;
				}
				continue;
			}
			if ((poll_fds.fds[i].revents & POLLIN))
			{
				fullRequest = receiveRequest(poll_fds.fds[i].fd);
				if (fullRequest.empty())
				{
					close(poll_fds.fds[i].fd);
					configIndexes.erase(configIndexes.begin() + i);
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
				request.extractElements(servers[configIndexes[i]]);
				Response response(request);
				response.composeResponse();
				std::string fullResponse = response.getFullResponse();
				int lenResponse = fullResponse.length();
				const char *cFullResponse = fullResponse.c_str();
				int n = send(poll_fds.fds[i].fd, cFullResponse, lenResponse, 0);
				if (n == -1 || n == 0)
				{
					::perror("send");
					close(poll_fds.fds[i].fd);
					poll_fds.fds.erase(poll_fds.fds.begin() + i);
					configIndexes.erase(configIndexes.begin() + i);
					nfds--;
					i--;
					continue;
				}
				int totalSent = n;
				while (totalSent < lenResponse)
				{
					n = send(poll_fds.fds[i].fd, cFullResponse + totalSent, lenResponse - totalSent, 0);
					if (n == -1 || n == 0)
					{
						::perror("send");
						close(poll_fds.fds[i].fd);
						poll_fds.fds.erase(poll_fds.fds.begin() + i);
						configIndexes.erase(configIndexes.begin() + i);
						nfds--;
						i--;
						continue;
					}
					totalSent = totalSent + n;
				}
				close(poll_fds.fds[i].fd);
				poll_fds.fds.erase(poll_fds.fds.begin() + i);
				configIndexes.erase(configIndexes.begin() + i);
				nfds--;
				i--;
			}
		}
	}
	return (0);
}

vector<int> createSockAddr(struct addrinfo *result, const vector<ServerConfig> &server)
{
	vector<int>	listenFds;
	for (size_t i = 0; i < server.size(); ++i)
	{
		struct addrinfo info;
		struct addrinfo *ptr;

		memset(&info, 0, sizeof(info));
		info.ai_family = AF_INET;
		info.ai_socktype = SOCK_STREAM;
		string port = to_string(server[i].getPort());
		if (getaddrinfo(server[i].getHost().c_str(), port.c_str(), &info, &result) != 0)
		{
			::perror("getaddrinfo");
			break;
		}
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
		{
			int listenFd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (listenFd == -1)
			{
				::perror("socket");
				continue;
			}
			if (fcntl(listenFd, F_SETFL, O_NONBLOCK) == -1)
			{
				::perror("fcntl");
				continue;
			}
			int on = true;
			if ((setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) == -1)
			{
				::perror("setsockopt");
				continue;
			}
			if (bind(listenFd, ptr->ai_addr, ptr->ai_addrlen) == -1)
			{
				::perror("bind");
				continue;
			}
			listenFds.push_back(listenFd);
			break;
		}
	}
	return (listenFds);
}

int server(const vector<ServerConfig> &servers)
{
	struct addrinfo *result = nullptr;
	vector<int> listenFds = createSockAddr(result, servers);

	if (listenFds.size() != servers.size())
		return (1);
	for (size_t i = 0; i < listenFds.size(); i++)
	{
		if (listen(listenFds[i], 10) != 0)
		{
			perror("listen");
			return (1);
		}
	}
	return (eventLoop(listenFds, servers));
}