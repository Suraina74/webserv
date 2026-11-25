#include "../../inc/CGI.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <cerrno>
#include "../../inc/ServerInfo.hpp"

CGI::CGI():_env(), _interpreter(""), _scriptPath(""), _requestBody(""), _bodySent(0), _output(""), _clientFd(-1), _inFd(-1), _outFd(-1), _pid(-1) {}

CGI::~CGI()
{
	if (_inFd != -1)
        close(_inFd);

    if (_outFd != -1)
        close(_outFd);
}

CGI::CGI(Request &request, const ServerConfig &server, Client& client, Route& route): CGI()
{
	_scriptPath = route.fsPath;
	size_t pyPos = _scriptPath.find(".py/");
	if (pyPos != string::npos)
		_scriptPath = _scriptPath.substr(0, pyPos + 3);
	_requestBody = request.getBody();
	_clientFd = client.getClientFd();
	_interpreter = route.interpreter;
	populateEnv(request, server, client);
}

void CGI::populateEnv(Request &request, const ServerConfig &server, Client& client)
{
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	//contains extra path info after the script name and before '?', '&' for example: 
	//PATH_INFO = /users/42 when request is /cgi-bin/app.py/users/42?mode=edit
	size_t scriptPos = request.getPath().find(".py/");
	if (scriptPos != string::npos)
	{
		_env["SCRIPT_NAME"] = request.getPath().substr(0, scriptPos + 3);
		_env["PATH_INFO"] = request.getPath().substr(scriptPos + 3);
		_env["PATH_TRANSLATED"] = makeAbsolute(server.getRoot() + _env["PATH_INFO"]);
	}
	else
	{
		_env["SCRIPT_NAME"] = request.getPath();
		_env["PATH_INFO"] = "";
		_env["PATH_TRANSLATED"] = "";
	}
	_env["QUERY_STRING"] = request.getQuery();
	_env["REMOTE_ADDR"] = client.getClientAddr();
	_env["SERVER_SOFTWARE"] = "webserv/1.0";
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env["SERVER_NAME"] = server.getServerName().empty() ? server.getHost() : server.getServerName(); 
	_env["SERVER_PORT"] = to_string(server.getPort());
	_env["REQUEST_METHOD"] = request.getMethod();
	_env["CONTENT_TYPE"] = request.getContentType();
	_env["CONTENT_LENGTH"] = request.getBody().empty() ? "0" : to_string(_requestBody.size());
}

int CGI::readOutput()
{
    char buffer[4096];
    ssize_t count = read(_outFd, buffer, sizeof(buffer));

    if (count <= 0)
        return count == 0 ? 0 : -1;

    _output.append(buffer, static_cast<size_t>(count));
    return 1;
}

int	CGI::writeInput()
{
	if (_bodySent == _requestBody.size())
		return 0;
	
	ssize_t bytewritten = write(_inFd, _requestBody.data() + _bodySent, _requestBody.size() - _bodySent);

	if (bytewritten == -1)
	{
		throw runtime_error("write failed");
		return 1; 
	}
	_bodySent += static_cast<size_t>(bytewritten);

    return _bodySent == _requestBody.size() ? 0:1;
}

