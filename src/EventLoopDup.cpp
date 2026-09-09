#include "../inc/EventLoop.hpp"
#include "configParser/ServerConfig.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

string receiveRequest(int clientFd){
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
	string messageUntillHeaders(buffer, n);
	while (messageUntillHeaders.find("\r\n\r\n") == string::npos){
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
		string newMessage(buffer, n);
		messageUntillHeaders = messageUntillHeaders + newMessage;
	}
	ssize_t contentLength = getContentlength(messageUntillHeaders);
	// Check on contentLength of het niet een -getal is of een heel groot getal.
	ssize_t headerBytes = getBytesUntilHeaders(messageUntillHeaders);
	string fullRequest = messageUntillHeaders;
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
			string newMessage(buffer, n);
			fullRequest = fullRequest + newMessage;
		}
	}
	return fullRequest;
}

int eventLoop(int *listen_fd, const ServerConfig &currentServer)
{
	// string path = currentServer.getRoot() + '/' + currentServer.getIndex();
	(void)currentServer;
	EventLoop poll_fds;//total amount of fds
	pollfd pfd;

	pfd.fd = *listen_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	poll_fds.fds.push_back(pfd);
	string fullRequest{};
	while (1)
	{
		pollfd client_pfd;

		int nfds = poll_fds.fds.size();
		int ready = poll(poll_fds.fds.data(), nfds, 100);//Q: what does poll_fds.fds.data() do?
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
			client_pfd.revents = 0;//Q: is it always necessary to set revents?
			poll_fds.fds.push_back(client_pfd);
			nfds++;//Q: if nfds = poll_fds.fds.size(), then nfds++ is unnecessary here
		}
		for (int i = 1; i < nfds; i++)
		{
			if ((poll_fds.fds[i].revents & POLLIN))
			{
				fullRequest = receiveRequest(poll_fds.fds[i].fd);
				if (fullRequest.empty())
                {
					close(poll_fds.fds[i].fd);
					poll_fds.fds.erase(poll_fds.fds.begin() + i);//Q: What does poll_fds.fds.erase(poll_fds.fds.begin() + i) erase here
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
				string fullResponse = response.getFullResponse();
				int lenResponse = fullResponse.length();
				const char *cFullResponse = fullResponse.c_str();//Q: why make it const char* when you can use &fullResponse for send()
				int n = send(poll_fds.fds[i].fd, &fullResponse, lenResponse, 0);
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

int createSockAddr(int *listen_fd, struct addrinfo **result, const ServerConfig &currentServer)
{
	struct addrinfo info;
	struct addrinfo *ptr;

	memset(&info, 0, sizeof(info));
	info.ai_family = AF_INET;//AF_INET sets address to IPv4 type
	info.ai_socktype = SOCK_STREAM;//SOCK_STREAM sets socket to TCP-style stream socket
    /* getaddrinfo() converts a hostname/address and service (port) into socket-ready address information.
    Its 1st parameter specifies the host/address, the 2nd specifies the port or service, and 
    the 3rd describes requirements such as IPv4/IPv6 and TCP. Finally getaddrinfo() returns a linked list of 
    possible addrinfo results, with the 4th parameter (&result) pointing to the first one.
	*/
    if (getaddrinfo(currentServer.getHost().c_str(), currentServer.getPort().c_str(), &info, result) != 0)
	{
		::perror("getaddrinfo failure.");
		return (1);
	}
	for (ptr = *result; ptr != NULL; ptr = ptr->ai_next)
	{
		*listen_fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol); //socket() creates a socket and returns a fd
        if (*listen_fd == -1)
		{
			::perror("socket setup failure.");
			continue;
		}
		if (fcntl(*listen_fd, F_SETFL, O_NONBLOCK) == -1) 
		{
            //fcntl() makes the socket fd non-blocking
            //F_SETFL tells fcntl() what operation you want. It means set the file-status flags of this file descriptor.
            //O_NONBLOCK means: Make operations on this descriptor non-blocking. 
            //It is closely related to why your project can use an event-driven poll() architecture.
			::perror("fcntl failure.");
			continue;
		}
		int on = 1;
		if ((setsockopt(*listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) == -1)
		{
            //setsockopt() enters socket addrinfo and configure socket behavior
            //the value of one decides if SO_REUSEADDR is enabled or disabled
            //on needs to be passed by reference because setsockopt requires cons void*
			::perror("setsockopt failure.");
			continue;
		}
		if (::bind(*listen_fd, ptr->ai_addr, ptr->ai_addrlen) == -1)
		{
            //bind() give socket an IP + port
			::perror("bind failure.");
			continue;
		}
		break;
	}
	return (0);
}

int	server(const ServerConfig &currentServer)
{
	struct addrinfo *result = nullptr;
	int listen_fd = 0;

	if (createSockAddr(&listen_fd, &result, currentServer) != 0)
		return (1);
	freeaddrinfo(result);
	if (listen(listen_fd, 10) != 0)
	{
        //turn that bound socket into a listening socket and ready to accept connection
		perror("listen failure");
		return (1);
	}
	if (eventLoop(&listen_fd, currentServer) != 0)
		return (1);
	return (0);
}