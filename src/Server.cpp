#include "../inc/Server.hpp"
#include "../inc/EventLoop.hpp"

int	server()
{
	struct addrinfo *result = nullptr;
	int sockfd = 0;

	if (createSockAddr(&sockfd, result))
		return (1);
	freeaddrinfo(result);
	if (listen(sockfd, 10) != 0){
		perror("listen");
		return (1);
	}
	if (eventLoop(&sockfd))
		return (1);
	return (0);
}