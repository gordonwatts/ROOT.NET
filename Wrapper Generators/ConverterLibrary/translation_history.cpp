///
/// Implementation of the history gathering code - writes out the history
/// of a job and also reads it back in
///

#include "translation_history.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <map>

#include <Windows.h>

using std::ofstream;
using std::ifstream;
using std::for_each;
using std::string;
using std::endl;
using std::runtime_error;
using std::getline;
using std::vector;
using std::map;
using std::for_each;
using std::pair;

translation_history::translation_history(void)
{
}


translation_history::~translation_history(void)
{
}

namespace
{
	///
	/// See if we can figure out what the library name is for a particular class.
	string findFile(const string &className, const map<string, string> &filesByLibrary)
	{
		auto lookup = filesByLibrary.find(className);
		if (lookup == filesByLibrary.end())
			return "";
		return lookup->second;
	}
}

///
/// Write out the history of this translation job
///
void translation_history::save_history (const std::string &dir,
		const std::vector<std::string> &classes_translated,
		const std::vector<std::string> &enums_translated,
		const map<string, vector<string>> &filesByLibrary)
{
	///
	/// Reparse the filesByLibrary to make it simpler to do the lookups
	///

	map<string, string> classLibraries;
	for_each (filesByLibrary.begin(), filesByLibrary.end(),
		[&classLibraries] (const pair<string, vector<string>> &item) {
			auto libName = item.first;
			auto &refCL = classLibraries;
			for_each(item.second.begin(), item.second.end(),
				[&libName, &refCL] (const string &cname) {
					refCL[cname] = libName;
					if (cname[0] == 'N')
						refCL[cname.substr(1)] = libName;
			});
			
	});

	///
	/// Ok, next we should actually dump out the file.
	///

	ofstream converted_info ((dir + "\\converted_items.txt").c_str());

	for_each(classes_translated.begin(), classes_translated.end(),
		[&converted_info, &classLibraries] (const string &s) { converted_info << "class " << s << findFile(s, classLibraries) << endl; });
	for_each(enums_translated.begin(), enums_translated.end(),
		[&converted_info, &classLibraries] (const string &s) { converted_info << "enum " << s << findFile(s, classLibraries) << endl; });

	converted_info.close();
}

///
/// load_history - loads the history in. Given a directory, load it back.
///
void translation_history::load_history (const std::string &dir)
{
	///
	/// Load up the previously translated classes
	///

	ifstream input ((dir + "\\converted_items.txt").c_str());
	if (!input.good()) {
		throw runtime_error("Error opening the converted_items.txt file in " + dir + "!");
	}

	while (!input.eof()) {
		string line;
		getline(input, line);

		if (line.find("class ") == 0) {
			auto c = line.substr(6);
			_classes.insert(c);
		} else if (line.find("enum ") == 0) {
			auto e = line.substr(5);
			_enums.insert(e);
		}
	}

	///
	/// And load up the previously translated libraries
	///

	WIN32_FIND_DATAA ffind;
	HANDLE hfind;

	string libdirName = dir + "\\lib";
	hfind = FindFirstFileA((libdirName + "\\*.dll").c_str(), &ffind);
	if (hfind == INVALID_HANDLE_VALUE) {
		throw runtime_error("Unable to search lib directory with error " + GetLastError());
	}

	do {
		string libName = ffind.cFileName;
		libName = libName.substr(0, libName.length() - 4);

		_lib_to_dir[libName] = libdirName;
	} while (FindNextFileA(hfind, &ffind));

	FindClose(hfind);

}
