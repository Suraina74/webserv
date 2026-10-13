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

httpStatus validateRouteStat(Request& request, const ServerConfig& server, Route &route)
{
	//validate location
	route.loc = findLocation(server, request);
	if (!route.loc)
		return (route.status = PageNotFound);
	//validate method
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
		return (route.status = MethodNotAllowed);
	//validate content length
	if (request.getContentLength() > server.getBodySize() || request.getBody().size() > server.getBodySize())
		return (route.status = ContentTooLarge);

	//populate fsPath
	string root = route.loc->getRoot();
	if (root.empty())//make sure if loc root is empty give loc the server root
	    root = server.getRoot();
	//if request path is '/image/cats.png' location path is '/image', suffix is '/cats.png'
	string suffix = request.getPath().substr(route.loc->getPath().size());
	if (!suffix.empty() && suffix[0] == '/')
	    suffix.erase(0, 1);
	route.fsPath = root + '/' + suffix;

	return OK;
}

static httpStatus statErrorStatus()
{
	if (errno == ENOENT || errno == ENOTDIR)
		return PageNotFound;
	if (errno == EACCES)
		return Forbidden;
	return InternalServerError;
}

void setRouteAction(Route& route, Request& request)
{
	// Called only after validateRouteStat() succeeds.
	if (!route.loc->getRedirect().empty())
	{
		route.redirectTo = route.loc->getRedirect();
		route.action = R_REDIRECT;
		return;
	}

	const string& method = request.getMethod();
	// Uploads may create a file: do not require fsPath to exist.
	// A matching CGI script takes precedence over an upload.
	const map<string, string>& cgi = route.loc->getCGI();
	size_t dot = route.fsPath.find_last_of('.');
	size_t slash = route.fsPath.find_last_of('/');
	bool matchesCGI = dot != string::npos &&
		(slash == string::npos || dot > slash) &&
		cgi.find(route.fsPath.substr(dot)) != cgi.end();
	if (method == "POST" && !matchesCGI && route.loc->getUploadEnabled())
	{
		route.action = R_Upload;
		return;
	}
	if (method != "GET" && method != "POST" && method != "DELETE")
	{
		route.status = MethodNotAllowed;
		return;
	}

	struct stat info;
	if (stat(route.fsPath.c_str(), &info) == -1)
	{
		route.status = statErrorStatus();
		return;
	}
	if (method == "DELETE")
	{
		// The delete handler must check permissions and perform the deletion.
		if (S_ISREG(info.st_mode))
			route.action = R_Delete;
		else
			route.status = Forbidden;
		return;
	}
	if (S_ISDIR(info.st_mode))
	{
		if (method != "GET")
		{
			route.status = MethodNotAllowed;
			return;
		}
		if (!route.loc->getIndex().empty())
		{
			string indexPath = route.fsPath;
			if (indexPath.empty() || indexPath.back() != '/')
				indexPath += '/';
			indexPath += route.loc->getIndex();
			struct stat indexInfo;
			if (stat(indexPath.c_str(), &indexInfo) == 0)
			{
				if (S_ISREG(indexInfo.st_mode))
				{
					route.fsPath = indexPath;
					info = indexInfo;
				}
			}
			else if (errno != ENOENT && errno != ENOTDIR)
			{
				route.status = statErrorStatus();
				return;
			}
		}
		if (S_ISDIR(info.st_mode))
		{
			if (route.loc->getDirectoryListing())
				route.action = R_AUTOINDEX;
			else
				route.status = Forbidden;
			return;
		}
	}
	if (!S_ISREG(info.st_mode))
	{
		route.status = Forbidden;
		return;
	}
	// Check again after resolving a directory's index file.
	dot = route.fsPath.find_last_of('.');
	slash = route.fsPath.find_last_of('/');
	if (dot != string::npos && (slash == string::npos || dot > slash))
	{
		auto it = cgi.find(route.fsPath.substr(dot));
		if (it != cgi.end())
		{
			route.interpreter = it->second;
			route.action = R_CGI;
			return;
		}
	}
	if (method == "GET")
		route.action = R_Static;
	else
		route.status = MethodNotAllowed;
}

void runCGI(Client&, Route&)
{
}

int processRoute(Client& client, ServerInfo&, Route& route)
{
	Request& request = client.getRequest();
	const ServerConfig& server = client.getClientServer();
	//validateRouteStat() validate route and sets its error status and returns 1 if it is invalid
	if (validateRouteStat(request, server, route) != OK)
	{
		request.setStatus(route.status);
		return 1;
	}
	else
	{
		setRouteAction(route, request);
		if (route.action == R_Error)
		{
			request.setStatus(route.status);
			return 1;
		}
		switch (route.action)
		{
			case R_CGI:
				runCGI(client, route);//fill response in client.setResponse()
				break;
			case R_Upload:
			//Upload function
				break;
			case R_REDIRECT:
			//Redirect function
				break;
			case R_AUTOINDEX:
			//AutoIndex function
				break;
			case R_Delete:
			//Delete function
				break;
			case R_Static:
			//Static function
				break;
			default:
				break;
		}
	}
	return 0;
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
					processRoute(client, eloop, route);
					//client.getRequest().postAndDelete(); 
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
