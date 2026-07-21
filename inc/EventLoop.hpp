#pragma once
#include "Server.hpp"

class EventLoop 
{
	public:
		struct pollfd	fds;
		nfds_t			nfds;
		int				timeout;
		struct pollfd	getPollfd();
		void			setPollfd(struct pollfd fdsNew);
		nfds_t			getNfds();
		void			setNfds(nfds_t nfdsNew);
		int				getTimeout();
		void			setTimeout(int timeoutNew);
};