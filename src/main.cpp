#include "configParser/Config.hpp"
#include "../inc/EventLoop.hpp"

int main(int ac, char **av)
{
    if (ac != 2)
    {
        cout << "Usage: ./webserv <config>" << endl;
        return 1;
    }

    try
    {
        Config config;
        const vector<ServerConfig>& servers = config.getServers();

        config.parse(av[1]);
        for (size_t i = 0; i < servers.size(); i++)
        {
            server(servers[i]);
        }
    }
    catch (const exception& e)
    {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}