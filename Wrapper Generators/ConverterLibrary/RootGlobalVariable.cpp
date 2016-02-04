#include "RootGlobalVariable.hpp"
#include "CPPNetTypeMapper.hpp"
#include "RootClassInfoCollection.hpp"
#include "ROOTHelpers.h"

#include <TSystem.h>
#include <TString.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

using std::string;
using std::ifstream;
using std::vector;
using std::for_each;
using std::getline;
using std::ostringstream;
using std::runtime_error;

//
// Set ourselves up for work!
//
RootGlobalVariable::RootGlobalVariable(const std::string &name, const std::string &type, const std::string &lib)
	: _name(name), _type(type), _lib(lib)
{
}


RootGlobalVariable::~RootGlobalVariable(void)
{
}

namespace {
	// Look for a string in a file.
	bool SearchFileFor (const string &filename, const string &name)
	{
		ifstream input(filename.c_str());
		string line;
		while (!input.eof())
		{
			getline(input, line);
			if (line.find(name) != string::npos)
				return true;
		}
		return false;
	}
}

///
/// determine all includes needed to make this guy work. There are potentially two:
/// 1) the header file where the global variable is actually defined
/// 2) If this is a ROOT object then we need ot include that type's include file.
///
vector<string> RootGlobalVariable::GetListOfIncludeFiles(void) const
{
	vector<string> result;

	//
	// See if we are a big-type type.
	//

	if (CPPNetTypeMapper::instance()->has_mapping(Type())) {
		auto trans = CPPNetTypeMapper::instance()->get_translator_from_cpp(Type());
		if (trans->net_typename().substr(0, 2) == "NT") {
			auto info = RootClassInfoCollection::GetRootClassInfo(trans->cpp_core_typename());
			result.push_back(info.NETQualifiedName() + ".hpp");
			result.push_back(info.include_filename());
		}
	}

	//
	// Next we need to figure out where the guy is located. Now, if it was defined as a complex object, that so far
	// ROOT has kept to the rule that including the object includes the global. So we hard code that here.
	//

	if (result.size() == 0)
	{
		string fullPath = gSystem->ExpandPathName("$ROOTSYS/include");
		string tempPath = gSystem->ExpandPathName("$TEMP/found.txt");

		ostringstream cmd;
		cmd << "findstr /S \"" << Name() << ";\" " << fullPath << "/*.h > " << tempPath;
		auto run_result = gSystem->Exec(cmd.str().c_str());
		if (run_result != 0)
			throw runtime_error("Unable to run '" + cmd.str() + "'");

		ifstream findstr_results (tempPath.c_str());
		string line;
		while (!findstr_results.eof())
		{
			getline(findstr_results, line);
			if (line.find(fullPath) == 0)
			{
				line = line.substr(fullPath.size()+1);
				auto colon = line.find(":");
				if (colon != string::npos)
				{
					auto header = line.substr(0, colon);
					result.push_back(header);
				}
			}
		}
#ifdef old
		auto all_headers = ROOTHelpers::GetAllExistingHeaders();
		bool foundit = false;
		for_each(all_headers.begin(), all_headers.end(), [&] (const string &filename)
		{
			if (!foundit) {
				auto fullname = gSystem->FindFile("$ROOTSYS/include", TString(filename.c_str()));
				if (fullname != 0) {
					if(SearchFileFor(fullname, Name()))
					{
						foundit = true;
						result.push_back(filename);
					}
				}
			}
		});
#endif
	}

	return result;
}
