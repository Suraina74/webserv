#include "../../inc/main.hpp"

void acceptClient(ServerInfo &eloop)
{
	//accept() creates/returns a new file descriptor for the newly connected client,
	//and that descriptor is separate from the listening socket.
	//listen fds are the first getListenCount() entries of _pfds
	for (size_t i = 0; i < eloop.getListenCount(); i++)
	{
		if (eloop.getPfds()[i].revents & POLLIN)
		{
			int listenFd = eloop.getPfds()[i].fd;
			int clientFd = accept(listenFd, NULL, NULL);
			if (clientFd == -1)
			{
				//one failed accept should not take the whole server down
				::perror("accept");
				continue;
			}
			//the client is served by the server block of the listen fd it connected to
			eloop.addClient(clientFd, eloop.getListenServer(listenFd));
		}
	}
}

int eventLoop(ServerInfo &eloop)
{
	while (1)
	{
		//getPfds().data() returns &pfds[0], one array with listen and client fds
		int ready = poll(eloop.getPfds().data(), eloop.getPfds().size(), TIMEOUT);

		if (ready == -1)
		{
			::perror("poll");
			eloop.closeAllFds();
			return (1);
		}
		acceptClient(eloop);//add client fds
		for (size_t i = eloop.getListenCount(); i < eloop.getPfds().size(); i++)
		{
			//re-read getPfds()[i] after every push_back/erase, never keep a reference
			int revents = eloop.getPfds()[i].revents;
			int clientFd = eloop.getPfds()[i].fd;
			Client &client = eloop.getClient(clientFd);
			if (revents & POLLIN)
			{
				//receiveRequest() parses the body, routes and composes the response itself
				int receivStatus = client.receiveRequest(eloop);
				if (receivStatus == PARSER_ERROR || receivStatus == 0)
				{
					eloop.removeClient(i);
					i--;
					continue;
				}
				else if (receivStatus == RESPONSE_READY)
				{
					// client.getRequest().postAndDelete();
					eloop.getPfds()[i].events = POLLOUT;
				}
			}
			else if (revents & POLLOUT)
			{
				int sendStatus = client.sendResponse();
				if (sendStatus == 0 || sendStatus == -1 || sendStatus == 2)
				{
					eloop.removeClient(i);
					i--;
					continue;
				}
			}
			else if (revents & (POLLERR | POLLNVAL))
			{
				eloop.removeClient(i);
				i--;
				continue;
			}
			else if (revents & POLLHUP)
			{
				eloop.removeClient(i);
				i--;
				continue;
			}
		}
	}
	return (0);
}
