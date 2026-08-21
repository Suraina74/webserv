#include "Location.hpp"

Location::Location(){}

Location::~Location(){}

void Location::setPath(const string& path)
{
	_path = path;
}

void Location::setMethods(const vector<string>& methods)
{
	_methods = methods;
}

void Location::addMethod(const string& method)
{
	_methods.push_back(method);
}

void Location::addCGI(const string& ext, const string& path)
{
	_cgi[ext] = path;
}

void Location::setRoot(const string& root)
{
	_root = root;
}

void Location::setIndex(const string& index)
{
	_index = index;
}
void Location::setUploadPath(const string& uploadPath)
{
	_uploadPath = uploadPath;
}
void Location::setRedirect(const string& redirect)
{
	_redirect = redirect;
}
void Location::setUploadEnabled(bool enabled)
{
	_uploadEnabled = enabled;
}
void Location::setDirectoryListing(bool enabled)
{
	_directoryListing = enabled;
}

const string& Location::getPath() const
{
	return _path;
}

const vector<string>& Location::getMethods() const
{
	return _methods;
}

const string& Location::getRoot() const
{
	return _root;
}

const string& Location::getIndex() const
{
	return _index;
}

const string& Location::getUploadPath() const
{
	return _uploadPath;
}

const string& Location::getRedirect() const
{
	return _redirect;
}

bool Location::getUploadEnabled() const
{
	return _uploadEnabled;
}

bool Location::getDirectoryListing() const
{
	return _directoryListing;
}

const map<string, string>& Location::getCGI() const
{
	return _cgi;
}