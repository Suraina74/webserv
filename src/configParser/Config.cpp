#include "Config.hpp"

Config::Config():_servers(){}

Config::~Config(){}

const std::vector<ServerConfig>& Config::getServers() const
{
	return _servers;
}

// void Config::parseLocLine(string& line, Location& loc, int lineNum)
// {
// 	istringstream iss(line);
//     string key, val, extra, rest;

//     if (!(iss >> key >> val))
//         throw runtime_error("Line " + to_string(lineNum) + ": Missing argument.");
//     if (key == "methods" || key == "cgi")
//     {
//         getline(iss, rest);
//         cleanLine(rest);
// 		if (key == "methods")
//         	parseMethods(rest, loc, lineNum);
// 		parseCGI(rest, loc, lineNum);
// 		return;
//     }
//     if (iss >> extra)
//         throw runtime_error("Line " + to_string(lineNum) + ": Too many arguments.");
//     if (key == "location")
//         parseLocPath(val, loc, lineNum);
// 	else if (key == "root")
//         parseLocRoot(val, loc, lineNum);
//     else if (key == "index")
//         parseLocIndex(val, loc, lineNum);
//     else if (key == "upload_path")
//         parseUploadPath(val, loc, lineNum);
//     else if (key == "upload")
//         parseUpload(val, loc, lineNum);
//     else if (key == "redirect")
//         parseRedirect(val, loc, lineNum);
//     else if (key == "directory_listing")
//         parseDirectoryListing(val, loc, lineNum);
//     else
//         throw runtime_error("Line " + to_string(lineNum) + ": Unknown directive '" + key + "'.");
// }

// void Config::parseLocation(ifstream& configFile, string& line, ServerConfig& server, int lineNum)
// {
// 	bool closed = false;
// 	Location loc = Location();
	
// 	parseLocLine(line, loc, lineNum);
// 	if (!getline(configFile, line))
//     	throw runtime_error("Line " + to_string(lineNum) + ": Unexpected end of file.");
// 	cleanLine(line);
// 	lineNum++;
//     if (line != "{")
// 	{
//     	throw runtime_error("Line " + to_string(lineNum) + ": Expected '{'.");
// 	}
// 	while (getline(configFile, line))
// 	{
// 		if (line.empty())
// 			continue;
// 		cleanLine(line);
// 		lineNum++;
// 		if (line == "}")
// 		{
// 			closed = true;
// 			break;
// 		}
// 		parseLocLine(line, loc, lineNum);
// 	}
// 	if (!closed)
//     	throw runtime_error("Line " + to_string(lineNum) + ": missing closing '}'.");
// 	server.addLocation(loc);
// }

void Config::parseLine(string& line, ServerConfig& server, int lineNum)
{
	istringstream iss(line);
    string key, val, extra, rest;

    if (!(iss >> key))
        throw runtime_error("Line " + to_string(lineNum) + ": Missing argument.");
    if (key == "error_page")
    {
        getline(iss, rest);
        cleanLine(rest);
		if (rest.empty())
            throw runtime_error("Line " + to_string(lineNum) + ": error_page requires a code and a path.");
        parseErr(rest, server, lineNum);
		return;
    }
	if (!(iss >> val))
        throw runtime_error("Line " + to_string(lineNum) + ": Missing argument.");
    if (iss >> extra)
        throw runtime_error("Line " + to_string(lineNum) + ": Too many arguments.");
    if (key == "listen")
        parsePort(val, server, lineNum);
    else if (key == "host")
        parseHost(val, server, lineNum);
    else if (key == "root")
        parseRoot(val, server, lineNum);
    else if (key == "index")
        parseIndex(val, server, lineNum);
    else if (key == "client_max_body_size")
        parseBodySize(val, server, lineNum);
    else if (key == "server_name")
        parseServerName(val, server, lineNum);
    else
        throw runtime_error("Line " + to_string(lineNum) + ": Unknown directive '" + key + "'.");
}

void Config::parseServer(ifstream& configFile, int& lineNum, bool& foundServer)
{
	bool closed = false;
	string line;
	ServerConfig newServer = ServerConfig();

	if (!getline(configFile, line))
    	throw runtime_error("Line " + to_string(lineNum) + ": Unexpected end of file.");
	cleanLine(line);
	lineNum++;
    if (line != "{")
	{
    	throw runtime_error("Line " + to_string(lineNum) + ": Expected '{'.");
	}
	while (getline(configFile, line))
	{
		if (line.empty())
			continue;
		cleanLine(line);
		lineNum++;
		if (line == "}")
		{
			closed = true;
			break;
		}
		// if (line.compare(0, 10, "location /") == 0)
		// {
			// parseLocation(configFile, line, newServer, lineNum);
			// continue;
		// }
		parseLine(line, newServer, lineNum);
	}
	if (!closed)
    	throw runtime_error("Line " + to_string(lineNum) + ": missing closing '}'.");
	//vector as a class member is automatically constructed and can be used immediately
	//push_back(x) add x to the end of the list
	foundServer = false;
	_servers.push_back(newServer);
}

void Config::parse(const string &filename)
{
	ifstream	configFile(filename);
	string		line;
	int			lineNum = 0;
	bool		foundServer = false;
	int			numServer = 0;

	if(!configFile.is_open())
		throw runtime_error("Line " + to_string(lineNum) + ": Open config file failed.");

	while(getline(configFile, line))
	{
		cleanLine(line);
		lineNum++;
		if (line.empty())
			continue;
		if (line == "server" && foundServer == false)
		{
			foundServer = true;
			parseServer(configFile, lineNum, foundServer);
		}
		else
		{
			throw runtime_error("Line " + to_string(lineNum) + ": Expected 'server'");
		}
		numServer++;
	}
	if (numServer == 0)
    	throw runtime_error("Config file contains no server block.");
}

