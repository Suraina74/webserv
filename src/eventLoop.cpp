#include "../inc/main.hpp"

const Location*	findLocation(const ServerConfig& server, const Request& request)
{
	const vector<Location>& locations = server.getLocations();
	const string requestPath = request.getPath();
	const Location* bestLoc = NULL;

	for (size_t i = 0; i < locations.size(); i++)
	{
		const string& locationPath = locations[i].getPath();

		if (requestPath.compare(0, locationPath.size(), locationPath) != 0)
			continue;

		// Match complete path segments: /images must not match /images-old.
		if (requestPath.size() > locationPath.size() &&
    		locationPath.back() != '/' && requestPath[locationPath.size()] != '/')
			continue;

		if (!bestLoc || locationPath.size() > bestLoc->getPath().size())
			bestLoc = &locations[i];
	}
	return bestLoc;
}

int checkRouteStat(Request& request, const ServerConfig& server, Route &route)
{
	route.status = request.getStatusCode();
	route.loc = findLocation(server, request);
	if (!route.loc)
		route.status = PageNotFound;

	const vector<string>& methods = route.loc->getMethods();
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
		route.status = MethodNotAllowed;
	if (request.getContentLength() > server.getBodySize() || request.getBody().size() > server.getBodySize())
		route.status = ContentTooLarge;

	if (route.status != OK)
		return 1;
	else
		return 0;
}

int populateRoute(Client &client, ServerInfo &eloop, Route &route)
{
	Request& request = client.getRequest();
	const ServerConfig& server = client.getClientServer();

	if (checkRouteStat(request, server, route) == 1)
	{
		client.getRequest().setStatus(route.status);
		return 1;
	}
	else
	{
		//under modification, check when restart
		struct stat info;
		string root = route.loc->getRoot();
		if (root.empty())
		    root = server.getRoot();

		string suffix = request.getPath().substr(route.loc->getPath().size());

		// Join with exactly one slash between root and suffix.
		if (!root.empty() && root.back() != '/')
		    root += '/';
		if (!suffix.empty() && suffix[0] == '/')
		    suffix.erase(0, 1);

		route.fsPath = root + suffix;
		if (stat(route.fsPath.c_str(), &info) == -1)
		{
		    // Determine the error response from the failure reason.
		}
		else if (S_ISDIR(info.st_mode))
		{
		    // Look for the configured index file,
		    // or handle directory listing.
		}
		else if (S_ISREG(info.st_mode))
		{
		    // Open the file and serve its contents.
		}
		client.getRequest().postAndDelete();
	}
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
					Route route{};
					populateRoute(client, eloop, route);
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
