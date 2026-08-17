#pragma once
#include <poll.h>
#include <vector>
#include <iostream>

using namespace std;

class EventLoop 
{
	public:
		vector<pollfd>	fds;
		int				timeout;
};