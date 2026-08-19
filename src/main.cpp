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

            const vector<Location>& locations = servers[i].getLocations();

            cout << "  Locations: " << locations.size() << endl;

            for (size_t j = 0; j < locations.size(); ++j)
            {
                const Location& loc = locations[j];

                cout << "    Location " << j + 1 << ": " << loc.getPath() << endl;

                const vector<string>& methods = loc.getMethods();
                cout << "      Methods:";
                for (size_t k = 0; k < methods.size(); ++k)
                    cout << " " << methods[k];
                cout << endl;

                cout << "      Root: " << loc.getRoot() << endl;
                cout << "      Index: " << loc.getIndex() << endl;
                cout << "      Redirect: " << loc.getRedirect() << endl;
                cout << "      Upload enabled: " << (loc.getUploadEnabled() ? "on" : "off") << endl;
                cout << "      Upload path: " << loc.getUploadPath() << endl;
                cout << "      Directory listing: " << (loc.getDirectoryListing() ? "on" : "off") << endl;

                const map<string, string>& cgi = loc.getCGI();
                for (map<string, string>::const_iterator it = cgi.begin();
                     it != cgi.end(); ++it)
                {
                    cout << "      CGI " << it->first
                         << ": " << it->second << endl;
                }
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