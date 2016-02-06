///
/// FindBadRootHeaders
///
///  Try to compile every single root header we can find, on its own, and keep track of which ones fail to compile. Yuck.
///

#include "ROOTHelpers.h"
#include "ROOTClassInfo.hpp"
#include "WrapperConfigurationInfo.hpp"
#include "RootClassInfoCollection.hpp"

#include "TSystem.h"
#include "TApplication.h"

#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <sstream>
#include <fstream>
#include <iterator>
#include <iostream>

using std::string;
using std::vector;
using std::for_each;
using std::set;
using std::ostringstream;
using std::ofstream;
using std::endl;
using std::copy;
using std::ostream_iterator;

class find_bad_headers
{
public:
	void operator() (const string &inlcude_filename);
	const set<string> &Headers (void) const {return _headers;}
private:
	set<string> _headers;
};


int main(int argc, char* argv[])
{
	///
	/// Start up ROOT
	///

	int nargs = 2;
	char *myargv[2];
	myargv[0] = "ROOT.NET Library Converter.exe";
	myargv[1] = "-b";
	TApplication *app = new TApplication ("ROOT.NET Library Converter", &nargs, myargv);

	//
	// Get all headers...
	//

	auto &headers = ROOTHelpers::GetAllHeaders();

	///
	/// Now, for each header, try it out. When we are done we'll have a list of bad ones! :-)
	///

	set<string> bad_headers = for_each(headers.begin(), headers.end(), find_bad_headers()).Headers();

	///
	/// Finally, write these bad boys out!
	///

	ofstream bad_header_list ("bad_headers.txt");
	copy (bad_headers.begin(), bad_headers.end(), ostream_iterator<string>(bad_header_list, "\n"));
	bad_header_list.close();

	/// Done!
	return 0;
}

void delete_file (const string &filename);

/// Compile a header and see if it "works" ok???
void find_bad_headers::operator ()(const std::string &inlcude_filename)
{
	vector<string> files_to_delete;

	string include_name_stub (inlcude_filename);
	size_t pos = include_name_stub.find_last_of(".");
	if (pos != include_name_stub.npos) {
		include_name_stub = include_name_stub.substr(0, pos);
	}
	replace(include_name_stub.begin(), include_name_stub.end(), '/', '_');

	/// Write out the input file
	string input_source_filename (include_name_stub + "-input.cpp");
	ofstream output_source (input_source_filename.c_str());
	files_to_delete.push_back(input_source_filename);
	files_to_delete.push_back(include_name_stub + "-input.obj");

	/// Add the hack to get a number of the important headers working.
	output_source << "#include \"iostream\"" << endl;
	output_source << "using namespace std;" << endl;

	/// Now include the header...
	output_source << "#include \"" << inlcude_filename << "\"" << endl;
	output_source.close();

	/// Build up the command line
	ostringstream cmd_line;
	cmd_line << "\"C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\vcvarsall.bat\" x86 & ";
	string error_output_file (include_name_stub + "-errors.txt");
	files_to_delete.push_back(error_output_file);
	cmd_line << "cl /D WIN32 /EHsc /I %ROOTSYS%\\include /FI w32pragma.h  /c /nologo " << input_source_filename << "> " << error_output_file;

	/// Now run the thing!
	int result = gSystem->Exec(cmd_line.str().c_str());

	/// Mark bad headers, leave the error output file around for people to look at if they are curious.
	if (result != 0) {
		_headers.insert(inlcude_filename);
		vector<string>::iterator itr = find (files_to_delete.begin(), files_to_delete.end(), error_output_file);
		files_to_delete.erase(itr);
	}

	/// Get rid of any files we don't care about any longer!
	for_each (files_to_delete.begin(), files_to_delete.end(), delete_file);
}

/// Delete a file...
void delete_file (const string &filename)
{
	gSystem->Unlink(filename.c_str());
}
