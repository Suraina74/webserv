#include "Config.hpp"

void Config::parseBodySize(string& val, ServerConfig& server, int lineNum)
{
	emptyValCheck(val, lineNum);
	size_t i = 0;
	while (i < val.size() && isdigit(static_cast<unsigned char>(val[i])))
		i++;
	if (i == 0 || val.size() - i > 1)
		throw runtime_error("Line " + to_string(lineNum) + ": invalid size value.");
	try
	{
		size_t num = static_cast<size_t>(stoll(val.substr(0, i)));
		size_t multiplier = 1;
		if (val.size() - i == 1)
		{
			switch (toupper(static_cast<unsigned char>(val[i])))
        	{
        	    case 'K': multiplier = 1024; break;
        	    case 'M': multiplier = 1024 * 1024; break;
        	    case 'G': multiplier = 1024 * 1024 * 1024; break;
        	    default:
        	        throw runtime_error("Line " + to_string(lineNum) + ": invalid size unit.");
        	}
		}
		if (num > numeric_limits<size_t>::max()/multiplier)
			throw runtime_error("Line " + to_string(lineNum) + ": size value too large.");
		server.setBodySize(num * multiplier);
	}
	catch(const out_of_range&)
	{
		throw runtime_error("Line " + to_string(lineNum) + ": size value too large.");
	}
}

void Config::parseErr(string& val, ServerConfig& server, int lineNum)
{
    emptyValCheck(val, lineNum);
    static const set<int> validErrCodes = {
        400, 401, 402, 403, 404, 405, 406, 407, 408, 409,
        410, 411, 412, 413, 414, 415, 416, 417, 418, 421,
        422, 426, 500, 501, 502, 503, 504, 505
    };

    istringstream iss(val);
    vector<int> errCodes;
    string errCode;
    string errPath;

    // Read all error codes
    while (iss >> errCode)
    {
        // If the token starts with '/', we've reached the path.
        if (errCode[0] == '/')
        {
            errPath = errCode;
            break;
        }
        size_t pos = 0;
        int code;
        try
        {
            code = stoi(errCode, &pos);
        }
        catch (const invalid_argument&)
        {
            throw runtime_error( "Line " + to_string(lineNum) + ": invalid error code.");
        }
        catch (const out_of_range&)
        {
            throw runtime_error("Line " + to_string(lineNum) + ": error code out of range.");
        }

        // Make sure the entire token was a number.
        if (pos != errCode.size())
        {
            throw runtime_error("Line " + to_string(lineNum) +": invalid error code.");
        }

        if (validErrCodes.find(code) == validErrCodes.end())
        {
            throw runtime_error("Line " + to_string(lineNum) +": found invalid error code.");
        }
        errCodes.push_back(code);
    }

    if (errCodes.empty() || errPath.empty())
    {
        throw runtime_error("Line " + to_string(lineNum) +": error_page requires a code and a path.");
    }
    verifyErrPath(errPath, lineNum);
    server.setErrPage(errCodes, errPath);
}

void Config::parseServerName(string& val, ServerConfig& server, int lineNum)
{
	emptyValCheck(val, lineNum);
	string tmp = val;

	if (val.size() > 253)
        throw runtime_error("Line " + to_string(lineNum) + ": server name too long.");
    if (val.front() == '.' || val.back() == '.')
	{
        throw runtime_error("Line " + to_string(lineNum) + ": server name cannot begin or end with '.'.");
	}
	while (true)
	{
    	size_t dotPos = tmp.find_first_of(".");
    	if (dotPos == string::npos)
            break;
		string label = tmp.substr(0, dotPos);
		verifyLabel(label, lineNum);
		tmp.erase(0, dotPos + 1);
	}
	verifyLabel(tmp, lineNum);
	server.setServerName(val);
}

void Config::parseHost(string& val, ServerConfig& server, int lineNum)
{
	emptyValCheck(val, lineNum);
	string tmp = val;//store val to tmp
	int numOfDots = 0;
	while (true)
	{
		size_t dotPos = tmp.find('.');
		//the correct host format should contain 3 dots, exit loop when there are no more dots
		if (dotPos == string::npos)
        	break;
		//extract the value before dotPos into sub
		string sub = tmp.substr(0, dotPos);
		//evaluate if extracted sub value is valid
		verifyNum(sub, lineNum);
		//count the amount of dots
		numOfDots++;
		//delete the value that has been verified
    	tmp.erase(0, dotPos + 1);
	}
	verifyNum(tmp, lineNum);
	if (numOfDots != 3)
		throw runtime_error("Line " + to_string(lineNum) + ": wrong amount of '.'.");
	server.setHost(val);
}

void Config::parseRoot(string& val, ServerConfig& server, int lineNum)
{
	emptyValCheck(val, lineNum);
	string	validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/._-";
	struct stat info;
	if (val.find_first_not_of(validChars) != string::npos)
		throw runtime_error("Line " + to_string(lineNum) + ": found invalid character.");
	if (stat(val.c_str(), &info) == -1)
		throw runtime_error("Line " + to_string(lineNum) + ": root path does not exist.");
	if (!S_ISDIR(info.st_mode))
		throw runtime_error("Line " + to_string(lineNum) + ": root path is not a directory.");
	server.setRoot(val);
}

void Config::parseIndex(string& val, ServerConfig& server, int lineNum)
{
	emptyValCheck(val, lineNum);
	if (val.find("/") != string::npos)
		throw runtime_error("Line " + to_string(lineNum) + ": invalid file name.");
	string path = server.getRoot() + '/' + val;
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
        throw runtime_error("Line " + to_string(lineNum) + ": index file does not exist.");
	if (!S_ISREG(info.st_mode))//check if a file is valid
		throw runtime_error("Line " + to_string(lineNum) + ": index is not a regular file.");
	server.setIndex(val);
}

void Config::parsePort(string& val, ServerConfig& server, int lineNum)
{
	//pop_back() removes the last element
	emptyValCheck(val, lineNum);
	int port = 0;
	size_t pos = 0;
	try
	{
		port = stoi(val, &pos);
	}
	catch(const std::exception& e)
	{
    	throw runtime_error("Line " + to_string(lineNum) + ": Invalid port number."); 
	}
	
    if (port < 1 || port > 65535 || pos != val.size())
	{
        throw runtime_error("Line " + to_string(lineNum) + ": invalid port number.");
	}
	server.setPort(port);
}