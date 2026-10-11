#include "../inc/main.hpp"

const Location*	findLocation(const ServerConfig& server, const Request& request)
{
	const vector<Location>& locations = server.getLocations();
	const string requestPath = request.getPath();
	const Location* bestMatch = NULL;

	for (size_t i = 0; i < locations.size(); i++)
	{
		const string& locationPath = locations[i].getPath();

		if (locationPath.empty())
			continue;

		if (requestPath.compare(0, locationPath.size(), locationPath) != 0)
			continue;

		// Match complete path segments: /images must not match /images-old.
		if (requestPath.size() > locationPath.size() &&
			locationPath[locationPath.size() - 1] != '/' &&
			requestPath[locationPath.size()] != '/')
			continue;

		if (!bestMatch || locationPath.size() > bestMatch->getPath().size())
			bestMatch = &locations[i];
	}
	return bestMatch;
}

bool validateRoute(const ServerConfig& server, const Request& request, Route& route)
{
	route = Route();
	if (request.getStatusCode() != OK)
	{
		route.status = request.getStatusCode();
		return false;
	}
	const Location* targetLoc = findLocation(server, request);
	route.loc = targetLoc;
	if (!targetLoc)
	{
		route.status = PageNotFound;
		return false;
	}

	const vector<string>& methods = targetLoc->getMethods();
	bool methodAllowed = false;

	for (size_t i = 0; i < methods.size(); i++)
	{
		if (methods[i] == request.getMethod())
		{
			methodAllowed = true;
			break;
		}
	}

	if (!methodAllowed)
	{
		route.status = MethodNotAllowed;
		return false;
	}
	if (request.getContentLength() > server.getBodySize() ||
		request.getBody().size() > server.getBodySize())
	{
		route.status = ContentTooLarge;
		return false;
	}

	return true;
}

Route routing(const ServerConfig& server, const Request& request)
{
	Route route;
	if (!validateRoute(server, request, route))
		return route;

	// Other successful route actions can be selected here later.
	route.action = R_Static;
	return route;
}

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
					Route route = routing(client.getClientServer(), client.getRequest());
					if (route.action == R_Error)
						client.getRequest().setStatus(route.status);
					else
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
