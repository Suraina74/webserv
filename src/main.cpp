#include "../inc/Server.hpp"
#include "../inc/EventLoop.hpp"

int	main(int ac, char **av)
{
	(void)av;
	if (ac != 2)
	{
		std::cout << "usage: ./webserv config" << std::endl;
		return (1);
	}
	if (server())
	{
		std::cout << "Server creation failed." << std::endl;
		return (1);
	}

	// Config config;
	// config.parse(av[1]);

	return (0);
}
