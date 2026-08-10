#include "../inc/Config.hpp"

ServerConfig::ServerConfig():_listen(0), _host(""), _serverName(""), _root(""), _errPage(""), _index("") {}

ServerConfig::~ServerConfig(){}

void ServerConfig::setPort(int port)
{
	_listen = port;
}

void ServerConfig::setHost(const std::string &host)
{
	_host = host;
}

void ServerConfig::setRoot(const std::string &root)
{
	_root = root;
}

void ServerConfig::setIndex(const std::string &index)
{
	_index = index;
}

Config::Config():_servers(){}

Config::~Config(){}


void Config::cleanLine(string& line)
{
	size_t commentPos = line.find('#');
	if (commentPos != string::npos)
		line.erase(commentPos);
	size_t firstPos = line.find_first_not_of(" \t");
	if (firstPos == string::npos)
    {
        line.clear();
        return;
    }

	line.erase(0, firstPos);
	size_t lastPos = line.find_last_not_of(" \t");
		line.erase(lastPos + 1);
}

void verifyNum(string sub, int lineNum)
{
	int octet;
	size_t pos = 0;
	if (sub.empty())
		throw runtime_error("Line " + std::to_string(lineNum) + ": empty octet value.");
	try
	{
		octet = stoi(sub, &pos); 
	}
	catch(const std::exception& e)
	{
    	throw runtime_error("Line " + std::to_string(lineNum) + ": Invalid octet number."); 
	}
	if (octet > 255 || octet < 0)
		throw runtime_error("Line " + std::to_string(lineNum) + ": Invalid octet range."); 
	if (pos != sub.length())
		throw runtime_error("Line " + std::to_string(lineNum) + ": Invalid octet value."); 
}

void Config::parseHost(string& val, ServerConfig& server, int lineNum)
{
	if (val.empty())
		throw runtime_error("Line " + std::to_string(lineNum) + ": empty value.");
	if (val.back() != ';')
    	throw runtime_error("Line " + std::to_string(lineNum) + ": missing ';'");
	val.pop_back();
	string tmp = val;
	int numOfDots = 0;
	while (true)
	{
		size_t pos = tmp.find('.');
		if (pos == string::npos)
        	break;
		string sub = tmp.substr(0, pos);
		verifyNum(sub, lineNum);
		numOfDots++;
    	tmp.erase(0, pos + 1);
	}
	verifyNum(tmp, lineNum);
	if (numOfDots != 3)
		throw runtime_error("Line " + std::to_string(lineNum) + ": wrong amount of '.'.");
	server.setHost(val);
}

void Config::parseRoot(string& val, ServerConfig& server, int lineNum)
{
	if (val.empty())
		throw runtime_error("Line " + std::to_string(lineNum) + ": empty value.");
	if (val.back() != ';')
    	throw runtime_error("Line " + std::to_string(lineNum) + ": Missing ';'");
	val.pop_back();
}

void Config::parseIndex(string& val, ServerConfig& server, int lineNum)
{
	if (val.empty())
		throw runtime_error("Line " + std::to_string(lineNum) + ": empty value.");
	if (val.back() != ';')
    	throw runtime_error("Line " + std::to_string(lineNum) + ": Missing ';'");
	val.pop_back();
}

void Config::parsePort(string& val, ServerConfig& server, int lineNum)
{
	//pop_back() removes the last element
	if (val.empty())
		throw runtime_error("Line " + std::to_string(lineNum) + ": empty value.");
	if (val.back() != ';')
    	throw runtime_error("Line " + std::to_string(lineNum) + ": Missing ';'");
	val.pop_back();
	int port = 0;
	size_t pos = 0;
	try
	{
		port = stoi(val, &pos);
	}
	catch(const std::exception& e)
	{
    	throw runtime_error("Line " + std::to_string(lineNum) + ": Invalid port number"); 
	}
	
    if (port < 1 || port > 65535 || pos != val.size())
        throw runtime_error("Line " + std::to_string(lineNum) + ": invalid port number");
	server.setPort(port);
}

void Config::parseLine(string& line, ServerConfig& server, int lineNum)
{
	istringstream iss(line);
	string		  key;
	string		  val;
	string		  extra;

	iss >> key >> val;

	if (iss >> extra)
    	throw runtime_error("Line " + std::to_string(lineNum) + ": Too many arguments.");
	if (key == "listen")
		parsePort(val, server, lineNum);
	else if (key == "host")
		parseHost(val, server, lineNum);
	else if (key == "root")
		parseRoot(val, server, lineNum);
	else if (key == "index")
		parseIndex(val, server, lineNum);
	else
		throw std::runtime_error("Line " + std::to_string(lineNum) + ": Unknown directive: " + key);
}

void Config::parseServer(ifstream& configFile, int& lineNum)
{
	bool closed = false;
	string line;
	ServerConfig server = ServerConfig();
	if (!getline(configFile, line))
    	throw runtime_error("Line " + std::to_string(lineNum) + ": Unexpected end of file.");
	
	cleanLine(line);
	lineNum++;

    if (line != "{")
    	throw runtime_error("Line " + std::to_string(lineNum) + ": Expected {");
	while (getline(configFile, line))
	{
		cleanLine(line);
		lineNum++;
		if (line.empty())
    		continue;
		if (line == "}")
		{
			closed = true;
			break;
		}
		parseLine(line, server, lineNum);
	}
	if (!closed)
    	throw runtime_error("Line " + std::to_string(lineNum) + ": missing closing '}'");

	//vector as a class member is automatically constructed and can be used immediately
	//push_back(x) add x to the end of the list
	_servers.push_back(server);
}

void Config::parse(const std::string &filename)
{
	ifstream configFile(filename);
	string line;
	int lineNum = 0;

	if(!configFile.is_open())
		throw runtime_error("Line " + std::to_string(lineNum) + ": Open config file failed.");

	while(getline(configFile, line))
	{
		cleanLine(line);
		lineNum++;
		if (line.empty())
			continue;
		if (line == "server")
			parseServer(configFile, lineNum);

		else
    		throw runtime_error("Line " + std::to_string(lineNum) + ": Expected 'server'");
		cout << line << endl;
	}
}

const std::vector<ServerConfig>& Config::getServers() const
{
	return _servers;
}