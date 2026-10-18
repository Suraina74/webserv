#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
#include <poll.h>

class Client{
	private:
		int			fd;
		Request		request;
		Response	response;
		std::string responsesString{};
	public:
		Client(int filedesc, const Request& req, const Response& resp) : fd(filedesc), request(req), response(resp){}
		~Client(){}

		int getFd();
		Request& getRequest();
		Response& getResponse();
		std::string& getFullResponse();

		int receiveRequest();
		int sendResponse();
};