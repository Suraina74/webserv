#include "../../inc/CGI.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
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
	_scriptPath = request.getPath();
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
	size_t scriptPos = _scriptPath.find(".py/");
	if (scriptPos != string::npos)
	{
		_env["SCRIPT_NAME"] = _scriptPath.substr(0, scriptPos + 3);
		_env["PATH_INFO"] = _scriptPath.substr(scriptPos + 3);
		_env["PATH_TRANSLATED"] = makeAbsolute(server.getRoot() + _env["PATH_INFO"]);
	}
	else
	{
		_env["SCRIPT_NAME"] = _scriptPath;
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
	char	buffer[4096];
	ssize_t	byteRead = 0;
	while (true)
	{
		byteRead = read(_outFd, buffer, sizeof(buffer));
		if (byteRead > 0)
			_output.append(buffer, byteRead);
		else if (byteRead == 0)
			return 0;
		else
		{
			if (errno == EINTR)
				continue;//ignore and continue to read
			else if (errno == EAGAIN || errno == EWOULDBLOCK)
				return 1;//signal to read later
			else
				return -1; //read fail
		}
	}
}

int	CGI::writeInput()
{
	ssize_t bytewritten = 0;
	while (_bodySent < _requestBody.size())
	{
		bytewritten = write(_inFd, _requestBody.data() + _bodySent, _requestBody.size() - _bodySent);
		if (bytewritten > 0)
			_bodySent += bytewritten;
		else if (bytewritten == -1)
		{
			if (errno == EINTR)
				continue;
			else if (errno == EAGAIN || errno == EWOULDBLOCK)
				return 1;
			else
				return -1;
		}
		else
			return -1;
	}
	if (_inFd != -1)
    {
        close(_inFd);
        _inFd = -1;
    }

    return 0;
}

int CGI::setupCgi(ServerInfo& eloop)
{
	//setup argv and envp
	char* argv[] = {const_cast<char*>(_interpreter.c_str()), const_cast<char*>(_scriptPath.c_str()), nullptr};
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

	if (_pid == 0)
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

		if (execve(_interpreter.c_str(), argv, envp) == -1)
		{
			perror("execve failed");
			_exit(127);
		}
	}
	else
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