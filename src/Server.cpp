#include "../inc/Server.hpp"
#include "../inc/EventLoop.hpp"

int	server()
{
	struct addrinfo *result = nullptr;
	int sockfd = 0;

	if (createSockAddr(&sockfd, result) != 0)
		return (1);
	//128 in listen(sockfd, 128) is referred to as backlog number
	//it is the maximum number of client connections that can be fully 
	//connected (finished the TCP handshake) but not yet accept()ed by your program, 
	//sitting and waiting in a kernel-managed queue. SOMAXCONN returns the actual max queue
	//the os can handle, however 128 is more of a universally acknowledged reasonable value.
	freeaddrinfo(result);
	if (listen(sockfd, 10) != 0)
	{
		perror("listen");
		return (1);
	}
	if (eventLoop(&sockfd))
		return (1);
	return (0);
}