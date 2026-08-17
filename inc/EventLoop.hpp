#pragma once
#include "Server.hpp"

using namespace std;

class EventLoop 
{
	public:
		struct pollfd	fds;
		nfds_t			nfds; //the number of fds you give to poll()
		int				timeout; //tells poll() how long it should wait for an event
};