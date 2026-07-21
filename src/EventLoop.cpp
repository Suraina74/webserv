#include "../inc/EventLoop.hpp"

EventLoop::EventLoop()
{
	this->fds.fd = 0;
	this->fds.events = POLLIN;
	this->nfds = 1;
	this->timeout = 100;
}

EventLoop::EventLoop(const EventLoop& value)
{
	*this = value;
}

EventLoop&	EventLoop::operator=(const EventLoop& value)
{
	if (this != &value)
	{
		fds.fd = value.fds.fd;
		fds.events = value.fds.events;
		nfds = value.nfds;
		timeout = value.timeout;
	}
	return (*this);
}

// EventLoop::~EventLoop()
// {}

struct pollfd	EventLoop::getPollfd()
{
	return (fds);
}

void	EventLoop::setPollfd(struct pollfd fdsNew)
{
	fds = fdsNew;
}

nfds_t	EventLoop::getNfds()
{
	return (nfds);
}

void	EventLoop::setNfds(nfds_t nfdsNew)
{
	nfds = nfdsNew;
}

int	EventLoop::getTimeout()
{
	return (timeout);
}

void	EventLoop::setTimeout(int timeoutNew)
{
	timeout = timeoutNew;
}