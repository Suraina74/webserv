#include "../inc/main.hpp"

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
		acceptClient(eloop);
		//clients accepted in this round are appended with revents 0,
		//so they are skipped until the next poll()
		for (size_t i = eloop.getListenCount(); i < eloop.getPfds().size(); i++)
		{
			//re-read getPfds()[i] after every push_back/erase, never keep a reference
			int revents = eloop.getPfds()[i].revents;
			int clientFd = eloop.getPfds()[i].fd;
			Client &client = eloop.getClient(clientFd);
			if (revents & POLLIN)
			{
				int receivStatus = receiveRequest(clientFd, client.getRequest());
				if (receivStatus == -1 || receivStatus == 0)
				{
					eloop.removeClient(i);
					i--;
					continue;
				}
				else if (receivStatus == 2)
				{
					client.getRequest().parseBody();
					//routing
					client.getRequest().postAndDelete();
					eloop.getPfds()[i].events = POLLOUT;
				}
			}
			else if (revents & POLLOUT)
			{
				client.getResponse().setRequest(client.getRequest());
				int sendStatus = sendResponse(clientFd, client.getResponse());
				if (sendStatus == 0 || sendStatus == -1 || sendStatus == 2)
				{
					eloop.removeClient(i);
					i--;
					continue;
				}
			}
		}
	}
	return (0);
}
