#pragma once
#include "Server.hpp"

class Config 
{
	private:
		// struct Configfd pfds;
	public:
		Config();
		Config(const Config&);
		Config& operator=(const Config&);
		~Config();
};