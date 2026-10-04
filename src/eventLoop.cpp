#include "../inc/main.hpp"

int eventLoop(const vector<pollfd> &listenFds, const vector<ServerConfig> &servers)
{
	ServerInfo serverData;
	Request request;
	Response response;
	vector<Client> clients;
	// setup listining sockets
	for (size_t i = 0; i < listenFds.size(); ++i)
	{
		pollfd listen_socket;

		listen_socket.fd = listenFds[i].fd;
		listen_socket.events = POLLIN;
		listen_socket.revents = 0;
		serverData.getPfds().push_back(listen_socket);
	}
	while (1)
	{
		pollfd client_pfd;
		size_t nfds = serverData.getPfds().size();
		int ready = poll(serverData.getPfds().data(), nfds, TIMEOUT);

		if (ready == -1)
		{
			::perror("poll");
			return (1);
		}
		for (size_t i = 0; i < nfds; i++)
		{
			// i keeps index of listening sock
			if (i < listenFds.size())
			{
				if (serverData.getPfds()[i].revents & POLLIN)
				{
					client_pfd.fd = accept(listenFds[i].fd, NULL, NULL);
					if (client_pfd.fd == -1)
					{
						::perror("accept");
						return (1);
					}
					client_pfd.events = POLLIN;
					client_pfd.revents = 0;
					serverData.getPfds().push_back(client_pfd);
					Client client(client_pfd.fd, &servers[0], request, response);
					clients.push_back(client);
					nfds++;
				}
				continue;
			}
			// client fds
			if ((serverData.getPfds()[i].revents & POLLIN))
			{
				int returnValue = receiveRequest(serverData.getPfds()[i].fd, clients[i - listenFds.size()].getRequest());
				if (returnValue == -1 || returnValue == 0)
				{
					clients.erase(clients.begin() + i - listenFds.size());
					close(serverData.getPfds()[i].fd);
					serverData.getPfds().erase(serverData.getPfds().begin() + i);
					nfds--;
					i--;
					continue;
				}
				else if (returnValue == 2)
				{
					clients[i - listenFds.size()].getRequest().parseBody();
					clients[i - listenFds.size()].getRequest().postAndDelete();
					//  check request against config file. To see what server (check host header) applies and what location applies.
					serverData.getPfds()[i].events = POLLOUT;
				}
			}
			else if (serverData.getPfds()[i].revents & POLLOUT)
			{
				clients[i - listenFds.size()].getResponse().setRequest(clients[i - listenFds.size()].getRequest());
				int returnValue = sendResponse(serverData.getPfds()[i].fd, clients[i - listenFds.size()].getResponse());
				if (returnValue == 0 || returnValue == -1)
				{
					clients.erase(clients.begin() + i - listenFds.size());
					close(serverData.getPfds()[i].fd);
					serverData.getPfds().erase(serverData.getPfds().begin() + i);
					nfds--;
					i--;
					continue;
				}
				else if (returnValue == 2)
				{
					clients.erase(clients.begin() + i - listenFds.size());
					close(serverData.getPfds()[i].fd);
					serverData.getPfds().erase(serverData.getPfds().begin() + i);
					nfds--;
					i--;
				}
			}
		}
	}
	return (0);
}