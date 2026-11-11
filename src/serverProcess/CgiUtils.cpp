#include "../../inc/CGI.hpp"
#include <unistd.h> // where getcwd() is from
#include <limits.h> // where PATH_MAX is from
#include <stdexcept> 

string makeAbsolute(const string& path)
{
	if (path[0] == '/')//its already /home
        return path;
	char buffer[PATH_MAX];
	if (getcwd(buffer, sizeof(buffer)) == NULL)
		throw runtime_error("getcwd fails");
	string cwd = buffer;
	if (cwd.back() != '/')
		cwd += '/';
	return cwd + path;
}