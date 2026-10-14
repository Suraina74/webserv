#include "../../inc/Config.hpp"

void cleanLine(string& line)
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
	size_t lastPos = line.find_last_not_of(" \t\r");
		line.erase(lastPos + 1);
}

void emptyValCheck(string& val, int lineNum)
{
	if (val.empty())
		throw runtime_error("Line " + to_string(lineNum) + ": empty value.");

	// strip trailing whitespace/CR/LF first
	while (!val.empty() && (val.back() == ' ' || val.back() == '\t' ||
	         val.back() == '\r' || val.back() == '\n'))
		val.pop_back();

	if (val.empty())
		throw runtime_error("Line " + to_string(lineNum) + ": empty value.");

	if (val.back() != ';')
		throw runtime_error("Line " + to_string(lineNum) + ": Missing ';'");

	val.pop_back();

	if (val.empty())
		throw runtime_error("Line " + to_string(lineNum) + ": empty value.");
}

void verifyNum(string sub, int lineNum)
{
	int octet;
	size_t pos = 0;
	if (sub.empty())
		throw runtime_error("Line " + to_string(lineNum) + ": empty octet value.");
	try
	{
		octet = stoi(sub, &pos); 
	}
	catch(const std::exception& e)
	{
    	throw runtime_error("Line " + to_string(lineNum) + ": Invalid octet number."); 
	}
	if (octet > 255 || octet < 0)
		throw runtime_error("Line " + to_string(lineNum) + ": Invalid octet range."); 
	if (pos != sub.length())
		throw runtime_error("Line " + to_string(lineNum) + ": Invalid octet value."); 
}

void verifyLabel(string& val, int lineNum)
{

	if (val.empty())
        throw runtime_error("Line " + to_string(lineNum) + ": empty server name label.");
	if (val.size() > 63)
		throw runtime_error("Line " + to_string(lineNum) + ": server name label too long.");
	string	validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-";
	if (val.find_first_not_of(validChars) != string::npos)
		throw runtime_error("Line " + to_string(lineNum) + ": invalid character in server name.");
	if (val.front() == '-' || val.back() == '-')
		throw runtime_error("Line " + to_string(lineNum) + ": server name cannot end or begin with '-'.");
}

//root and upload_path: relative to the directory webserv runs from (e.g. www) or absolute,
//must exist and be a directory. Trailing '/' are removed.
void verifyDirPath(string& val, int lineNum, const string& name)
{
	// Preserve the filesystem root while removing trailing slashes.
	while (val.size() > 1 && val.back() == '/')
		val.pop_back();
	static const string validChars =
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/._-";
	if (val.find_first_not_of(validChars) != string::npos)
		throw runtime_error("Line " + to_string(lineNum) + ": found invalid character in " + name + ".");
	struct stat info;
	if (stat(val.c_str(), &info) == -1)
		throw runtime_error("Line " + to_string(lineNum) + ": " + name + " '" + val + "' does not exist.");
	if (!S_ISDIR(info.st_mode))
		throw runtime_error("Line " + to_string(lineNum) + ": " + name + " '" + val + "' is not a directory.");
}

//true if any '/'-separated segment is exactly "..", e.g. /a/../b or /.. (but not /a..b)
bool hasDotDotSegment(const string& path)
{
	size_t start = 0;
	while (start <= path.size())
	{
		size_t end = path.find('/', start);
		if (end == string::npos)
			end = path.size();
		if (path.compare(start, end - start, "..") == 0)
			return true;
		start = end + 1;
	}
	return false;
}

//every directive may appear once per block, except error_page and cgi (one per code/extension)
void checkDuplicate(const string& line, set<string>& seen, int lineNum)
{
	istringstream iss(line);
	string key;
	iss >> key;
	if (key == "error_page" || key == "cgi")
		return;
	if (!seen.insert(key).second)
		throw runtime_error("Line " + to_string(lineNum) + ": duplicate directive '" + key + "'.");
}

void verifyErrPath(string& path, int lineNum)
{
	if (path[0] != '/')
		throw runtime_error("Line " + to_string(lineNum) + ": path should start with '/'.");
	static const string validChars =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/._-";
    if (path.find_first_not_of(validChars) != string::npos)
        throw runtime_error("Line " + to_string(lineNum) + ": invalid character in error path.");

    if (path.find("..") != string::npos)
        throw runtime_error("Line " + to_string(lineNum) + ": path cannot contain '..'.");

}