///
/// Implementation for the project file writer code
///

#include "create_project_files.hpp"
#include "ClassTranslator.hpp"
#include "WrapperConfigurationInfo.hpp"

#include "shlobj.h"

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <stdexcept>

using std::pair;
using std::string;
using std::vector;
using std::ofstream;
using std::ifstream;
using std::ostream;
using std::endl;
using std::for_each;
using std::back_inserter;
using std::find;
using std::find_if;
using std::runtime_error;

///
/// Helper to add an include into the project file.
///
class add_class_file_reference
{
public:
	inline add_class_file_reference (const std::string &filetype, const string &command, ostream &output)
		: _filetype (filetype), _output(output), _command(command)
	{}

	inline void operator() (const string &class_name)
	{
		_output << "<" << _command << " Include=\".\\Source\\" << class_name << "." << _filetype << "\"/>" << endl;
	}
private:
	const string _filetype, _command;
	ostream &_output;
};

///
/// Get the guid for a project. If we know about it - use it. Otherwise make up a new one.
///
string create_project_files::get_guid (const string &libname)
{
	auto l = find_if (_project_guid.begin(), _project_guid.end(), [&libname] (const pair<string, string> &info) { return info.first == libname; });
	if (l == _project_guid.end()) {
		GUID guid;
		CoCreateGuid (&guid);
		RPC_CSTR g_string;
		UuidToStringA (&guid, &g_string);
		string guidstr = string((const char*)g_string);
		RpcStringFreeA(&g_string);
		_project_guid.push_back(make_pair(libname, guidstr));

		return guidstr;
	} else {
		return l->second;
	}
}

///
/// Create the project file
///
void create_project_files::operator ()(const pair<string,vector<string> > &library_classes)
{
	const string library_name = library_classes.first;
	const vector<string> &classes = library_classes.second;

	///
	/// Open the input template and the output solution file
	///

	string project_path = _base_dir + "\\" + library_name + "\\" + library_name + ".vcxproj-temp";
	ofstream output (project_path.c_str());
	ifstream input (WrapperConfigurationInfo::TemplatePath("cpp_template_project.vcxproj"));
	if (!input.good()) {
		throw runtime_error ("Unable to find the cpp_template_project.vcxproj file!");
	}

	///
	/// Now, read one to the other!
	///

	while (!input.fail()) {
		string line;
		getline (input, line);

		if (line.find("<!-- CPP Files -->") != line.npos) {
			for_each (classes.begin(), classes.end(),
				add_class_file_reference ("cpp", "ClCompile", output));
			for_each (_extra_files.begin(), _extra_files.end(),
				[&output] (const string &file_name)
					{output << "<File RelativePath=\"" << file_name << "\"></File>" << endl; } );

		} else if (line.find("<!-- HPP Files -->") != line.npos) {
			for_each (classes.begin(), classes.end(),
				add_class_file_reference ("hpp", "ClInclude", output));

		} else if (line.find("<!-- ProjectGUID -->") != line.npos) {
			output << "    <ProjectGuid>{" << get_guid(library_name) << "}</ProjectGuid>" << endl;

		} else if (line.find("<!-- Name -->") != line.npos) {
			output << "Name=\"" << library_name << "Wrapper\"" << endl;

		} else if (line.find("<!-- RootNameSpace -->") != line.npos) {
			output << "<RootNamespace>" << library_name << "</RootNamespace>" << endl;

		} else if (line.find("<!-- ADDIncludes -->") != line.npos) {
			vector<string> dependent_libraries = _translator.get_dependent_libraries(library_name);
			for_each(_extra_include_dirs.begin(), _extra_include_dirs.end(),
				[&output] (const string &dir) { output << ";" << dir; });
			auto bylib = _includes_by_library.find(library_name);
			if (bylib != _includes_by_library.end()) {
				for_each(bylib->second.begin(), bylib->second.end(),
					[&output] (const string &lib) { output << ";$(SolutionDir)\\" << lib << "\\Source"; });
			}

		} else if (line.find("<!-- ADDLinkLibraries -->") != line.npos) {
			vector<string> dependent_libraries = _translator.get_dependent_libraries(library_name);
			copy (_global_link_dependencies.begin(), _global_link_dependencies.end(), back_inserter(dependent_libraries));
			for_each (dependent_libraries.begin(), dependent_libraries.end(),
				[&output] (const string &proj_name) {	output << " " << proj_name << ".lib;"; });

		} else if (line.find("<!-- ADDLinkLibraryDirectories -->") != line.npos) {
			for_each(_link_libs.begin(), _link_libs.end(),
				[&output] (const string &dir) { output << dir << ";"; });
		} else {
			output << line << endl;
		}
	}

	input.close();
	output.close();
}
