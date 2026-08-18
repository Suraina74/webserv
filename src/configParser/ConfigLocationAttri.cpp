#include "Config.hpp"
#include <algorithm>

void Config::parseLocPath(string& val, Location& loc, int lineNum)
{
    if (val.empty())
		throw runtime_error("Line " + to_string(lineNum) + ": empty value.");

    const string validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/._-";

    if (val[0] != '/')
        throw runtime_error("Line " + to_string(lineNum) + ": location path must start with '/'.");

    if (val.find_first_not_of(validChars) != string::npos)
        throw runtime_error("Line " + to_string(lineNum) + ": found invalid character in location path.");

    if (val.find("//") != string::npos)
	{
        throw runtime_error("Line " + to_string(lineNum) + ": invalid location path.");
	}

	loc.setPath(val);
}

void Config::parseMethods(string& val, Location& loc, int lineNum)
{
    emptyValCheck(val, lineNum);
    vector<string> methods;

    static const vector<string> allowed = {"GET", "POST", "DELETE"};

    istringstream iss(val);
    string token;
    while (iss >> token)
    {
        if (find(allowed.begin(), allowed.end(), token) == allowed.end())
            throw runtime_error("Line " + to_string(lineNum) + ": unsupported HTTP method '" + token + "'.");
        if (find(methods.begin(), methods.end(), token) != methods.end())
            throw runtime_error("Line " + to_string(lineNum) + ": duplicate method '" + token + "'.");
        methods.push_back(token);
    }

    if (methods.empty())
        throw runtime_error("Line " + to_string(lineNum) + ": no valid methods specified.");

    loc.setMethods(methods);
}

void Config::parseLocRoot(string& val, Location& loc, int lineNum)
{
	 emptyValCheck(val, lineNum);

    if (val[0] != '/')
        throw runtime_error("Line " + to_string(lineNum) + ": root path must start with '/'.");

    loc.setRoot(val);
}

void Config::parseLocIndex(string& val, Location& loc, int lineNum)
{
	emptyValCheck(val, lineNum);

    loc.setIndex(val);
}

void Config::parseRedirect(string& val, Location& loc, int lineNum)
{
	emptyValCheck(val, lineNum);

    loc.setRedirect(val);
}

void Config::parseCGI(string& val, Location& loc, int lineNum)
{
	emptyValCheck(val, lineNum);

    istringstream iss(val);
    string ext, path;

    if (!(iss >> ext >> path))
        throw runtime_error("Line " + to_string(lineNum) + ": cgi directive expects '<extension> <path>'.");

    string extra;
    if (iss >> extra)
        throw runtime_error("Line " + to_string(lineNum) + ": too many arguments for cgi directive.");

    if (ext[0] != '.')
        throw runtime_error("Line " + to_string(lineNum) + ": cgi extension must start with '.'.");

    loc.addCGI(ext, path);
}

void Config::parseUploadPath(string& val, Location& loc, int lineNum)
{
	emptyValCheck(val, lineNum);

    if (val[0] != '/')
        throw runtime_error("Line " + to_string(lineNum) + ": upload path must start with '/'.");

    loc.setUploadPath(val);
}
void Config::parseUpload(string& val, Location& loc, int lineNum)
{
	emptyValCheck(val, lineNum);

    if (val == "on" || val == "true")
        loc.setUploadEnabled(true);
    else if (val == "off" || val == "false")
        loc.setUploadEnabled(false);
    else
        throw runtime_error("Line " + to_string(lineNum) + ": upload value must be 'on' or 'off'.");
}

void Config::parseDirectoryListing(string& val, Location& loc, int lineNum)
{
	emptyValCheck(val, lineNum);

    if (val == "on" || val == "true")
        loc.setDirectoryListing(true);
    else if (val == "off" || val == "false")
        loc.setDirectoryListing(false);
    else
        throw runtime_error("Line " + to_string(lineNum) + ": directory listing value must be 'on' or 'off'.");
}