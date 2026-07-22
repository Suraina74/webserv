#pragma once
#include "Server.hpp"

class EventLoop 
{
	public:
		struct pollfd	fds;
		nfds_t			nfds;
		int				timeout;
};