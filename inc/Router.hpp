#pragma once 
#include "../src/configParser/Config.hpp"

using namespace std;

class Router 
{
	private:
		string	status_line;
		string	*body;
		//hashtable var voor headers
	public:
		Router();
		Router(const Router& other);
		Router operator=(const Router& other);
		~Router();
};
