#include "ROOTHelpers.h"
#include "ConverterErrorLog.hpp"
#include "RootGlobalVariable.hpp"
#include "WrapperConfigurationInfo.hpp"
#include "RootClassInfoCollection.hpp"

#include <iostream>
#include <algorithm>
#include <exception>
#include <iterator>

#include "TROOT.h"
#include "TClass.h"
#include "TClassTable.h"
#include "TSystem.h"
#include "TGlobal.h"
#include "TSystemDirectory.h"

#include "Api.h"

using std::string;
using std::vector;
using std::map;
using std::find;
using std::set;
using std::for_each;
using std::pair;
using std::make_pair;
using std::transform;
using std::back_inserter;

using std::cout;
using std::endl;
using std::exception;

std::map<std::string, std::vector<std::pair<std::string, int> > > ROOTHelpers::_all_enums;
vector<string> ROOTHelpers::_all_headers;
vector<string> ROOTHelpers::_all_existing_headers;


class load_library
{
public:
	inline void operator() (const string &lib)
	{
		int result = gSystem->Load (lib.c_str());
		if (result != 0 && result != 1) {
			throw exception (("Failed to load library '" + lib + "'").c_str());
		}
	}
};

class extract_enum_classname {
public:
	inline extract_enum_classname (vector<string> &class_names)
		: _classes (class_names)
	{}
	inline void operator() (const pair<string, vector<pair<string, int> > > &item)
	{
		_classes.push_back(item.first);
	}
private:
	vector<string> &_classes;
};


///
/// Scan root's list of classes for ones that are in the passed in libraries.
///
vector<string> ROOTHelpers::GetAllClassesInLibraries(const vector<string> &library_names)
{
	///
	/// First, make sure all the libraries are loaded...
	///

	for_each (library_names.begin(), library_names.end(), load_library());

	///
	/// Re-do the library names into something that we will be able to use below. Mostly, this means
	/// removing any directories and .dll or .so's.
	///

	vector<string> shortLibNames;
	transform(
		library_names.begin(), library_names.end(),
		back_inserter<vector<string> >(shortLibNames),
		[] (const string &s) -> string {
			string lib (s);
			auto lastDir = lib.find_last_of('\\');
			if (lastDir != lib.npos) {
				lib = lib.substr(lastDir+1);
			}
			auto lastDot = lib.find_last_of('.');
			if (lastDot != lib.npos) {
				lib = lib.substr(0, lastDot);
			}
			return lib;
	});

	///
	/// Great. Next, get the list of classes root knows about and filter them by the list of libraries that
	/// we have been asked about...
	///

	vector<string> results;

	int num_classes = gClassTable->Classes();
	gClassTable->Init();
	for (int i = 0; i < num_classes; i++)
	{
		string class_name = gClassTable->Next();
		if (class_name.find("::") != class_name.npos) {
			ConverterErrorLog::log_type_error (class_name, "Private namespace not supported yet.");
			continue; // Don't do private namespace for now.
		}
		if (class_name.find("<") != class_name.npos) {
			ConverterErrorLog::log_type_error (class_name, "Template classes not supported yet.");
			continue; // Don't do templates for now.
		}
		if (!IsClass(class_name)) {
			continue;
		}

		string shared_library (GetClassLibraryName(class_name));

		if (find(shortLibNames.begin(), shortLibNames.end(), shared_library) != shortLibNames.end()) {
			results.push_back (class_name);
		} else {
			cout << "Class '" << class_name << "' can't be translated because it isn't in requested library (it is in '" << shared_library << "')." << endl;
		}
	}

	return results;
}

map<string, string> gClassLibraryNameCache;

void ROOTHelpers::ForceClassLibraryname (const std::string &class_name, const std::string &library_name)
{
	gClassLibraryNameCache[class_name] = library_name;
}


