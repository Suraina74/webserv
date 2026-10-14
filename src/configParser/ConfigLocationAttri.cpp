#include "../../inc/Config.hpp"
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

    while (val.size() > 1 && val.back() == '/')//removes all trailing '/' in path. etc. /images/// -> /images
        val.pop_back();

    if (val.find("//") != string::npos || hasDotDotSegment(val))
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
	verifyDirPath(val, lineNum, "root path");
	loc.setRoot(val);
}

void Config::parseLocIndex(string& val, Location& loc, int lineNum)
{
	emptyValCheck(val, lineNum);
	//a file name only: "../x" or "a/b" would escape or change the location root
	if (val.find('/') != string::npos || val == "..")
		throw runtime_error("Line " + to_string(lineNum) + ": invalid file name.");
	loc.setIndex(val);
}

void Config::parseRedirect(string& val, Location& loc, int lineNum)
{
	emptyValCheck(val, lineNum);
	//a path on this server (/new-page) or a full URL (http://example.com/x)
	if (val[0] != '/' && val.compare(0, 7, "http://") != 0 && val.compare(0, 8, "https://") != 0)
		throw runtime_error("Line " + to_string(lineNum) + ": redirect must start with '/', 'http://' or 'https://'.");
	static const string validChars =
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~:/?#[]@!$&'()*+,=%";
	if (val.find_first_not_of(validChars) != string::npos)
		throw runtime_error("Line " + to_string(lineNum) + ": found invalid character in redirect.");
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
	verifyDirPath(val, lineNum, "upload path");
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
