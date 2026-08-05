#pragma once
#include "Server.hpp"

class EventLoop 
{
	public:
		vector<pollfd>	fds;
		// nfds_t			nfds;
		int				timeout;
};