#include "../../inc/Client.hpp"
#include "../../inc/Request.hpp"
#include "../../inc/Response.hpp"
#include "../../inc/main.hpp"
#include <sys/socket.h>
#include <cstdio>

Client::Client():_fd(-1), _server(NULL){}

Client::Client(int fd, const ServerConfig *server, Request request, Response response):_fd(fd), _server(server), _request(request),_response(response){}

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

// Change Suraina
int Client::receiveRequest(ServerInfo &eloop)
{
	char buffer[2048];
	ssize_t n = recv(_fd, buffer, sizeof(buffer), 0);
	if (n == -1)
	{
		::perror("recv");
		return PARSER_ERROR;
	}
	// Change Suraina
	else if (n == 0 && _request.getFullRequest().empty())
	{
		return 0;
	}
	if (n != 0)
	{
		_request.setBytesRead(_request.getBytesRead() + n);
		string part(buffer, n);
		_request.setRequest(_request.getFullRequest() + part);
	}
	// Change Suraina
	size_t delimiter = _request.getFullRequest().find("\r\n\r\n");
	if (delimiter == string::npos)
	{
		if (n == 0)
		{
			return RESPONSE_READY;
		}
		return INCOMPLETE_HEADERS;
	}
	else if (_request.getHeaderBytes() == 0)
	{
		if ((_request.parseUntilHeaders(_request.getFullRequest())) == false)
		{
			_response.setRequest(_request);
			_response.composeResponse();
			_responsesString += _response.getFullResponse();
			return RESPONSE_READY;
		}
		_request.setHeaderBytes(_request.getRequestTillHeaders().size());
	}
	// Change Suraina
	if (_request.getContentLength())
	{
		size_t startBody = delimiter + 4;
		size_t endBody = _request.getFullRequest().length();
		size_t bodySize = endBody - startBody;
		if (bodySize < _request.getContentLength())
		{
			if (n == 0)
			{
				return RESPONSE_READY;
			}
			return BODY_INCOMPLETE;
		}
	}
	size_t expectedBytes = _request.getHeaderBytes() + _request.getContentLength();
	if (_request.getChunked() == true && _request.getFullRequest().find("0\r\n\r\n") != string::npos)
	{
		_request.parseBody();
		Route route{};
		processRoute(*this, eloop, route);
		_response.setRequest(_request);
		_response.composeResponse();
		_responsesString = _response.getFullResponse();
		return RESPONSE_READY;
	}
	// Change Suraina
	else if (_request.getBytesRead() >= expectedBytes)
	{
		_request.parseBody();
		Route route{};
		processRoute(*this, eloop, route);
		_response.setRequest(_request);
		_response.composeResponse();
		_responsesString += _response.getFullResponse();
		if (_request.getBytesRead() > expectedBytes)
		{
			size_t startBody = delimiter + 4;
			size_t endBody = startBody + _request.getContentLength();
			std::string requestLeft = _request.getFullRequest().substr(endBody, _request.getFullRequest().length() - endBody);
			_request.resetParserVars();
			_response.resetResponseVars();
			_request.setRequest(requestLeft);
			_request.setBytesRead(requestLeft.length());
			_request.setHeaderBytes(0);
			return REQUEST_READY;
		}
		_request.setRequest("");
		return RESPONSE_READY;
	}
	return 1;
}

// Change Suraina
int Client::sendResponse()
{
	string fullResponse = _responsesString;
	_response.setLenResponse(fullResponse.length());
	_response.setCString(fullResponse.c_str());
	if (_response.getBytesSent() < _response.getLenResponse())
	{
		int n = send(_fd, _response.getCFullResponse() + _response.getBytesSent(), _response.getLenResponse() - _response.getBytesSent(), 0);
		if (n == -1)
		{
			::perror("send");
			return -1;
		}
		else if (n == 0)
		{
			return 0;
		}
		_response.setBytesSent(_response.getBytesSent() + n);
	}
	if (_response.getBytesSent() == _response.getLenResponse())
	{
		return 2;
	}
	return 1;
}

