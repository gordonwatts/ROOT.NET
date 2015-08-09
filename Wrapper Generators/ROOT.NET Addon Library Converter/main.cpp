///
/// Generate a ROOT.NET translation that is:
///		1) Based on only a set of libraries already specified
///		2) All in one output .DLL/etc/ file
///		3) Possibly building on a previous translation
///

#include "LibraryConverterDriver.h"

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <stdexcept>

using std::string;
using std::vector;
using std::for_each;
using std::cout;
using std::endl;
using std::exception;

void usage (void)
{
	cout << "Usage: the add on tool is simple, if unfriendly..." << endl;
	cout << "Addon <dir-of-wrappers> <output-solution-folder> <output-library-name> <dll1> <dll2>..." << endl;
	cout << "  dir-of-wrapers - location of the wrappers on your system. For example" << endl;
	cout << "                   c:\\root\\NetWrappers" << endl;
	cout << "  output-solution-folder - the directory where the output solution is located." << endl;
	cout << "                           This tool will create a sub-dir with" << endl;
	cout << "                           output-library-name that will contain the wrapper" << endl;
	cout << "                           library files. It will not modify any .sln files." << endl;
	cout << "                           You have to do that!" << endl;
	cout << "  output-library-name - Legal VC project name - like 'additional'. " << endl;
	cout << "  dllN - Path that can be used by root to load a ROOT dll. For example: " << endl;
	cout << "            C:\\Users\\gordon\\\\Projects\\LINQToROOT\\NTupleSource\\BTagJet_cpp" << endl;
}

int main(int argc, char* argv[])
{
	///
	/// Command line arguments processing
	///

	if (argc < 5) {
		usage();
		return 0;
	}

	vector<string> prev_trans_directory;
	prev_trans_directory.push_back(argv[1]);

	string dir_of_project_folder = argv[2];

	string libname = argv[3];

	vector<string> dlls;
	for (int i = 4; i < argc; i++) {
		dlls.push_back(argv[i]);
	}

	///
	/// If there are any other directories that contain libraries we've previously translated,
	/// pick up all the previously translated things from there
	///

	LibraryConverterDriver driver;
	for_each(prev_trans_directory.begin(), prev_trans_directory.end(), [&driver] (const string &s) { driver.add_converted_info(s);});

	///
	/// Ok, now specify the list of libraries we want translated
	///

	for_each(dlls.begin(), dlls.end(), [&driver] (const string &s) { driver.translate_classes_in_library(s); });

	///
	/// The specifics (output location, etc.)
	///

	driver.set_output_solution_directory(dir_of_project_folder);
	driver.write_solution(false);
	driver.use_class_header_locations(true);
	driver.write_all_in_single_library(libname);
	driver.print_error_report(false);
	driver.set_template_files_location (prev_trans_directory[0] + "\\bin");

	///
	/// Now, do the translation and write out the final project solutoin
	///

	try {
		driver.translate();
	} catch (exception &e)
	{
		cout << "ERROR translating: " << e.what() << endl;
		return 100;
	}
}
