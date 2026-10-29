#include "../../inc/Client.hpp"

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
	//reject paths that could escape the root: must start with '/' and contain no ".." segment
	const string requestPath = request.getPath();
	if (requestPath.empty() || requestPath[0] != '/' || hasDotDotSegment(requestPath))
		return (route.status = BadRequest);
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

bool checkPathStat(string& pathName, Route& route, struct stat& pathStat)
{
	if (stat(pathName.c_str(), &pathStat) == -1)
	{
		if (errno == ENOENT || errno == ENOTDIR)
			route.status = PageNotFound;
		else if (errno == EACCES)
			route.status = Forbidden;
		else
			route.status = InternalServerError;
		return false;
	}
	return true;
}

void setRouteAction(Route& route, Request& request, const ServerConfig& server)
{
	const string& requestMethod = request.getMethod();
	const string& requestPath = request.getPath();
	//All request methods here are validated against server config
	//route.fsPath is parsed, but still need to check its status using checkPathStat()
	if (!route.loc->getRedirect().empty())
	{
		route.redirectTo = route.loc->getRedirect();
		route.action = R_REDIRECT;
		return;
	}
	//CGI: extension of the last path segment only, e.g. /cgi-bin/test.py -> .py
	size_t slashPos = requestPath.rfind('/');
	size_t dotPos = requestPath.rfind('.');
	if (dotPos != string::npos && (slashPos == string::npos || dotPos > slashPos))
	{
		string fileExtension = requestPath.substr(dotPos);
		if (route.loc->getCGIs().count(fileExtension) != 0)
		{
			if (requestMethod != "GET" && requestMethod != "POST")
			{
				route.status = MethodNotAllowed;
				return;
			}
			route.interpreter = route.loc->getCGIs().at(fileExtension);
			if (access(route.interpreter.c_str(), X_OK) == -1)
			{
				route.status = InternalServerError;//config error, not client's fault
				return;
			}
			route.action = R_CGI;//still need to check fsPathStat when constructing CGI obj
			return;
		}
	}
	//upload target usually doesn't exist yet, so it is decided before stat()
	if (requestMethod == "POST")
	{
		if (route.loc->getUploadEnabled())
			route.action = R_UPLOAD;
		else
			route.status = MethodNotAllowed;
		return;
	}

	struct stat fsPathStat;
	if (!checkPathStat(route.fsPath, route, fsPathStat))
		return;
	if (requestMethod == "DELETE")
	{
		if (S_ISREG(fsPathStat.st_mode))
			route.action = R_DELETE;
		else
			route.status = Forbidden;
		return;
	}
	//GET
	if (S_ISREG(fsPathStat.st_mode))
	{
		route.action = R_STATIC;
		return;
	}
	if (S_ISDIR(fsPathStat.st_mode))
	{
		//condition ? value_if_true : value_if_false
		const string& index = route.loc->getIndex().empty() ? server.getIndex() : route.loc->getIndex();
		if (!index.empty())
		{
			string indexPath = route.fsPath + '/' + index;
			struct stat indexPathStat;
			if (stat(indexPath.c_str(), &indexPathStat) == 0 && S_ISREG(indexPathStat.st_mode))
			{
				route.fsPath = indexPath;
				route.action = R_STATIC;
				return;
			}
		}
		if (route.loc->getDirectoryListing())
			route.action = R_AUTOINDEX;
		else
			route.status = Forbidden;
		return;
	}
	route.status = Forbidden;
}

int processRoute(Client& client, ServerInfo& eloop, Route& route)
{
	(void)eloop;
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
		setRouteAction(route, request, server);
		if (route.action == R_ERROR)
		{
			request.setStatus(route.status);
			return 1;
		}
		switch (route.action)
		{
			case R_CGI:
				runCGI(request, server, client.getFd());//fill response in client.setResponse()
				break;
			case R_UPLOAD:
			// A Client connects to a Server that listens on a specific port, like 8080.
			// The server has different location, a root, and index, a root, etc.
			//Upload function
				runUpload(request, route);
				break;
			case R_REDIRECT:
			//Redirect function
				break;
			case R_AUTOINDEX:
			//AutoIndex function: The autoindex handler builds an HTML response with clickable links to the files and subdirectories.
				break;
			case R_DELETE:
			//Delete function
				break;
			case R_STATIC:
			runStatic(request, server, route, client);
			// Files that are in the directory only reach this part.
			// still need to check file.is_open, because there could be permission issues or the file is deleted. Adjust the status code accordingly.
			// Prepare body and add to response.
			//Static function: 1.Read the file at route.fsPath
			//2.Put its contents into the HTTP response body 3.Set headers such as Content-Type: image/jpeg and Content-Length
				break;
			default:
				break;
		}
	}
	return 0;
}


// index
// The default file to look for when a directory is requested.
// If the browser requests /, the server may look for ./www/index.html.