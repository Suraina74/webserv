#include "../inc/EventLoop.hpp"
#include "configParser/ServerConfig.hpp"

int	server(const ServerConfig &servers)
{
	struct addrinfo *result = nullptr;
	int listen_fd = 0;

	if (createSockAddr(&listen_fd, result, servers) != 0)
		return (1);
	//128 in listen(sockfd, 128) is referred to as backlog number
	//it is the maximum number of client connections that can be fully 
	//connected (finished the TCP handshake) but not yet accept()ed by your program, 
	//sitting and waiting in a kernel-managed queue. SOMAXCONN returns the actual max queue
	//the os can handle, however 128 is more of a universally acknowledged reasonable value.
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