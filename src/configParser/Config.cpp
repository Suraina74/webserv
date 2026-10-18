#include "../../inc/Config.hpp"

Config::Config():_servers(){}

Config::~Config(){}

void Config::parseLocLine(string& line, Location& loc, int lineNum)
{
	istringstream iss(line);
	string key, val, extra, rest;

	if (!(iss >> key))
		throw runtime_error("Line " + to_string(lineNum) + ": Missing argument.");
	if (key == "methods" || key == "cgi")
	{
		getline(iss, rest);
		cleanLine(rest);
		if (key == "methods")
			parseMethods(rest, loc, lineNum);
		else
			parseCGI(rest, loc, lineNum);
		return;
	}
	if (key == "location")
		throw runtime_error("Line " + to_string(lineNum) + ": nested location blocks are not allowed.");
	if (!(iss >> val))
		throw runtime_error("Line " + to_string(lineNum) + ": Missing argument.");
	if (iss >> extra)
		throw runtime_error("Line " + to_string(lineNum) + ": Too many arguments.");
	if (key == "root")
		parseLocRoot(val, loc, lineNum);
	else if (key == "index")
		parseLocIndex(val, loc, lineNum);
	else if (key == "upload_path")
		parseUploadPath(val, loc, lineNum);
	else if (key == "upload")
		parseUpload(val, loc, lineNum);
	else if (key == "redirect")
		parseRedirect(val, loc, lineNum);
	else if (key == "directory_listing")
		parseDirectoryListing(val, loc, lineNum);
	else
		throw runtime_error("Line " + to_string(lineNum) + ": Unknown directive '" + key + "'.");
}

void Config::parseLocation(ifstream& configFile, string& line, ServerConfig& server, int& lineNum)
{
	bool closed = false;
	Location loc = Location();
	set<string> seen;
	int locLine = lineNum;

	//header line: "location <path>"
	istringstream iss(line);
	string key, path, extra;
	if (!(iss >> key >> path))
		throw runtime_error("Line " + to_string(lineNum) + ": Missing argument.");
	if (iss >> extra)
		throw runtime_error("Line " + to_string(lineNum) + ": Too many arguments.");
	parseLocPath(path, loc, lineNum);
	const vector<Location>& locations = server.getLocations();
	for (size_t i = 0; i < locations.size(); i++)
	{
		if (locations[i].getPath() == loc.getPath())
			throw runtime_error("Line " + to_string(lineNum) + ": duplicate location '" + loc.getPath() + "'.");
	}
	if (!getline(configFile, line))
		throw runtime_error("Line " + to_string(lineNum) + ": Unexpected end of file.");
	lineNum++;
	cleanLine(line);
	if (line != "{")
	{
		throw runtime_error("Line " + to_string(lineNum) + ": Expected '{'.");
	}
	while (getline(configFile, line))
	{
		lineNum++;
		cleanLine(line);
		if (line.empty())
			continue;
		if (line == "}")
		{
			closed = true;
			break;
		}
		checkDuplicate(line, seen, lineNum);
		parseLocLine(line, loc, lineNum);
	}
	if (!closed)
		throw runtime_error("Line " + to_string(lineNum) + ": missing closing '}'.");
	if (loc.getUploadEnabled() && loc.getUploadPath().empty())
		throw runtime_error("Line " + to_string(locLine) + ": 'upload on' requires 'upload_path'.");
	server.addLocation(loc);
}

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

//runs after the whole server block is read, so directive order does not matter
void Config::validateServer(ServerConfig& server, const set<string>& seen, int serverLine)
{
	if (!seen.count("listen"))
		throw runtime_error("Line " + to_string(serverLine) + ": server block is missing 'listen'.");
	if (!seen.count("root"))
		throw runtime_error("Line " + to_string(serverLine) + ": server block is missing 'root'.");
	if (!seen.count("host"))
		server.setHost("0.0.0.0");//listen on all interfaces, like nginx
	if (!server.getIndex().empty())
	{
		string path = server.getRoot() + '/' + server.getIndex();
		struct stat info;
		if (stat(path.c_str(), &info) != 0)
			throw runtime_error("Line " + to_string(serverLine) + ": index file '" + path + "' does not exist.");
		if (!S_ISREG(info.st_mode))
			throw runtime_error("Line " + to_string(serverLine) + ": index '" + path + "' is not a regular file.");
	}
}

void Config::parseServer(ifstream& configFile, int& lineNum, bool& foundServer)
{
	bool closed = false;
	string line;
	ServerConfig newServer = ServerConfig();
	set<string> seen;
	int serverLine = lineNum;

	if (!getline(configFile, line))
		throw runtime_error("Line " + to_string(lineNum) + ": Unexpected end of file.");
	lineNum++;
	cleanLine(line);
	if (line != "{")
	{
		throw runtime_error("Line " + to_string(lineNum) + ": Expected '{'.");
	}
	while (getline(configFile, line))
	{
		lineNum++;
		cleanLine(line);
		if (line.empty())
			continue;
		if (line == "}")
		{
			closed = true;
			break;
		}
		//any line whose first word is "location", so "location a" gets a proper path error
		string firstWord;
		istringstream(line) >> firstWord;
		if (firstWord == "location")
		{
			parseLocation(configFile, line, newServer, lineNum);
			continue;
		}
		checkDuplicate(line, seen, lineNum);
		parseLine(line, newServer, lineNum);
	}
	if (!closed)
		throw runtime_error("Line " + to_string(lineNum) + ": missing closing '}'.");
	validateServer(newServer, seen, serverLine);
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
		lineNum++;
		cleanLine(line);
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

const vector<ServerConfig>& Config::getServers() const
{
	return _servers;
}
