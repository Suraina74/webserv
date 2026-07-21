#pragma once
#include "Server.hpp"

class EventLoop 
{
	private:
		struct pollfd	fds;
		nfds_t			nfds;
		int				timeout;
	public:
		EventLoop();
		EventLoop(const EventLoop&);
		EventLoop& operator=(const EventLoop&);
		~EventLoop();
		struct pollfd	getPollfd();
		void			setPollfd(struct pollfd fdsNew);
		nfds_t			getNfds();
		void			setNfds(nfds_t nfdsNew);
		int				getTimeout();
		void			setTimeout(int timeoutNew);

};