void CGI::setupCgi(ServerInfo& eloop)
{
	//setup argv and envp
	const string scriptPath = makeAbsolute(_scriptPath);
	const string interpreter = makeAbsolute(_interpreter);
	const string scriptDir = scriptPath.substr(0, scriptPath.rfind('/') + 1);
	char* argv[] = {
		const_cast<char*>(interpreter.c_str()),
		const_cast<char*>(scriptPath.c_str()),
		nullptr
	};
	char** envp = new char*[_env.size() + 1]();
	int i = 0;
	for (auto& entry: _env)
	{
		string s = entry.first + "=" + entry.second;
		envp[i] = new char[s.size() + 1];
		strcpy(envp[i], s.c_str());
		i++;
	}
	envp[i] = nullptr;
	
	//setup pipe
	int inputFd[2];
	int outputFd[2];
	if (pipe(inputFd) == -1)//Pipe() always produce two descriptors
		throw runtime_error("input pipe creation failed");
	if (pipe(outputFd) == -1)
	{
		close(inputFd[0]);
   		close(inputFd[1]);
    	throw runtime_error("output pipe creation failed");
	}

	//fork the process
	_pid = fork();
	if (_pid == -1)
	{
		close(inputFd[0]);
		close(inputFd[1]);
		close(outputFd[0]);
		close(outputFd[1]);
		throw runtime_error("fork failed.");
	}

	if (_pid == 0)//child process
	{
		eloop.closeAllFds();
		close(inputFd[1]);
		close(outputFd[0]);
		if (dup2(inputFd[0], STDIN_FILENO) == -1 || dup2(outputFd[1], STDOUT_FILENO) == -1)
		{
			perror("dup2 failed");
			_exit(1);
		}
		
		close(inputFd[0]);
		close(outputFd[1]);

		if (chdir(scriptDir.c_str()) == -1)
		{
		    perror("chdir failed");
		    _exit(1);
		}

		if (execve(interpreter.c_str(), argv, envp) == -1)
		{
			perror("execve failed");
			_exit(127);
		}
	}
	else //parent process
	{
		close(inputFd[0]);
		close(outputFd[1]);
		_inFd = inputFd[1];
		_outFd = outputFd[0];

		if (_requestBody.empty())
		{
			close(_inFd);
			_inFd = -1;
		}
		//set in and out fd to be non-blocking
		if (_inFd != -1)
		{
			int inFlag = fcntl(_inFd, F_GETFL);
			if (inFlag == -1)
				throw runtime_error("inFd fcntl F_GETFL failed");
			if (fcntl(_inFd, F_SETFL, inFlag | O_NONBLOCK) == -1) 
				throw runtime_error("inFd fcntl F_SETFL failed");
		}
		int outFlag = fcntl(_outFd, F_GETFL);
		if (outFlag == -1)
			throw runtime_error("fcntl F_GETFL failed");
		if (fcntl(_outFd, F_SETFL, outFlag | O_NONBLOCK) == -1)
			throw runtime_error("outFd fcntl F_SETFL failed");
	}
}

int CGI::getInFd()
{
	return _inFd;
}

int CGI::getOutFd()
{
	return _outFd;
}

void CGI::closeIn()
{
	close(_inFd);
}

void CGI::closeOut()
{
	close(_outFd);
}

string makeAbsolute(const string& path)
{
	if (path[0] == '/')//its already /home, so it returns the path as it is
        return path;
	char buffer[PATH_MAX];
	if (getcwd(buffer, sizeof(buffer)) == NULL)
		throw runtime_error("getcwd fails");
	string cwd = buffer;
	if (cwd.back() != '/')
		cwd += '/';
	return cwd + path;
}

string CGI::buildHttpResponse()
{
	string prefix = "Content-Type: ";
	size_t headersEnd = _output.find("\r\n\r\n");
	size_t firstLineEnd = _output.find("\r\n");

	/*
	1. headersEnd == string::npos means .find("\r\n\r\n") 
	couldn’t find the blank line separating the CGI headers from the body.
	2. _output.compare(0, prefix.size(), prefix) != 0 means the output doesn’t start with the exact prefix "Content-Type: ". 
	The 0 says to start comparing at the first character, and compare() returns 0 when the text matches.
	3. firstLineEnd <= prefix.size() means there’s no room for a value after the prefix. For example, 
	"Content-Type: \r\n" matches the prefix, but the line ends exactly where text/plain would start.
	*/
	if (headersEnd == string::npos || _output.compare(0, prefix.size(), prefix) != 0 ||
        firstLineEnd <= prefix.size())
    {
        return "HTTP/1.1 502 Bad Gateway\r\n"
               "Content-Length: 0\r\n"
               "Connection: close\r\n\r\n";
    }

    string contentType = _output.substr(prefix.size(), firstLineEnd - prefix.size());
    string body = _output.substr(headersEnd + 4);

    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: " + contentType + "\r\n" +
           "Content-Length: " + to_string(body.size()) + "\r\n" +
           "Connection: close\r\n\r\n" + body;
}
