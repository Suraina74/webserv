#pragma once
#include "Server.hpp"

class EventLoop 
{
	public:
		vector<pollfd>	fds;
		int				timeout;
};