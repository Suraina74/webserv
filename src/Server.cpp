#include "../inc/EventLoop.hpp"
#include "configParser/ServerConfig.hpp"

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