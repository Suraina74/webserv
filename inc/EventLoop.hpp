#pragma once
#include "Server.hpp"

class EventLoop 
{
	private:
		// struct EventLoopfd pfds;
	public:
		EventLoop();
		EventLoop(const EventLoop&);
		EventLoop& operator=(const EventLoop&);
		~EventLoop();
};