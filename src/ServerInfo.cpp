#include "configParser/ServerConfig.hpp"
#include "../inc/ServerInfo.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int receiveRequest(int clientFd, Request& request){
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
	if (request.getFullRequest().find("\r\n\r\n") != std::string::npos && request.getHeaderBytes() == 0){
		if ((request.parseUntilHeaders(request.getFullRequest())) == false){
			return 2;
		}
		request.setHeaderBytes(request.getRequestTillHeaders().size());
	}
	// Er kan ook chunked transfer encoding zijn. Dan is er geen content length;
	if ((request.getContentLength()) && (request.getBytesRead() == request.getHeaderBytes() + request.getContentLength())){
		return 2;
	}
	return 1;
}

int sendResponse(int clientFd, Response& response){
	response.composeResponse();
	std::string fullResponse = response.getFullResponse();
	response.setLenResponse(fullResponse.length());
	response.setCString(fullResponse.c_str());
	if (response.getBytesSent() < response.getLenResponse()){
		int n = send(clientFd, response.getCFullResponse() + response.getBytesSent(), response.getLenResponse() - response.getBytesSent(), 0);
		if (n == -1){
			::perror("send");
			return -1;
		}
		else if (n == 0){
			return 0;
		}
		response.setBytesSent(response.getBytesSent() + n);
	}
	if (response.getBytesSent() == response.getLenResponse()){
		return 2;
	}
	return 1;
}

int eventLoop(const vector<pollfd> &fds, const vector<ServerConfig> &servers)
{
	(void)servers;
	ServerInfo serverData;
	Request request;
	Response response;
	//setup listining sockets
	for (size_t i = 0; i < fds.size(); ++i)
	{
		pollfd listen_socket;

		listen_socket.fd = fds[i].fd;
		listen_socket.events = POLLIN;
		listen_socket.revents = 0;

		//why create a general list of fds when you can keep them seprate? just cleanup on both.
		serverData.fds.push_back(listen_socket);
	}
	while (1)
	{
		pollfd client_pfd;

		size_t nfds = serverData.fds.size();
		int ready = poll(serverData.fds.data(), nfds, TIMEOUT);
		if (ready == -1)
		{
			::perror("poll");
			return (1);
		}
		for (size_t i = 0; i < nfds; i++)
		{
			//begining of list is listining fds but I could instead use a list of listen fds todo this part..
			if (i < fds.size())
			{
				if (serverData.fds[i].revents & POLLIN)
				{
					client_pfd.fd = accept(fds[i].fd, NULL, NULL);
					if (client_pfd.fd == -1)
					{
						::perror("accept");
						return (1);
					}
					client_pfd.events = POLLIN;
					client_pfd.revents = 0;
					serverData.fds.push_back(client_pfd);
					nfds++;
				}
				continue;
			}
			//client fds
			if ((serverData.fds[i].revents & POLLIN))
			{
				int returnValue = receiveRequest(serverData.fds[i].fd, request);
				if (returnValue == -1 || returnValue == 0){
					close(serverData.fds[i].fd);
					serverData.fds.erase(serverData.fds.begin() + i);
					nfds--;
					i--;
					continue;
				}
				else if (returnValue == 2){
					request.parseBody();
					//request.action?
					// check request against config file. To see what server (check host header) applies and what location applies.
					serverData.fds[i].events = POLLOUT;
				}
			}
			else if (serverData.fds[i].revents & POLLOUT)
			{
				response.setRequest(request);
				int returnValue = sendResponse(serverData.fds[i].fd, response);
				if (returnValue == 0 || returnValue == -1){
					close(serverData.fds[i].fd);
					serverData.fds.erase(serverData.fds.begin() + i);
					nfds--;
					i--;
					continue;
				}
				else if (returnValue == 2){
					close(serverData.fds[i].fd);
					serverData.fds.erase(serverData.fds.begin() + i);
					nfds--;
					i--;
					request.cleanRequest();
					response.cleanResponse();
				}
			}
		}
	}
	return (0);
}

vector<pollfd> createSockAddr(struct addrinfo *result, const vector<ServerConfig> &server)
{
	vector<pollfd>	listenFdsList;

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
			pollfd listenFd;
			listenFd.fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (listenFd.fd == -1)
			{
				::perror("socket");
				continue;
			}
			if (fcntl(listenFd.fd, F_SETFL, O_NONBLOCK) == -1)
			{
				::perror("fcntl");
				continue;
			}
			int on = true;
			if ((setsockopt(listenFd.fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) == -1)
			{
				::perror("setsockopt");
				continue;
			}
			if (::bind(listenFd.fd, ptr->ai_addr, ptr->ai_addrlen) == -1)
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
	ServerInfo eloop;

	eloop.result = nullptr;
	eloop.fds = createSockAddr(eloop.result, servers);
	for (size_t i = 0; i < eloop.fds.size(); i++)
	{
		if (listen(eloop.fds[i].fd, 10) != 0)
		{
			::perror("listen");
			return (1);
		}
	}
	return (eventLoop(eloop.fds, servers));
}

// void ServerInfo::setResult(struct addrinfo *res){
// 	result = res;
// }
// void ServerInfo::setClient(Client client){
	
// }
// void ServerInfo::setFd(pollfd fd){

// }

// struct addrinfo ServerInfo::getResult(){

// }
// vector<Client> ServerInfo::getClients(){

// }
// vector<pollfd> ServerInfo::getFds(){

// }


//todos
// Socket cleanup, error events, partial sends, and server-to-config mapping need work.