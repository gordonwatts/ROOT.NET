#pragma once
///
/// A reference to a global variable.
///

#include <string>
#include <vector>

class RootGlobalVariable
{
public:
	// Init the global variable struct
	// - name - the fully qualified name
	// - type - the C++ type ("TSystem *" or "int", etc.).
	RootGlobalVariable(const std::string &name, const std::string &type, const std::string &libName);
	inline RootGlobalVariable(void) {}
	~RootGlobalVariable(void);

	// Simple getters
	inline const std::string &Name (void) const
	{ return _name; }
	inline const std::string &Type (void) const
	{ return _type;}
	inline const std::string &LibName (void) const
	{ return _lib; }

	// Get the list of include files that must be done in order ot use this include in C++/CLI code
	std::vector<std::string> GetListOfIncludeFiles(void) const;

public:
	std::string _name;
	std::string _type;
	std::string _lib;
};

