#include "../inc/EventLoop.hpp"

EventLoop::EventLoop()
{
	this->fds.events = POLLIN;
	this->fds.fd = 0;
	this->nfds = 1;
	this->timeout = 100;
}

EventLoop::EventLoop(const EventLoop&)
{
	//
}

EventLoop&	EventLoop::operator=(const EventLoop&)
{
	//
}

EventLoop::~EventLoop()
{
	//
}

struct pollfd	EventLoop::getPollfd()
{

}

void	EventLoop::setPollfd(struct pollfd fdsNew)
{

}

nfds_t	EventLoop::getNfds()
{

}

void	EventLoop::setNfds(nfds_t nfdsNew)
{

}

int	EventLoop::getTimeout()
{

}

void	EventLoop::setTimeout(int timeoutNew)
{

}