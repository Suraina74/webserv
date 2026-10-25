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

void Client::actionsRequestLeft(size_t startNextRequest){
	string reqLeft = _request.getFullRequest().substr(startNextRequest, _request.getFullRequest().length() - startNextRequest);
	_request.setRequestLeft(reqLeft);
	_request.resetParserVars();
	_response.resetResponseVars();
	_request.setRequest(reqLeft);
	_request.setBytesRead(reqLeft.length());
	_request.setHeaderBytes(0);
	_request.setParserState(REQUESTLINE);
}

void Client::transferEncodingRequest(){
	size_t indexLastChunk = _request.getFullRequest().find("0\r\n\r\n");
	if (indexLastChunk != string::npos){
		_request.parseBody();
		if (_request.getParserState() == INCOMPLETE){
			return;
		}
		// Route route{};
		// processRoute(*this, eloop, route);
		_request.postAndDelete();
		_response.setRequest(_request);
		_response.composeResponse();
		_responsesString += _response.getFullResponse();
		if (_request.getFullRequest().length() > indexLastChunk + 5){
			size_t startNextRequest = indexLastChunk + 5;
			actionsRequestLeft(startNextRequest);
			return;
		}
		_request.setRequest("");
		_request.setParserState(COMPLETE);
		return;
	}
	else if (indexLastChunk == string::npos){
		_request.setParserState(BODY);
		return;
	}
}

void Client::contentLengthRequest(size_t delimiter, ssize_t n){
	(void)n;
	if (_request.getContentLength())
	{
		size_t startBody = delimiter + 4;
		size_t endBody = _request.getFullRequest().length();
		size_t bodySize = endBody - startBody;
		if (_request.getContentLength() > bodySize)
		{
			// if (n == 0)
			// {
			// 	_request.setParserState(COMPLETE);
			// 	return;
			// }
			_request.setParserState(BODY);
			return;
		}
	}
	size_t expectedBytes = _request.getHeaderBytes() + _request.getContentLength();
	if (_request.getBytesRead() >= expectedBytes)
	{
		_request.parseBody();
		_request.postAndDelete();
		// Route route{};
		// processRoute(*this, eloop, route);
		_response.setRequest(_request);
		_response.composeResponse();
		_responsesString += _response.getFullResponse();
		if (_request.getBytesRead() > expectedBytes)
		{
			size_t startBody = delimiter + 4;
			size_t startNextRequest = startBody + _request.getContentLength();
			actionsRequestLeft(startNextRequest);
			return;
		}
		_request.setRequest("");
		_request.setParserState(COMPLETE);
		return;
	}
}

void Client::handleHeaders(ssize_t n, size_t delimiter){
	(void)n;
	if (delimiter == string::npos)
	{
		// if (n == 0)
		// {
		// 	_request.setParserState(COMPLETE);
		// 	return;
		// }
		_request.setParserState(HEADERS);
		return;
	}
	else if (_request.getHeaderBytes() == 0)
	{
		// delimiter found
		if ((_request.parseUntilHeaders(_request.getFullRequest())) == false)
		{
			_response.setRequest(_request);
			_response.composeResponse();
			_responsesString += _response.getFullResponse();
			_request.setParserState(COMPLETE);
			return;
		}
		_request.setHeaderBytes(_request.getRequestTillHeaders().size());
		_request.setParserState(BODY);
	}
}

void Client::handleRequestLine(){
	if ((_request.parseRequestLine(_request.getFullRequest())) == false){
		_response.setRequest(_request);
		_response.composeResponse();
		_responsesString += _response.getFullResponse();
		_request.setParserState(COMPLETE);
		return;
	}
	if (_request.getParserState() == INCOMPLETE){
		_request.setParserState(REQUESTLINE);
		return;
	}
	_request.setParserState(HEADERS);
}

void Client::receiveRequest(ServerInfo &eloop)
{
	(void)eloop;
	char buffer[2048];
	ssize_t n = recv(_fd, buffer, sizeof(buffer), 0);
	if (n == -1)
	{
		::perror("recv");
		_request.setParserState(ERROR);
		return ;
	}
	else if (n == 0 && _request.getRequestLeft().empty())
	{
		_request.setParserState(ERROR);
		return;
	}
	// _request.setRequestLeft("");
	if (n != 0)
	{
		_request.setBytesRead(_request.getBytesRead() + n);
		string part(buffer, n);
		_request.setRequest(_request.getFullRequest() + part);
	}
	while (true){
		if (_request.getParserState() == REQUESTLINE){
			handleRequestLine();
			if (_request.getParserState() == REQUESTLINE){
				return;
			}
		}
		size_t delimiter = _request.getFullRequest().find("\r\n\r\n");
		if (_request.getParserState() == HEADERS){
			handleHeaders(n, delimiter);
			if (_request.getParserState() == HEADERS){
				return;
			}
		}
		if (_request.getParserState() == BODY){
			if (_request.getChunked() == true){
				transferEncodingRequest();
			}
			else{
				contentLengthRequest(delimiter, n);
			}
			if (_request.getParserState() == BODY){
				return;
			}
		}
		if (_request.getParserState() == COMPLETE){
			return;
		}
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

