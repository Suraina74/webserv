#include "../inc/Config.hpp"

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