#pragma once
#include <string>
#include <map>
#include <vector>
#include <cstddef>

using namespace std;

class Location
{
	private:
   		string					_path;
   		vector<string>			_methods;
   		string					_root;
   		string					_index;
   		string					_uploadPath;
   		string					_redirect;
		map<string, string>  	_cgi;
   		bool					_uploadEnabled;
   		bool					_directoryListing;
	
	public:
		// Constructor
		Location();
		~Location();
		// Setters
    	void setPath(const string& path);
    	void setMethods(const vector<string>& methods);
    	void addMethod(const string& method);
		void addCGI(const string& ext, const string& path);
    	void setRoot(const string& root);
    	void setIndex(const string& index);
    	void setUploadPath(const string& uploadPath);
    	void setRedirect(const string& redirect);
    	void setUploadEnabled(bool enabled);
    	void setDirectoryListing(bool enabled);

    	// Getters
    	const string& getPath() const;
    	const vector<string>& getMethods() const;
    	const string& getRoot() const;
    	const string& getIndex() const;
    	const string& getUploadPath() const;
    	const string& getRedirect() const;
    	bool getUploadEnabled() const;
    	bool getDirectoryListing() const;
		const map<string, string>& getCGI() const;
};