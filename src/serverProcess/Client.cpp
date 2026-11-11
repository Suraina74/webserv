#include "../../inc/Client.hpp"
#include <sys/socket.h>
#include <cstdio>
#include <iostream>

Client::Client():_fd(-1), _server(NULL){}

Client::Client(int fd, const ServerConfig *server, Request request, Response response, string& remoteAddr):_fd(fd), _server(server), _request(request),_response(response), _remoteAddr(remoteAddr) {}

Client::~Client(){}

int Client::getClientFd()
{
	return _fd;
}

Request& Client::getRequest() 
{
	return _request;
}

const ServerConfig&	Client::getClientServer()
{
	return *_server;
}

Response& Client::getResponse()
{
	return _response;
}

string& Client::getFullResponse()
{
	return _responsesString;
}

string& Client::getClientAddr()
{
	return _remoteAddr;
}

int	Client::getFd()
{
    return _fd;
}

int	Client::timer(chrono::steady_clock::time_point time_cur, chrono::steady_clock::time_point time_after_action)
{
	auto time_to_check = time_cur - time_after_action;

	if (time_to_check > chrono::seconds(TIMEOUT))
	{
		cout << "max timeout reached, client " << _fd << " kicked out" << endl;
		//set 408
		return (1);
	}
	return (0);
}

void Client::actionsRequestLeft(size_t startNextRequest)
{
	string reqLeft = _request.getFullRequest().substr(startNextRequest, _request.getFullRequest().length() - startNextRequest);
	_request.resetParserVars();
	_response.resetResponseVars();
	_request.setRequest(reqLeft);
	_request.setBytesRead(reqLeft.length());
	_request.setHeaderBytes(0);
	_request.setParserState(REQUESTLINE);
}

void Client::transferEncodingRequest(ServerInfo &eloop, const ServerConfig& server)
{
	size_t indexLastChunk = _request.getFullRequest().find("0\r\n\r\n");
	if (indexLastChunk != string::npos)
	{
		_request.parseBody();
		if (_request.getParserState() == INCOMPLETE)
		{
			_request.setParserState(BODY);
			return;
		}
		Route route{};
		processRoute(*this, eloop, route);
		_response.setRequest(_request);
		_response.composeResponse(route.redirectTo, route.fsPath, server);
		_responsesString += _response.getFullResponse();
		if (_request.getFullRequest().length() > indexLastChunk + 5)
		{
			size_t startNextRequest = indexLastChunk + 5;
			actionsRequestLeft(startNextRequest);
			return;
		}
		_request.setParserState(COMPLETE);
		return;
	}
	else if (indexLastChunk == string::npos){
		_request.setParserState(BODY);
		return;
	}
}

void Client::contentLengthRequest(size_t delimiter, ServerInfo &eloop, const ServerConfig& server)
{
	if (_request.getContentLength())
	{
		size_t startBody = delimiter + 4;
		size_t endBody = _request.getFullRequest().length();
		size_t bodySize = endBody - startBody;
		if (_request.getContentLength() > bodySize)
		{
			_request.setParserState(BODY);
			return;
		}
	}
	size_t expectedBytes = _request.getHeaderBytes() + _request.getContentLength();
	if (_request.getBytesRead() >= expectedBytes)
	{
		_request.parseBody();
		Route route{};
		processRoute(*this, eloop, route);
		_response.setRequest(_request);
		_response.composeResponse(route.redirectTo, route.fsPath, server);
		_responsesString += _response.getFullResponse();
		if (_request.getBytesRead() > expectedBytes)
		{
			size_t startBody = delimiter + 4;
			size_t startNextRequest = startBody + _request.getContentLength();
			actionsRequestLeft(startNextRequest);
			return;
		}
		_request.setParserState(COMPLETE);
		return;
	}
}

