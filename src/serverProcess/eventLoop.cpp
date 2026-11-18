#include "../../inc/main.hpp"
#include "../../inc/ServerInfo.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void acceptClient(ServerInfo &eloop)
{
	struct sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(clientAddr);
	//accept() creates/returns a new file descriptor for the newly connected client,
	//and that descriptor is separate from the listening socket.
	//listen fds are the first getListenCount() entries of _pfds
	for (size_t i = 0; i < eloop.getListenCount(); i++)
	{
		if (eloop.getPfds()[i].revents & POLLIN)
		{
			int listenFd = eloop.getPfds()[i].fd;
			int clientFd = accept(listenFd, reinterpret_cast<struct sockaddr*>(&clientAddr), &addrLen);
			if (clientFd == -1)
			{
				//one failed accept should not take the whole server down
				::perror("accept");
				continue;
			}
			char ip[INET_ADDRSTRLEN];
			if (inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip)) != NULL)
			{
			    string remoteAddr = ip; // e.g. "127.0.0.1"
				eloop.addClient(clientFd, eloop.getListenServer(listenFd), remoteAddr);
			}
			//the client is served by the server block of the listen fd it connected to
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
			int fd = eloop.getPfds()[i].fd;
			int revents = eloop.getPfds()[i].revents;
			int clientFd = eloop.findCgiOwner(fd);
			
			
			if (clientFd != -1)
			{
			    CGI& curCgi = eloop.getClientCgi(clientFd);
			
			    if (fd == curCgi.getInFd())
			    {
			        int result = 1;
			        if (revents & (POLLERR | POLLHUP | POLLNVAL))
			            result = -1;
			        else if (revents & POLLOUT)
			            result = curCgi.writeInput();
				
			        if (result != 1)
			        {
			            eloop.unregisterCgiPipe(i);
			            curCgi.closeIn();
			            --i;
			        }
			    }
			    else if (fd == curCgi.getOutFd())
			    {
			        int result = 1;
			        if (revents & (POLLERR | POLLNVAL))
			            result = -1;
			        else if (revents & (POLLIN | POLLHUP))
			            result = curCgi.readOutput();
				
			        if (result == 1)
			            continue;
				
			        eloop.unregisterCgiPipe(i);
			        curCgi.closeOut();
			        --i;
				
			        std::string response =
			            "HTTP/1.1 502 Bad Gateway\r\n"
			            "Content-Length: 0\r\n"
			            "Connection: close\r\n\r\n";
				
			        if (result == 0)
			        {
			            // CGI output starts with headers, then a blank line, then the body.
			            const std::string& output = curCgi.getOutput();
			            std::istringstream stream(output);
			            std::string status = "200 OK";
			            std::string headers;
			            std::string line;
					
			            while (std::getline(stream, line))
			            {
			                if (!line.empty() && line.back() == '\r')
			                    line.pop_back();
						
			                if (line.empty())
			                {
			                    std::string body = output.substr(
			                        static_cast<size_t>(stream.tellg()));
			                    response = "HTTP/1.1 " + status + "\r\n" + headers
			                        + "Content-Length: " + std::to_string(body.size())
			                        + "\r\nConnection: close\r\n\r\n" + body;
			                    break;
			                }
						
			                size_t colon = line.find(':');
			                if (colon == std::string::npos || colon == 0)
			                    break;
						
			                std::string name = line.substr(0, colon);
			                for (char& c : name)
			                    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
						
			                if (name == "status")
			                {
			                    size_t start = line.find_first_not_of(" \t", colon + 1);
			                    if (start == std::string::npos)
			                        break;
			                    status = line.substr(start);
			                }
			                else if (name != "content-length" && name != "transfer-encoding"
			                         && name != "connection")
			                {
			                    headers += line + "\r\n";
			                }
			            }
			        }
				
			        eloop.getClient(clientFd).setCgiResponse(response);
			        for (pollfd& entry : eloop.getPfds())
			        {
			            if (entry.fd == clientFd)
			            {
			                entry.events = POLLOUT;
			                break;
			            }
			        }
			    }
			    continue;
			}

			Client &client = eloop.getClient(fd);
			
			if (client.timer(chrono::steady_clock::now(), client.getTimeout()))
			{
				eloop.removeClient(i);
				i--;
				continue;
			}
			if (revents & POLLIN)//A client has connected
			{
				//receiveRequest() parses the body, routes and composes the response itself
				client.receiveRequest(eloop);
				int receivStatus = client.getRequest().getParserState();
				if (receivStatus == ERROR)
				{
					eloop.removeClient(i);
					i--;
					continue;
				}
				else if (receivStatus == REQUESTLINE || receivStatus == HEADERS || receivStatus == BODY)
				{
					eloop.getPfds()[i].events = POLLIN;
					i--;
					continue;
				}
				else if (receivStatus == COMPLETE)
				{
					eloop.getPfds()[i].events = POLLOUT;
				}
			}
			else if (revents & POLLOUT)
			{
				int sendStatus = client.sendResponse();
				if (sendStatus == 0 || sendStatus == -1)
				{
					eloop.removeClient(i);
					i--;
					continue;
				}
				else if (sendStatus == 2)
					eloop.getPfds()[i].events = POLLIN;
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
