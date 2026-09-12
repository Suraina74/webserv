#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
#include <poll.h>

class Client{
	private:
		int		fd;
		Request		request;
		Response	response;
	public:
		Client(int filedesc, const Request& req, const Response& resp) : fd(filedesc), request(req), response(resp){}
		~Client(){}

		int getFd();
		Request& getRequest();
		Response& getResponse();

};