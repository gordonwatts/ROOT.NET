///
/// ROOT.NET Library Converter
///
///  Attempts to wrap the ROOT libraries on a library-by-library basis. Leaves be hehind a full blown solution with
/// a subproject for every library. That can be built from the command line or opened in the IDE and built there.
///

#include "LibraryConverterDriver.h"
#include "WrapperConfigurationInfo.hpp"

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iterator>

using std::string;
using std::vector;
using std::for_each;
using std::cout;
using std::endl;
using std::istringstream;
using std::find_if;
using std::ostringstream;
using std::ostream_iterator;

namespace {
	// Split a string by a delimeter...
	std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
		std::stringstream ss(s);
		std::string item;
		while(std::getline(ss, item, delim)) {
			elems.push_back(item);
		}
		return elems;
	}
}

int main(int argc, char* argv[])
{
	///
	/// Parameters to be filled by the command line arguments
	///

	string output_base_directory;	// Where the solution and sub-projects will be written.
	vector<string> libraries;		// List of libraries we will convert.
	int v_maj = 0, v_min = 0;		// What is the major version and minor version?

	///
	/// Command line parsing
	///

	output_base_directory = "..\\..\\Wrappers\\ROOTDotNetByLib";

	for (int i = 1; i < argc; i++) {
	  string arg = argv[i];
	  if (arg == "-d") { // Destination directory
	    i++;
	    output_base_directory = argv[i];
	  }
	  if (arg == "-v") { // Version number (x.y)
		  i++;
		  string temp = argv[i];
		  auto index = temp.find(".");
		  if (index == string::npos)
		  {
			  cout << "Version number must be <int>.<int> for major and minor versions" << endl;
			  cout << "  Assuming the version number is " << temp << ".0" << endl;
			  temp += ".0";
		  }
		  string major = temp.substr(0, index);
		  string minor = temp.substr(index+1);

		  istringstream s_major(major), s_minor(minor);
		  s_major >> v_maj;
		  s_minor >> v_min;
	  }
	}

	///
	/// Clean up the directory name, just in case. This is a lot of work for something pretty simple!
	///

	vector<string> subdirs;
	split (output_base_directory, '\\', subdirs);
	while (true)
	{
		auto backwards = find_if (subdirs.begin(), subdirs.end(), [] (const string &d) { return d == ".."; });
		if (backwards == subdirs.end())
			break;
		subdirs.erase(backwards-1, backwards+1);
	}
	ostringstream newoutput;
	copy (subdirs.begin(), subdirs.end(), ostream_iterator<string>(newoutput, "\\"));
	output_base_directory = newoutput.str().substr(0, newoutput.str().size() - 1);

	///
	/// Tell the translation system what libraries to look at!
	///

	LibraryConverterDriver driver;
	driver.set_version(v_maj, v_min);

	vector<string> base_root_libs (WrapperConfigurationInfo::GetAllRootDLLS());
	for_each(base_root_libs.begin(), base_root_libs.end(), [&driver] (string &s) { driver.translate_classes_in_library(s); });

	///
	/// Set the output info
	///

	driver.set_output_solution_directory(output_base_directory);
	driver.write_solution(true);

	///
	/// Do the work!
	///

	driver.translate();

	return 0;
}
