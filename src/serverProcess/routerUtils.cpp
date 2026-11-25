#include "../../inc/main.hpp"
#include "../../inc/CGI.hpp"
#include <dirent.h>
#include <memory>

void runCGI(Request& request, const ServerConfig& server, Client& client, Route& route, ServerInfo& eloop)
{
	CGI cgi(request, server, client, route);
	eloop.populateCgi(client.getClientFd(), cgi);
	eloop.getClientCgi(client.getClientFd()).setupCgi(eloop);
	eloop.registerCgiPipes(client.getClientFd());
}

void runUpload(Request &request, Route &route)
{
	if (request.getStatusCode() == OK)
	{
		if (request.getContentLength() > 0 || request.getChunked() == true)
		{
			request.extractFileElements();
			request.setStatus(request.getStatusCode());
			if (request.getStatusCode() == OK)
				request.addFile(route.loc->getUploadPath());
		}
	}
}

void runRedirect(Request &request)
{
	request.setStatus(MovedPermanently);
	request.setStatusText(request.getStatusCode());
}

static string escapeHtml(const string &text)
{
	string escaped;
	for (char character : text)
	{
		switch (character)
		{
			case '&': escaped += "&amp;"; break;
			case '<': escaped += "&lt;"; break;
			case '>': escaped += "&gt;"; break;
			case '"': escaped += "&quot;"; break;
			default: escaped += character; break;
		}
	}
	return escaped;
}

static string encodePathSegment(const string &name)
{
	const char hex[] = "0123456789ABCDEF";
	string encoded;
	for (unsigned char character : name)
	{
		if ((character >= 'a' && character <= 'z')
			|| (character >= 'A' && character <= 'Z')
			|| (character >= '0' && character <= '9')
			|| character == '-' || character == '_' || character == '.' || character == '~')
			encoded += character;
		else
		{
			encoded += '%';
			encoded += hex[character >> 4];
			encoded += hex[character & 15];
		}
	}
	return encoded;
}

void runAutoIndex(Request &request, Route &route, Client &client)//Codex fix
{
	std::unique_ptr<DIR, int (*)(DIR *)> directory(opendir(route.fsPath.c_str()), closedir);
	if (!directory)
	{
		request.setStatus(errno == EACCES ? Forbidden : InternalServerError);
		return;
	}

	const string requestPath = request.getPath();
	string base = "./";
	if (requestPath.back() != '/')
		base += requestPath.substr(requestPath.rfind('/') + 1) + '/';

	string body = "<!doctype html><html><head><meta charset=\"utf-8\">"
		"<title>Directory listing</title></head><body><ul>\n";
	while (true)
	{
		errno = 0;
		const dirent *entry = readdir(directory.get());
		if (!entry)
		{
			if (errno != 0)
			{
				request.setStatus(InternalServerError);
				return;
			}
			break;
		}

		const string name = entry->d_name;
		if (name == "." || name == "..")
			continue;
		const string href = escapeHtml(base + encodePathSegment(name));
		body += "<li><a href=\"" + href + "\">" + escapeHtml(name) + "</a></li>\n";
	}
	body += "</ul></body></html>\n";
	client.getResponse().setBody(body);
}

void runDelete(Request &request, Route &route)
{
	if (request.getStatusCode() == OK) //  curl -X DELETE localhost:8080/uploads/cat.png
	{
		const char *cDeletePlace = route.fsPath.c_str();
		int status = remove(cDeletePlace);
		if (status != 0)
		{
			request.setStatus(PageNotFound);
			request.setStatusText(request.getStatusCode());
			return;
		}
		request.setStatus(NoContent);
		request.setStatusText(request.getStatusCode());
	}
}

void runStatic(Request &request, const ServerConfig &server, Route &route, Client &client)
{
	string b{};
	if (request.getStatusCode() == OK)
	{
		ifstream file(route.fsPath, std::ios::binary);
		if (!file.is_open())
		{
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
				while(getline(file, line))
					b += line + '\n';
			}
			file.close();
			client.getResponse().setBody(b);
			break ;
		}
		else if ((route.fsPath == server.getRoot() + it->second) && (request.getStatusCode() == OK))
			return ;
	}
	if (request.getStatusCode() == OK)
	{
		ifstream file(route.fsPath, std::ios::binary);
		string line;
		if (file.is_open())
		{
			while(getline(file, line))
				b += line + '\n';
		}
		file.close();
		client.getResponse().setBody(b);
	}
}
