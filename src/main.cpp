#include "../inc/Config.hpp"
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
        config.parse(av[1]);

        const vector<ServerConfig>& servers = config.getServers();

        for (size_t i = 0; i < servers.size(); ++i)
        {
            cout << "Server " << i + 1 << endl;
            cout << "  Port: " << servers[i].getPort() << endl;
            cout << "  Host: " << servers[i].getHost() << endl;
            cout << "  Name: " << servers[i].getServerName() << endl;
            cout << "  Root: " << servers[i].getRoot() << endl;
            cout << "  Index: " << servers[i].getIndex() << endl;
            cout << "  Body size: " << servers[i].getBodySize() << endl;

            
            const map<int, string>& errors = servers[i].getErrPages();

            for (map<int, string>::const_iterator it = errors.begin();
                 it != errors.end(); ++it)
            {
                cout << "  Error " << it->first
                     << ": " << it->second << endl;
            }
        }
        server(servers[0]);
    }
    catch (const exception& e)
    {
        cerr << e.what() << endl;
        return 1;
    }
    
    return 0;
}