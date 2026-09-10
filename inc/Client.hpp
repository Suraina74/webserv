#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
#include <poll.h>

class Client{
	private:
		pollfd		fd;
		Request		request;
		Response	response;
	public:
		Client(){}
		~Client(){}
};