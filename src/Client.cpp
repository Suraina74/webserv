#include "../inc/Client.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"

int Client::getFd(){
	return fd;
}

Request& Client::getRequest(){
	return request;
}

Response& Client::getResponse(){
	return response;
}