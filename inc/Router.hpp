#pragma once 
#include "Server.hpp"

class Router 
{
	private:
		std::string	status_line;
		std::string	*body;
		//hashtable var voor headers
	public:
		Router();
		Router(const Router& other);
		Router operator=(const Router& other);
		~Router();
};
