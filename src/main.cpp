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
		return (1);
	return (0);
}