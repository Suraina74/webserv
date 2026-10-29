#include "../../inc/main.hpp"
#include "../../inc/CGI.hpp"

void runCGI(Request &request, const ServerConfig &server, int clientFd)
{
    CGI newCGI = CGI(request, server, clientFd);
    
}

void runUpload(Request &request, Route &route)
{
	if (request.getContentLength() > 0 || request.getChunked() == true){
			request.extractFileElements();
			request.setStatusText(request.getStatusCode());
			if (request.getStatusCode() == OK)
			{
				request.addFile(route.loc->getUploadPath());
			}
		}
}

void runRedirect(Request &request, const ServerConfig &server, int clientFd)
{
	(void)request;
	(void)server;
	(void)clientFd;

}

void runAutoIndex(Request &request, const ServerConfig &server, int clientFd)
{
	(void)request;
	(void)server;
	(void)clientFd;

}

void runDelete(Request &request, const ServerConfig &server, int clientFd)
{
	(void)request;
	(void)server;
	(void)clientFd;

}

void runStatic(Request &request, const ServerConfig &server, Route &route, Client &client)
{
	string b{};
	if (request.getStatusCode() == OK){
		ifstream file(route.fsPath, std::ios::binary);
		if (!file.is_open()){
			request.setStatus(PageNotFound);
			request.setStatusText(PageNotFound);
		}
		file.close();
	}
	for (auto it = server.getErrPages().begin(); it != server.getErrPages().end(); it++)
	{
		if ((route.fsPath == server.getRoot() + it->second) && (request.getStatusCode() != OK))
		{
			ifstream file(route.fsPath, std::ios::binary);
			string line;
			if (file.is_open())
			{
				while(getline(file, line)){
					b += line + '\n';
				}
			}
			file.close();
			client.getResponse().setBody(b);
			break ;
		}
		else if ((route.fsPath == server.getRoot() + it->second) && (request.getStatusCode() == OK))
		{
			return ;
		}
	}
	if (request.getStatusCode() == OK){
		ifstream file(route.fsPath, std::ios::binary);
		string line;
		if (file.is_open())
		{
			while(getline(file, line)){
				b += line + '\n';
			}
		}
		file.close();
		client.getResponse().setBody(b);
	}
}