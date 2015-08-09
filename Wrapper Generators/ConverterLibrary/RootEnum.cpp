///
/// Code to help out with enums. Which are pesky.
///

#include "RootEnum.hpp"
#include "ROOTHelpers.h"
#include "ConverterErrorLog.hpp"
#include "RootClassInfoCollection.hpp"

#include "TClass.h"
#include "Api.h"

#include <vector>
#include <string>
#include <stdexcept>

using std::vector;
using std::string;
using std::pair;
using std::runtime_error;

///
/// Update values -- see if we should search global scope to find some enum contents. :-)
///
void RootEnum::update_values() const
{
	///
	/// If it was done by hand, skip it.
	///

	if (_self_fill) {
		return;
	}

	///
	/// If there is already stuff in there, then bug out too
	///

	if (_values.size() > 0) {
		return;
	}

	vector<pair<string, int> > vals = ROOTHelpers::GetEnumValues(_name);
	_values.insert(_values.end(), vals.begin(), vals.end());
}

///
/// return the include filename
///
string RootEnum::include_filename() const
{
	init_cint_data();
	return _include_filename;
}

//
// What library are we in?
//
string RootEnum::LibraryName() const
{
	init_cint_data();
	return _library_name;
}

///
/// Return the .NET class name that this enum is defined in. If it is the
/// global space, just return a blank.
///
string RootEnum::NETClassName() const
{
	init_cint_data();

	if (!IsClassDefined()) {
		return "";
	}

	int lastID = _name.rfind("::");
	if (lastID == string::npos) {
		return "";
	}

	return "N" + _name.substr(0, lastID);
}

///
/// Load up cint stuff
///
void RootEnum::init_cint_data() const
{
	if (_cint_inited) {
		return;
	}
	_cint_inited = true;

	///
	/// Get the class name that contains this enum then we have to figure out where the class is defined
	///

	string class_name (_name);
	auto indexOfClass = class_name.rfind("::");
	bool is_class_defined = indexOfClass != string::npos;
	if (is_class_defined) {
		class_name = class_name.substr(0, indexOfClass);
	}

	///
	/// Things are pretty simple if this is a class enum
	///

	if (is_class_defined)
	{
		auto &c = RootClassInfoCollection::GetRootClassInfo(class_name);
		_library_name = c.LibraryName();
		_include_filename = c.include_filename();
		return;
	}

	///
	/// Get the low-down from CINT for the actual enum.
	///

	string full_name;

	G__ClassInfo *cinfo = new G__ClassInfo (class_name.c_str());
	if (!cinfo->IsValid()) {
		ConverterErrorLog::log_type_error(_name, "CINT doesn't seem to know about this enum!");
		full_name = "TObject.h";
		_library_name = "libCore";
	}

	///
	/// Hard time figuring out where the files are actually included here...
	///

	if (cinfo->DefFile() == 0) {
		_library_name = "libCore";
		full_name = "TObject.h";
		ConverterErrorLog::log_type_error(_name, "CINT doesn't seem to know the file where this object is defined!");
	} else {
		full_name = cinfo->DefFile();
	}

	///
	/// Now do the parse for the actual name of the include.
	///

	int index = full_name.find_last_of ("/");
	if (index != string::npos) {
		index = index == string::npos ? -1 : index;
		full_name = full_name.substr(index+1);
	}
	_include_filename = full_name;
}

///
/// Return the name with no class specification, etc.
///
string RootEnum::NameQualified() const
{
	if (_name == "enum ") {
		return "enum_";
	} else {
		return _name;
	}
}

///
/// Return the full name
///
string RootEnum::NameUnqualified() const
{
	if (_name == "enum ") {
		return "enum_";
	} else {
		int index = _name.find("::");
		if (index == _name.npos) {
			return _name;
		}

		string stripped (_name.substr(index+2));
		return stripped;
	}
}

///
/// Return true if this is defined in a class
///
bool RootEnum::IsClassDefined() const
{
	return _name.find("::") != _name.npos;
}
