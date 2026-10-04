#include "configParser/ServerConfig.hpp"
#include "../inc/ServerInfo.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
#include "../inc/Client.hpp"
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int receiveRequest(int clientFd, Request &request)
{
	char buffer[2048];
	ssize_t n = recv(clientFd, buffer, sizeof(buffer), 0);
	if (n == -1)
	{
		::perror("recv");
		return -1;
	}
	else if (n == 0)
	{
		return 0;
	}
	request.setBytesRead(request.getBytesRead() + n);
	std::string part(buffer, n);
	request.setRequest(request.getFullRequest() + part);
	if (request.getFullRequest().find("\r\n\r\n") != std::string::npos && request.getHeaderBytes() == 0)
	{
		if ((request.parseUntilHeaders(request.getFullRequest())) == false)
		{
			return 2;
		}
		request.setHeaderBytes(request.getRequestTillHeaders().size());
	}
	if (request.getChunked() == true && request.getFullRequest().find("0\r\n\r\n") != std::string::npos){
		return 2;
	}
	else if (request.getBytesRead() == request.getHeaderBytes() + request.getContentLength()){
		return 2;
	}
	return 1;
}

int sendResponse(int clientFd, Response &response)
{
	response.composeResponse();
	std::string fullResponse = response.getFullResponse();
	response.setLenResponse(fullResponse.length());
	response.setCString(fullResponse.c_str());
	if (response.getBytesSent() < response.getLenResponse())
	{
		int n = send(clientFd, response.getCFullResponse() + response.getBytesSent(), response.getLenResponse() - response.getBytesSent(), 0);
		if (n == -1)
		{
			::perror("send");
			return -1;
		}
		else if (n == 0)
		{
			return 0;
		}
		response.setBytesSent(response.getBytesSent() + n);
	}
	if (response.getBytesSent() == response.getLenResponse())
	{
		return 2;
	}
	return 1;
}

int eventLoop(const vector<pollfd> &fds, const vector<ServerConfig> &servers)
{

	ServerInfo serverData;
	Request request;
	Response response;
	vector<Client> clients;
	// setup listining sockets
	for (size_t i = 0; i < fds.size(); ++i)
	{
		pollfd listen_socket;

		listen_socket.fd = fds[i].fd;
		listen_socket.events = POLLIN;
		listen_socket.revents = 0;
		serverData.getFd().push_back(listen_socket);
	}
	while (1)
	{
		pollfd client_pfd;
		size_t nfds = serverData.getFd().size();
		int ready = poll(serverData.getFd().data(), nfds, TIMEOUT);

		if (ready == -1)
		{
			::perror("poll");
			return (1);
		}
		for (size_t i = 0; i < nfds; i++)
		{
			// i keeps index of listening sock
			if (i < fds.size())
			{
				if (serverData.getFd()[i].revents & POLLIN)
				{
					client_pfd.fd = accept(fds[i].fd, NULL, NULL);
					if (client_pfd.fd == -1)
					{
						::perror("accept");
						return (1);
					}
					client_pfd.events = POLLIN;
					client_pfd.revents = 0;
					serverData.getFd().push_back(client_pfd);
					Client client(client_pfd.fd, &servers[0], request, response);
					clients.push_back(client);
					nfds++;
				}
				continue;
			}
			// client fds
			if ((serverData.getFd()[i].revents & POLLIN))
			{
				int returnValue = receiveRequest(serverData.getFd()[i].fd, clients[i - fds.size()].getRequest());
				if (returnValue == -1 || returnValue == 0)
				{
					clients.erase(clients.begin() + i - fds.size());
					close(serverData.getFd()[i].fd);
					serverData.getFd().erase(serverData.getFd().begin() + i);
					nfds--;
					i--;
					continue;
				}
				else if (returnValue == 2)
				{
					clients[i - fds.size()].getRequest().parseBody();
					clients[i - fds.size()].getRequest().postAndDelete();
					//  check request against config file. To see what server (check host header) applies and what location applies.
					serverData.getFd()[i].events = POLLOUT;
				}
			}
			else if (serverData.getFd()[i].revents & POLLOUT)
			{
				clients[i - fds.size()].getResponse().setRequest(clients[i - fds.size()].getRequest());
				int returnValue = sendResponse(serverData.getFd()[i].fd, clients[i - fds.size()].getResponse());
				if (returnValue == 0 || returnValue == -1)
				{
					clients.erase(clients.begin() + i - fds.size());
					close(serverData.getFd()[i].fd);
					serverData.getFd().erase(serverData.getFd().begin() + i);
					nfds--;
					i--;
					continue;
				}
				else if (returnValue == 2)
				{
					clients.erase(clients.begin() + i - fds.size());
					close(serverData.getFd()[i].fd);
					serverData.getFd().erase(serverData.getFd().begin() + i);
					nfds--;
					i--;
				}
			}
		}
	}
	return (0);
}

//FIX: moved struct addrinfo *result; from ServerInfo class to local variable inside createSockAddr()
//because it is just a assisting variable not needed elsewhere.
vector<pollfd> createSockAddr(const vector<ServerConfig> &server)
{
	//suggestion:change para name server to serverList for clarity
	vector<pollfd>	listenFdsList;
	struct addrinfo *result;
	//Outter for loop scans through serverList
	for (size_t i = 0; i < server.size(); ++i)
	{
		struct addrinfo info; 
		struct addrinfo *ptr;
	
		result = nullptr;
		memset(&info, 0, sizeof(info));
		info.ai_family = AF_INET;
		info.ai_socktype = SOCK_STREAM;
		string port = to_string(server[i].getPort());
		if (getaddrinfo(server[i].getHost().c_str(), port.c_str(), &info, &result) != 0)
		{
			::perror("getaddrinfo");
			break;
		}
		//getaddrinfo fills result with a list of potential fitting addr
		//Inner for loop scans the result of addr list until it finds 
		//a fitting fd for the current server
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
		{
			pollfd listenFd;
			//FIX:use ptr instead of result to update scanning
			listenFd.fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (listenFd.fd == -1)
			{
				::perror("socket");
				continue;
			}
			if (fcntl(listenFd.fd, F_SETFL, O_NONBLOCK) == -1)
			{
				::perror("fcntl");
				close(listenFd.fd);//FIX: added close() to prevent fd leak
				continue;
			}
			int on = true;
			if ((setsockopt(listenFd.fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) == -1)
			{
				::perror("setsockopt");
				close(listenFd.fd);
				continue;
			}
			if (::bind(listenFd.fd, ptr->ai_addr, ptr->ai_addrlen) == -1)
			{
				::perror("bind");
				close(listenFd.fd);
				continue;
			}
			listenFdsList.push_back(listenFd);
			break;
		}
		//FIX:free occupying space from result as it is no longer needed
		freeaddrinfo(result);
	}
	return (listenFdsList);
}

int server(const vector<ServerConfig> &servers)
{
	ServerInfo eloop;

	//eloop.fds holds the listening sockets, one per server block in the config. 
	//They never receive request data and never send responses. 
	//Their only job is to tell you that a new client is trying to connect.
	eloop.fds = createSockAddr(servers);
	for (size_t i = 0; i < eloop.fds.size(); i++)
	{
		if (listen(eloop.getFd()[i].fd, 10) != 0)
		{
			::perror("listen");
			//FIX: this for loop closes all listen fds when there listen() fails
			for (size_t j = 0; j < eloop.fds.size(); j++)
				close(eloop.fds[j].fd);
			return (1);
		}
	}
	return (eventLoop(eloop.getFd(), servers));
}

