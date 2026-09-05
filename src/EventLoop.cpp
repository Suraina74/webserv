#include "configParser/ServerConfig.hpp"
#include "../inc/EventLoop.hpp"
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

int eventLoop(const vector<int> &listenFds, const vector<ServerConfig> &servers)
{
	EventLoop poll_fds;

	//setup listining sockets
	for (size_t i = 0; i < listenFds.size(); ++i)
	{
		pollfd listen_socket;

		listen_socket.fd = listenFds[i];
		listen_socket.events = POLLIN;
		listen_socket.revents = 0;

		//why create a general list of fds when you can keep them seprate? just cleanup on both.
		poll_fds.fds.push_back(listen_socket);
		poll_fds.configIndexes.push_back(i);
	}
	std::string fullRequest{};
	while (1)
	{
		pollfd client_pfd;

		size_t nfds = poll_fds.fds.size();
		int ready = poll(poll_fds.fds.data(), nfds, TIMEOUT);
		if (ready == -1)
		{
			::perror("poll");
			return (1);
		}
		for (size_t i = 0; i < nfds; i++)
		{
			//begining of list is listining fds but I could instead use a list of listen fds todo this part..
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
					poll_fds.configIndexes.push_back(i);
					nfds++;
				}
				continue;
			}
			//client fds
			if ((poll_fds.fds[i].revents & POLLIN))
			{
				fullRequest = receiveRequest(poll_fds.fds[i].fd);
				if (fullRequest.empty())
				{
					close(poll_fds.fds[i].fd);
					poll_fds.configIndexes.erase(poll_fds.configIndexes.begin() + i);
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
				request.extractElements(servers[poll_fds.configIndexes[i]]);
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
					poll_fds.configIndexes.erase(poll_fds.configIndexes.begin() + i);
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
						poll_fds.configIndexes.erase(poll_fds.configIndexes.begin() + i);
						nfds--;
						i--;
						continue;
					}
					totalSent = totalSent + n;
				}
				close(poll_fds.fds[i].fd);
				poll_fds.fds.erase(poll_fds.fds.begin() + i);
				poll_fds.configIndexes.erase(poll_fds.configIndexes.begin() + i);
				nfds--;
				i--;
			}
		}
	}
	return (0);
}

vector<int> createSockAddr(struct addrinfo *result, const vector<ServerConfig> &server)
{
	vector<int>	listenFdsList;

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
			listenFdsList.push_back(listenFd);
			break;
		}
	}
	return (listenFdsList);
}

int server(const vector<ServerConfig> &servers)
{
	EventLoop eloop;

	eloop.result = nullptr;
	eloop.listenFds = createSockAddr(eloop.result, servers);
	for (size_t i = 0; i < eloop.listenFds.size(); i++)
	{
		if (listen(eloop.listenFds[i], 10) != 0)
		{
			::perror("listen");
			return (1);
		}
	}
	return (eventLoop(eloop.listenFds, servers));
}

//todos
// Socket cleanup, error events, partial sends, and server-to-config mapping need work.