void Client::handleHeaders(size_t delimiter, ServerInfo &eloop, const ServerConfig& server)
{
	if (delimiter == string::npos)
	{
		_request.setParserState(HEADERS);
		return;
	}
	else if (_request.getHeaderBytes() == 0)
	{
		// delimiter found
		if ((_request.parseUntilHeaders(_request.getFullRequest())) == false)
		{
			Route route{};
			processRoute(*this, eloop, route);
			_response.setRequest(_request);
			_response.composeResponse(route.redirectTo, {}, server);
			_responsesString += _response.getFullResponse();
			_request.setParserState(COMPLETE);
			return;
		}
		_request.setHeaderBytes(_request.getRequestTillHeaders().size());
		_request.setParserState(BODY);
	}
}

void Client::handleRequestLine(ServerInfo &eloop, const ServerConfig& server)
{
	if ((_request.parseRequestLine(_request.getFullRequest())) == false)
	{
		Route route{};
		processRoute(*this, eloop, route);
		_response.setRequest(_request);
		_response.composeResponse(route.redirectTo, {}, server);
		_responsesString += _response.getFullResponse();
		_request.setParserState(COMPLETE);
		return;
	}
	if (_request.getParserState() == INCOMPLETE)
	{
		_request.setParserState(REQUESTLINE);
		return;
	}
	_request.setParserState(HEADERS);
}

void Client::receiveRequest(ServerInfo &eloop)
{
	// Misschien op een andere manier zodat de buffer groter is?
	char buffer[2048];
	const ServerConfig& server = getClientServer();
	ssize_t n = recv(_fd, buffer, sizeof(buffer), 0);//reads incoming data from a the client _fd socket.
	if (n == -1)
	{
		// if on this, there is currently nothing to read
		if (errno == EAGAIN || errno == EWOULDBLOCK){
			return;
		}
		::perror("recv");
		_request.setParserState(ERROR);
		return;
	}
	else if (n == 0)//Double check this logic, what if it is EOF
	{
		_request.setParserState(ERROR);
		return;
	}
	if (n > 0)
	{
		_request.setBytesRead(_request.getBytesRead() + n);
		string part(buffer, n);
		_request.setRequest(_request.getFullRequest() + part);
		_last_active = chrono::steady_clock::now();
	}
	_last_active = chrono::steady_clock::now();
	while (true)
	{
		if (_request.getParserState() == REQUESTLINE)
		{
			handleRequestLine(eloop, server);
			if (_request.getParserState() == REQUESTLINE)
				return;
		}
		size_t delimiter = _request.getFullRequest().find("\r\n\r\n");
		if (_request.getParserState() == HEADERS)
		{
			handleHeaders(delimiter, eloop, server);
			if (_request.getParserState() == HEADERS)
				return;
		}
		if (_request.getParserState() == BODY)
		{
			if (_request.getChunked() == true)
				transferEncodingRequest(eloop, server);
			else
				contentLengthRequest(delimiter, eloop, server);
			if (_request.getParserState() == BODY)
				return;
		}
		if (_request.getParserState() == COMPLETE)
			return;
	}
	return;
}
// Your sendResponse() must handle partial writes. If it sends only part of fullResponse, keep POLLOUT enabled until all bytes have been sent. 
// Only then switch back to POLLIN or close the connection, depending on the connection policy.

int Client::sendResponse()
{
	_response.setLenResponse(_responsesString.length());
	_response.setCString(_responsesString.c_str());
	if (_response.getBytesSent() < _response.getLenResponse())
	{
		int n = send(_fd, _response.getCFullResponse() + _response.getBytesSent(), _response.getLenResponse() - _response.getBytesSent(), 0);
		if (n == -1)
		{
			// if on this, there is currently nothing to send
			if (errno == EAGAIN || errno == EWOULDBLOCK){
				return 1;
			}
			::perror("send");
			return -1;
		}
		else if (n == 0)
			return 0;
		_last_active = chrono::steady_clock::now();
		_response.setBytesSent(_response.getBytesSent() + n);
	}
	if (_response.getBytesSent() == _response.getLenResponse())
	{
		_last_active = chrono::steady_clock::now();
		return 2;
	}
	return 1;
}

void Client::setTimeout(chrono::steady_clock::time_point newTimeout)
{
	_last_active = newTimeout;
}