string ROOTHelpers::GetClassLibraryName (const std::string &class_name)
{
	if (gClassLibraryNameCache.find(class_name) != gClassLibraryNameCache.end()) {
		return gClassLibraryNameCache[class_name];
	}

	TClass *cls = TClass::GetClass(class_name.c_str(), true);
	if (!cls || !cls->GetSharedLibs()) {
		gClassLibraryNameCache[class_name] = "";
		return "";
	}

	string shared_libraries (cls->GetSharedLibs());
	string shared_library = shared_libraries;
	int first_space = shared_libraries.find(" ");
	if (first_space != shared_libraries.npos) {
		shared_library = shared_libraries.substr(0, first_space);
	}

	int dot_index = shared_library.find(".");
	if (dot_index != shared_library.npos) {
		shared_library = shared_library.substr(0, dot_index);
	}

	gClassLibraryNameCache[class_name] = shared_library;
	return shared_library;
}

/// True if this guy is a class, actually!
bool ROOTHelpers::IsClass (const std::string &class_name)
{
	TClass *cls = TClass::GetClass(class_name.c_str(), true);
	if (!cls) {
		return false;
	}

	/// Make sure there is a dictionary loaded for this guy!

	if (cls->GetClassInfo() == 0) {
	  return false;
	}

	/// Now see if it is declared as a class.

	return (cls->Property() & kIsClass) != 0;
}

/// True if this guy is an enum!
bool ROOTHelpers::IsEnum (const std::string &enum_name)
{
	G__ClassInfo *info = new G__ClassInfo (enum_name.c_str());
	bool is_enum = true;
	if (!info->IsValid()) {
		is_enum = false;
	} else {
		is_enum = (info->Property() & kIsEnum) != 0;
	}
	delete info;
	return is_enum;
}

namespace
{
	string CleanCINTCharReturn(const char *s, const string def = "")
	{
		if (s == nullptr)
			return def;
		return string(s);
	}
}
	/// Return all globalls we can find
map<string, vector<RootGlobalVariable> > ROOTHelpers::GetAllGlobals()
{
	G__DataMemberInfo dataMember;

	auto junk = dataMember.IsValid();
	map<string, vector<RootGlobalVariable> > result;

	long stuffWeDontWant = kIsEnum | kIsConstant;
	auto badGlobals = WrapperConfigurationInfo::GetListOfBadGlobalVariables();

	while (dataMember.Next() != 0)
	{
		auto p = dataMember.Property();
		if ((p & stuffWeDontWant) == 0) {
			auto typeinfo = dataMember.Type();
			if (typeinfo != nullptr) {
				string typeName = CleanCINTCharReturn(typeinfo->Name());
				string dataName = CleanCINTCharReturn(dataMember.Name());
				if (typeName != "#define" && find(badGlobals.begin(), badGlobals.end(), dataName) == badGlobals.end()) {
					string filename = CleanCINTCharReturn(dataMember.FileName());
					// By default the library is the core library... otherwise we will extract
					// the proper thing.
					string lib = "Core";
					auto libStarts = filename.find("\\lib");
					auto libEnds = filename.find(".dll");
					if (libStarts != string::npos && libEnds != string::npos)
					{
						libStarts += 4;
						lib = filename.substr(libStarts, libEnds - libStarts);
					}
			
					//
					// We need to keep the full type name for later processing. However, for
					// cataloging we need to strip out all the extra stuff (like pointers, etc.).
					//

					auto ptrLoc = typeName.find("*");
					auto rawTypeName (typeName);
					if (ptrLoc != string::npos)
						rawTypeName = typeName.substr(0, ptrLoc);

					//
					// Ok, now we can save this.
					//

					result[rawTypeName].push_back(RootGlobalVariable(dataName, typeName, lib));
				}
			}
		}
	}

#if old
	TIter iterator (gROOT->GetListOfGlobals());
	TGlobal *g;
	map<string, vector<string> > result;
	while ((g = static_cast<TGlobal*>(iterator.Next())) != 0) {
		if ((g->Property() & kIsEnum) == 0 && (g->Property() & kIsConstant) == 0) {
			result[g->GetTypeName()].push_back(g->GetName());
		}
	}
#endif
	return result;
}

