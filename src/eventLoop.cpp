#include "../inc/main.hpp"

int eventLoop(const vector<pollfd> &fds, const vector<ServerConfig> &servers)
{
	ServerInfo serverData;
	Request request;
	Response response;
	vector<Client> clients;
	// setup listining sockets
	for (size_t i = 0; i < fds.size(); ++i)
	{
		pollfd listen_socket;

		listen_socket.fd = fds[i].fd;
		listen_socket.events = POLLIN;
		listen_socket.revents = 0;
		serverData.getFds().push_back(listen_socket);
	}
	while (1)
	{
		pollfd client_pfd;
		size_t nfds = serverData.getFds().size();
		int ready = poll(serverData.getFds().data(), nfds, TIMEOUT);

		if (ready == -1)
		{
			::perror("poll");
			return (1);
		}
		for (size_t i = 0; i < nfds; i++)
		{
			// i keeps index of listening sock
			if (i < fds.size())
			{
				if (serverData.getFds()[i].revents & POLLIN)
				{
					client_pfd.fd = accept(fds[i].fd, NULL, NULL);
					if (client_pfd.fd == -1)
					{
						::perror("accept");
						return (1);
					}
					client_pfd.events = POLLIN;
					client_pfd.revents = 0;
					serverData.getFds().push_back(client_pfd);
					Client client(client_pfd.fd, &servers[0], request, response);
					clients.push_back(client);
					nfds++;
				}
				continue;
			}
			// client fds
			if ((serverData.getFds()[i].revents & POLLIN))
			{
				int returnValue = receiveRequest(serverData.getFds()[i].fd, clients[i - fds.size()].getRequest());
				if (returnValue == -1 || returnValue == 0)
				{
					clients.erase(clients.begin() + i - fds.size());
					close(serverData.getFds()[i].fd);
					serverData.getFds().erase(serverData.getFds().begin() + i);
					nfds--;
					i--;
					continue;
				}
				else if (returnValue == 2)
				{
					clients[i - fds.size()].getRequest().parseBody();
					clients[i - fds.size()].getRequest().postAndDelete();
					//  check request against config file. To see what server (check host header) applies and what location applies.
					serverData.getFds()[i].events = POLLOUT;
				}
			}
			else if (serverData.getFds()[i].revents & POLLOUT)
			{
				clients[i - fds.size()].getResponse().setRequest(clients[i - fds.size()].getRequest());
				int returnValue = sendResponse(serverData.getFds()[i].fd, clients[i - fds.size()].getResponse());
				if (returnValue == 0 || returnValue == -1)
				{
					clients.erase(clients.begin() + i - fds.size());
					close(serverData.getFds()[i].fd);
					serverData.getFds().erase(serverData.getFds().begin() + i);
					nfds--;
					i--;
					continue;
				}
				else if (returnValue == 2)
				{
					clients.erase(clients.begin() + i - fds.size());
					close(serverData.getFds()[i].fd);
					serverData.getFds().erase(serverData.getFds().begin() + i);
					nfds--;
					i--;
				}
			}
		}
	}
	return (0);
}