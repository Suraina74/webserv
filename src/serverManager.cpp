#include "../inc/main.hpp"

//return values:
//-1 = recv() failed, close the client
// 0 = client closed the connection, close the client
// 1 = request not complete yet, keep reading on the next POLLIN
// 2 = request complete (or header parse failed, error is in statusCode), switch to POLLOUT
int receiveRequest(int clientFd, Request &request)
{
	char buffer[2048];
	ssize_t n = recv(clientFd, buffer, sizeof(buffer), 0);
	if (n == -1)
	{
		::perror("recv");
		return -1;	//recv error
	}
	else if (n == 0)
		return 0;	//client closed the connection
	request.setBytesRead(request.getBytesRead() + n);
	std::string part(buffer, n);
	request.setRequest(request.getFullRequest() + part);
	if (request.getFullRequest().find("\r\n\r\n") != std::string::npos && request.getHeaderBytes() == 0)
	{
		if ((request.parseUntilHeaders(request.getFullRequest())) == false)
			return 2;	//bad headers, stop reading and send the error response
		request.setHeaderBytes(request.getRequestTillHeaders().size());
	}
	if (request.getChunked() == true && request.getFullRequest().find("0\r\n\r\n") != std::string::npos)
		return 2;	//last chunk received, request complete
	else if (request.getBytesRead() == request.getHeaderBytes() + request.getContentLength())
		return 2;	//headers + full body received, request complete
	return 1;	//request incomplete, wait for more data
}

//return values:
//-1 = send() failed, close the client
// 0 = nothing was sent, close the client
// 1 = response partly sent, send the rest on the next POLLOUT
// 2 = response fully sent, close the client (Connection: close)
int sendResponse(int clientFd, Response &response)
{
	response.composeResponse();
	string fullResponse = response.getFullResponse();
	response.setLenResponse(fullResponse.length());
	response.setCString(fullResponse.c_str());
	if (response.getBytesSent() < response.getLenResponse())
	{
		int n = send(clientFd, response.getCFullResponse() + response.getBytesSent(), response.getLenResponse() - response.getBytesSent(), 0);
		if (n == -1)
		{
			::perror("send");
			return -1;	//send error
		}
		else if (n == 0)
			return 0;	//nothing sent
		response.setBytesSent(response.getBytesSent() + n);
	}
	if (response.getBytesSent() == response.getLenResponse())
		return 2;	//whole response sent
	return 1;	//response partly sent, rest goes out on the next POLLOUT
}

//FIX: moved struct addrinfo *result; from ServerInfo class to local variable inside createSockAddr()
//because it is just a assisting variable not needed elsewhere.
void createSockAddr(const vector<ServerConfig> &server, ServerInfo &eloop)
{
	//suggestion:change para name server to serverList for clarity
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
			//remember which server block this listen fd belongs to
			eloop.addListenFd(listenFd.fd, &server[i]);
			break;
		}
		//FIX:free occupying space from result as it is no longer needed
		freeaddrinfo(result);
	}
}

int server(const vector<ServerConfig> &servers)
{
	ServerInfo eloop;

	//the listen fds go first in eloop's pfds, one per server block in the config.
	//They never receive request data and never send responses.
	//Their only job is to tell you that a new client is trying to connect.
	createSockAddr(servers, eloop);
	if (eloop.getListenCount() == 0)
	{
		cerr << "Error: no listening sockets could be created" << endl;
		return (1);
	}
	for (size_t i = 0; i < eloop.getListenCount(); i++)
	{
		if (listen(eloop.getPfds()[i].fd, 10) != 0)
		{
			::perror("listen");
			//FIX: closes all listen fds when listen() fails
			eloop.closeAllFds();
			return (1);
		}
	}
	return (eventLoop(eloop));
}
