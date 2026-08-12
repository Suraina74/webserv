#include "../inc/Server.hpp"
#include "../inc/EventLoop.hpp"

int	main(int ac, char **av)
{
	if (ac != 2)
	{
		cout << "usage: ./webserv config" << endl;
		return (1);
	}
	// if (server())
	// {
	// 	cout << "Server creation failed." << endl;
	// 	return (1);
	// }

	
	Config basicConfig;
	basicConfig.parse(av[1]);

	return (0);
}
