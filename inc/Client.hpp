#pragma once 
#include "Server.hpp"

class Client 
{
	private:
		//client socket	
	public:
		Client();
		Client(const Client& other);
		Client& operator=(const Client& other);
		~Client();
};