/// Helper to rip off the first item in the list.
class grab_enum_name
{
public:
	void operator() (const pair<string, vector<pair<string, int> > > &item)
	{
		_result.push_back(item.first);
	}
	vector<string> result(void) {return _result;}
private:
	vector<string> _result;
};

/// Return all the enums we can find
vector<string> ROOTHelpers::GetAllEnums()
{
	LoadAllEnums();
	return for_each (_all_enums.begin(), _all_enums.end(), grab_enum_name()).result();
}

/// Load up the internal cache if not done already
void ROOTHelpers::LoadAllEnums(void)
{
	if (_all_enums.size() > 0) {
		return;
	}

	TIter iterator (gROOT->GetListOfGlobals());
	TGlobal *g;
	while ((g = static_cast<TGlobal*>(iterator.Next())) != 0) {
		if ((g->Property() & kIsEnum) != 0) {
			const int *pvalue = (const int *)g->GetAddress();
			string enum_name(g->GetTypeName());
			if (enum_name.find("::") != enum_name.npos) {
				continue;
			}
			_all_enums[enum_name].push_back(make_pair(string(g->GetName()), *pvalue));
		}
	}
}

///
/// Return the enum values for a particular enum dude.
const vector<pair<string, int> > &ROOTHelpers::GetEnumValues(const std::string &enum_type)
{
	LoadAllEnums();
	return _all_enums[enum_type];
}

///
/// Extract the arguments from a template. Go down only a single level! If this isn't a template, just return
/// the argument as the list!
///
vector<string> ROOTHelpers::GetTemplateArguments (const string &template_name)
{
	vector<string> result;
	int arg_start = template_name.find("<");
	if (arg_start == template_name.npos) {
		result.push_back(template_name);
		return result;
	}

	string arg = template_name.substr(arg_start+1, template_name.size()-2-arg_start);

	while (arg.find(",") != arg.npos) {
		result.push_back(arg.substr(0, arg.find(",")));
		arg = arg.substr(arg.find(",")+1);
	}
	result.push_back(arg);
	return result;
}

//
// Get a list of all headers that we are dealing with currently.
//
const vector<string> &ROOTHelpers::GetAllHeaders()
{
	if (_all_headers.size() > 0)
		return _all_headers;

	//
	// Extract from the list of classes we are looking at.
	//

	auto libraries = WrapperConfigurationInfo::GetAllRootDLLS();
	libraries = WrapperConfigurationInfo::RemoveBadLibraries(libraries);
	auto all_classes = ROOTHelpers::GetAllClassesInLibraries (libraries);

	set<string> headers;
	for_each(all_classes.begin(), all_classes.end(), [&headers] (const string &class_name)
	{
		RootClassInfo &class_info (RootClassInfoCollection::GetRootClassInfo(class_name));
		headers.insert(class_info.include_filename());
	});

	_all_headers.assign(headers.begin(), headers.end());

	return _all_headers;
}

namespace
{
	set<string> FindHeaders(TSystemDirectory *dir)
	{
		set<string> results;
		TIter next(dir->GetListOfFiles());
		while (TSystemFile *f = static_cast<TSystemFile*>(next()))
		{
			if (f->GetName() != string(".") && string(f->GetName()) != "..")
			{
				if (f->IsDirectory())
				{
					auto deeper = FindHeaders(static_cast<TSystemDirectory*>(f));
					for_each(deeper.begin(), deeper.end(), [&] (const string &name)
					{
						results.insert(string(f->GetName()) + "/" + name);
					});
				} else {
					results.insert(f->GetName());
				}
			}
		}
		return results;
	}
}

//
// Look in ROOTSYS for all include files around!
//
const vector<string> &ROOTHelpers::GetAllExistingHeaders()
{
	if (_all_existing_headers.size() > 0)
		return _all_existing_headers;

	auto fullpath = gSystem->ExpandPathName("$ROOTSYS/include");
	auto includeDir = new TSystemDirectory("include", fullpath);
	
	auto found_headers = FindHeaders(includeDir);
	_all_existing_headers.assign(found_headers.begin(), found_headers.end());
	delete includeDir;

	return _all_existing_headers;
}
