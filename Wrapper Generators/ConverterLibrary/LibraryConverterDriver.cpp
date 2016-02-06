///
/// Main driver routine that takes a set of libraries, finds the classes
/// in them, and then does the translation.
///

#include "LibraryConverterDriver.h"

#include "WrapperConfigurationInfo.hpp"
#include "RootClassInfoCollection.hpp"
#include "ROOTHelpers.h"
#include "ConverterErrorLog.hpp"
#include "ClassInterfaceRepositoryState.hpp"
#include "LibraryClassTranslator.hpp"
#include "create_project_files.hpp"
#include "translation_history.hpp"

#include <TApplication.h>
#include <TSystem.h>
#include <TROOT.h>

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iterator>
#include <set>
#include <iterator>

#include "shlobj.h"

using std::vector;
using std::map;
using std::string;
using std::cout;
using std::endl;
using std::for_each;
using std::runtime_error;
using std::exception;
using std::ofstream;
using std::pair;
using std::inserter;
using std::set;
using std::make_pair;
using std::getline;
using std::ostringstream;
using std::ostream_iterator;

LibraryConverterDriver::LibraryConverterDriver(void)
	: _write_solution(true), _use_class_header_locations(false), _print_error_report(true), _wrapper_version("0.0")
{
}


LibraryConverterDriver::~LibraryConverterDriver(void)
{
}

///
/// This directory contains already translated classes.
///
void LibraryConverterDriver::add_converted_info(const std::string &dir_path)
{
	_already_translated_dirs.push_back(dir_path);
}

void LibraryConverterDriver::print_error_report(bool printit)
{
	_print_error_report = printit;
}

void LibraryConverterDriver::set_template_files_location (const string &loc)
{
	WrapperConfigurationInfo::SetTemplateLocation (loc);
}

///
/// we will scan all classes in this library and translate them!
///
void LibraryConverterDriver::translate_classes_in_library (const std::string &dir_path)
{
	///
	/// We can't have paths - well, we can, but it really confuses everything - so
	/// we do some fancy footwork here to get around the issue.
	///

	string libname (dir_path);
	auto last_seperator = dir_path.find_last_of("\\");
	string dir = "";
	if (last_seperator != dir_path.npos) {
		dir = dir_path.substr(0, last_seperator);
		libname = dir_path.substr(last_seperator+1);
	}

	///
	/// If it is a relative path, fix it up!
	///

	if (dir.find(":") == dir.npos) {
		if (dir.size() == 0) {
			dir = string(gSystem->WorkingDirectory());
		} else {
			dir = string(gSystem->WorkingDirectory()) + "\\" + dir;
		}
	}

	///
	/// Keep track of the libraries we need to track!
	///

	gSystem->AddDynamicPath(dir.c_str());
	_library_dirs.insert(_library_dirs.begin(), dir);

	///
	/// If the libname has a ".dll" on it or a ".lib" or a ".so" on it, then remove that
	///

	vector<string> bad_endings;
	bad_endings.push_back(".lib");
	bad_endings.push_back(".dll");
	bad_endings.push_back(".so");

	for_each (bad_endings.begin(), bad_endings.end(),
		[&libname] (const string &ending) {
			auto l = libname.rfind(ending);
			if (l == libname.size() - ending.size()) {
				libname = libname.substr(0, libname.size() - ending.size());
			}
	});

	_libs_to_translate.push_back(libname);
}

void LibraryConverterDriver::set_output_solution_directory (const std::string &sol_path)
{
	_output_dir = sol_path;
}

void LibraryConverterDriver::write_all_in_single_library (const std::string &library_name)
{
	_single_library_name = library_name;
}

///
/// Should we write out a solution file?
///
void LibraryConverterDriver::write_solution (bool dowrite)
{
	_write_solution = dowrite;
}

///
/// Each class in ROOT knows where its headers came from. If this is true then
/// we add these path's to the list of include directories for each library. By default
/// this is false.
///
void LibraryConverterDriver::use_class_header_locations(bool use_headers)
{
	_use_class_header_locations = use_headers;
}

void LibraryConverterDriver::include_directory(const std::string &dir)
{
	_include_dirs.insert(_include_dirs.begin(), dir);
}

namespace {
	/// Write out the guid's for a project
	void save_project_guid (const string &fname, const string &guid)
	{
		ofstream output (fname);
		if (!output.good()) {
			throw runtime_error ("Unable to open '" + fname + "' for output of project guids!");
		}

		output << guid << endl;

		output.close();
	}

	/// Load up the guid's from a file.
	string load_project_guids (const string &fname)
	{
		ifstream input (fname);
		if (!input.good()) {
			return "";
		}
		string line;
		getline(input, line);
		return line;
	}

	/// Check to see that a directory exists. If not, create it.
	void check_dir (const string &dir_name)
	{
		string full_dir = dir_name;
		if (full_dir.find(":\\") == full_dir.npos) {
			const int dir_size = 400;
			char b_current_dir[dir_size];
			int err = GetCurrentDirectoryA(dir_size, b_current_dir);
			if (err == 0) {
				throw runtime_error ("Failed to get current working directory!");
			}

			full_dir = string(b_current_dir) + "\\" + full_dir;
		}

		if (gSystem->AccessPathName(full_dir.c_str())) {
			int err = gSystem->mkdir(full_dir.c_str(), true);
			if (err != 0) {
				throw runtime_error ("Error trying to create directory " + full_dir);
			}
		}
#ifdef notnow
		int err = SHCreateDirectoryExA (NULL, full_dir.c_str(), NULL);
		if (err != ERROR_SUCCESS && err != ERROR_ALREADY_EXISTS && err != ERROR_FILE_EXISTS) {
			throw runtime_error ("Error trying to create directory " + dir_name);
		}
#endif
	}

	class write_project_reference {
	public:
		inline write_project_reference (ostream &output, const map<string, string> &project_guids)
			: _output (output), _project_guids(project_guids)
		{}
		void operator() (const string &library_name)
		{
			map<string, string>::const_iterator itr (_project_guids.find(library_name));
			if (itr == _project_guids.end()) {
				return;
			}

			_output << "    <ProjectReference";
			_output << " Include=\"..\\" << library_name << "\\" << library_name << "Wrapper.vcxproj" << "\">" << endl;
			_output << "      <Project>{" << itr->second << "}</Project>" << endl;
			_output << "    </ProjectReference>" << endl;
		}
	private:
		const map<string, string> &_project_guids;
		ostream &_output;
	};

	void write_out_assembly_reference (ostream &output, const std::string &library_name, const std::string &hint_dir)
	{
		output << "<Reference Include=\"" << library_name << "\">" << endl;
		output << "  <HintPath>" << hint_dir << "\\" << library_name << ".dll</HintPath>" << endl;
		output << "</Reference>" << endl;
	}

	class write_assembly_reference {
	public:
		inline write_assembly_reference (ostream &output, const map<string, string> &project_guids, const map<string, string> &lib_dir)
			: _output (output), _project_guids(project_guids), _lib_dirs(lib_dir)
		{}
		void operator() (const string &library_name)
		{
			map<string, string>::const_iterator itr (_project_guids.find(library_name));
			if (itr != _project_guids.end()) {
				return;
			}
			itr = _lib_dirs.find(library_name + "Wrapper");
			if (itr == _lib_dirs.end()) {
				return;
			}

			write_out_assembly_reference (_output, library_name + "Wrapper", itr->second);
		}
	private:
		const map<string, string> &_project_guids;
		const map<string, string> &_lib_dirs;
		ostream &_output;
	};

	/// Projects depend on one and the other -- we will "fill them in" -- this has to be done after we are all finished, unfortunately.
	class fill_in_project_references
	{
	public:
		fill_in_project_references (const string &proj_dir, const ClassTranslator &trans, vector<pair<string, string> > project_guids, const map<string, string> &lib_dirs, const string &lib_suffix = "Wrapper")
			: _base_dir (proj_dir), _translator(trans), _lib_dirs(lib_dirs), _lib_suffix(lib_suffix)
		{
			for (unsigned int i = 0; i < project_guids.size(); i++) {
				_project_guids[project_guids[i].first] = project_guids[i].second;
			}
		}

		void operator() (const pair<string, vector<string> > &library_classes)
		{
			const string library_name = library_classes.first;

			///
			/// Open the previously created vcxproj file and the final file.
			///

			string temp_project_path = _base_dir + "\\" + library_name + "\\" + library_name + ".vcxproj-temp";
			ifstream input (temp_project_path.c_str());

			string project_path = _base_dir + "\\" + library_name + "\\" + library_name + _lib_suffix + ".vcxproj";
			SourceEmitter output (project_path);

			string assembly_loader_header = _base_dir + "\\" + library_name + "\\assembly_loader.hpp";
			ofstream ass_loader (assembly_loader_header.c_str());

			///
			/// Now, read one to the other!
			///

			while (!input.fail()) {
				string line;
				getline (input, line);

				if (line.find("<!-- ReferenceProjects -->") != line.npos) {
					vector<string> dependent_libraries = _translator.get_dependent_libraries(library_name);
					for_each (dependent_libraries.begin(), dependent_libraries.end(), write_project_reference(output(), _project_guids));
				} else if (line.find("<!-- ADDReferencedAssemblies -->") != line.npos) {
					vector<string> dependent_libraries = _translator.get_dependent_libraries(library_name);
					for_each (dependent_libraries.begin(), dependent_libraries.end(), write_assembly_reference(output(), _project_guids, _lib_dirs));
					auto itr = _lib_dirs.find("WrapperPlumbingLibrary");
					if (itr != _lib_dirs.end()) {
						write_out_assembly_reference(output(), "WrapperPlumbingLibrary", itr->second);
					}
				} else {
					output() << line << endl;
				}
			}

			input.close();
			output.close();
			ass_loader.close();
		}
		vector<pair<string, string> > ProjectGuids (void) const {return _project_guid;}
	private:
		const string _base_dir;
		vector<pair<string, string> > _project_guid;
		const ClassTranslator &_translator;
		map<string, string> _project_guids;
		const map<string, string> &_lib_dirs;
		const string _lib_suffix;
	};

	char mytoupper (char c)
	{
	  return (char) toupper(c);
	}

	string upcase(const string &str)
	{
	  string result;
	  transform (str.begin(), str.end(), inserter(result, result.end()), &mytoupper);
	  return result;
	}

	/// Helper class to define a few lines of code for the project def in a solution file
	class output_project_defines
	{
	public:
		inline output_project_defines (ostream &output)
			: _output(output)
		{}
		void operator() (const pair<string, string> &item)
		{
			const string &project_name = item.first;
			const string guid = upcase(item.second);

			_output << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"" << project_name
				<< "\", \".\\" << project_name << "\\" << project_name << "Wrapper.vcxproj\", \"{"
				<< guid << "}\"" << endl;
			_output << "EndProject" << endl;
		}
	private:
		ostream &_output;
	};

	/// Out put the active configurations for our projects!
	class output_project_configs
	{
	public:
		inline output_project_configs (ostream &output)
			: _output(output)
		{}
		void operator() (const pair<string, string> &item)
		{
			const string &project_name = item.first;
			const string guid = upcase(item.second);

			_output << "\t\t{" << guid << "}.Release|Win32.ActiveCfg = Release|Win32" << endl;
			_output << "\t\t{" << guid << "}.Release|Win32.Build.0 = Release|Win32" << endl;
			//_output << "\t\t{" << guid << "}.Debug|Win32.ActiveCfg = Debug|Win32" << endl;
			//_output << "\t\t{" << guid << "}.Debug|Win32.Build.0 = Debug|Win32" << endl;
		}
	private:
		ostream &_output;
	};

	/// Create a solution file
	void make_solution_file (const string &output_dir, const vector<pair<string, string> > &project_guids)
	{
		string project_path = output_dir + "\\ROOT.NET Lib.sln";
		SourceEmitter output (project_path);
		ifstream input (WrapperConfigurationInfo::TemplatePath("solution_template.sln"));
		if (!input.good()) {
			throw runtime_error ("Unable to find the template file solution_template.sln");
		}

		while (!input.fail()) {
			string line;
			getline (input, line);

			if (line.find("<!-- DefProjects -->") != line.npos) {
				for_each (project_guids.begin(), project_guids.end(),
					output_project_defines(output()));
			} else if (line.find("<!-- ProjectConfig -->") != line.npos) {
				for_each (project_guids.begin(), project_guids.end(),
					output_project_configs(output()));
			} else {
				output() << line << endl;
			}
		}

		output.close();
		input.close();
	}

	///
	/// Write out the AssemblyInfo file, which contains all the versioning...
	///
	void write_assembly_info (const string &output_dir, const string &libName, const string &version_spec)
	{
		SourceEmitter ass (output_dir + "\\AssemblyInfo.cpp");
		ass.start_line() << "// Auto generated versioning info for the ROOT.NET wrapper library" << endl;

		ass.start_line() << "using namespace System;" << endl;
		ass.start_line() << "using namespace System::Reflection;" << endl;
		ass.start_line() << "using namespace System::Runtime::CompilerServices;" << endl;
		ass.start_line() << "using namespace System::Runtime::InteropServices;" << endl;
		ass.start_line() << "using namespace System::Security::Permissions;" << endl;

		ass.start_line() << "[assembly:AssemblyTitleAttribute(\"" << libName << "\")];" << endl;
		ass.start_line() << "[assembly:AssemblyDescriptionAttribute(\"ROOT.NET Wrapper library " << libName << "\")];" << endl;
		ass.start_line() << "[assembly:AssemblyCompanyAttribute(\"G. Watts/University of Washington - Auto Generated\")];" << endl;
		ass.start_line() << "[assembly:AssemblyProductAttribute(\"ROOT.NET\")];" << endl;

		ass.start_line() << "[assembly:AssemblyVersionAttribute(\"" << version_spec << ".*\")];" << endl;

		ass.start_line() << "[assembly:ComVisible(false)];" << endl;
		ass.start_line() << "[assembly:CLSCompliantAttribute(true)];" << endl;

		ass.start_line() << "[assembly:ROOTVersionAttribute("
			<< gROOT->GetSvnRevision()
			<< ", \"" << gROOT->GetSvnBranch() << "\""
			<< ", \"" << gROOT->GetSvnDate() << "\""
			<< ", \"" << gROOT->GetVersion() << "\" )];" << endl;

		ass.close();
	}

	//
	// Write out the library dependencies for later package-building reference.
	//

	void write_library_dependencies (const string &dir, const ClassTranslator &translator)
	{
		ofstream output (dir + "/library_dependencies.txt");

		auto libs = translator.get_all_library_names();
		for_each (libs.begin(), libs.end(), [&output, &translator] (const string &libname)
		{
			output << libname << ": ";
			auto dep = translator.get_dependent_libraries(libname);
			copy(dep.begin(), dep.end(), std::ostream_iterator<string> (output, " "));
			output << endl;
		});

		output.close();
	}
}

void LibraryConverterDriver::translate(void)
{
	///
	/// Start up ROOT
	///

	int nargs = 2;
	char *myargv[2];
	myargv[0] = "ROOT.NET Library Converter.exe";
	myargv[1] = "-b";
	TApplication *app = new TApplication ("ROOT.NET Library Converter", &nargs, myargv);

	///
	/// Get the type system up and running. This is mostly things like "int", "long long",
	/// and the key "char*" stuff (the last of which requires real work!).
	///

	WrapperConfigurationInfo::InitTypeTranslators();
	RootClassInfoCollection::SetBadMethods(WrapperConfigurationInfo::GetListOfBadMethods());

	///
	/// Get all the classes we are going to do now...
	///

	_libs_to_translate = WrapperConfigurationInfo::RemoveBadLibraries(_libs_to_translate);
	vector<string> all_classes = ROOTHelpers::GetAllClassesInLibraries (_libs_to_translate);

	///
	/// With all the libraries loaded up, we need to pull in the type-defs.
	///

	WrapperConfigurationInfo::InitTypeDefs();

	///
	/// Get all the enums that we are going to do as well
	///

	vector<string> all_enums = ROOTHelpers::GetAllEnums();

	///
	/// Next, remove any classes that are in the bad list.
	///

	all_classes = WrapperConfigurationInfo::RemoveBrokenClasses(all_classes);

	if (all_classes.size() == 0) {
		cout << "ERROR: No classes were requested for translation!" << endl;
		return;
	}

	///
	/// And the last thing is to look to see if this trandslation has been done
	/// before, and remove things from the class and enum list if that has been
	/// the case.
	///

	translation_history history;
	vector<string> already_translated_classes, alread_translated_enums;
	for_each(_already_translated_dirs.begin(), _already_translated_dirs.end(),
		[&history] (const string &dir) { history.load_history(dir); });

	const set<string> &prev_classes (history.previous_classes());
	const set<string> &prev_enums (history.previous_enums());

	{
		set<string> set_all_classes (all_classes.begin(), all_classes.end());
		set<string> set_all_enums (all_enums.begin(), all_enums.end());

		for_each(prev_classes.begin(), prev_classes.end(),
			[&set_all_classes] (const string &class_name) { set_all_classes.erase(class_name); }); 
		for_each(prev_enums.begin(), prev_enums.end(),
			[&set_all_enums] (const string &enum_name) { set_all_enums.erase(enum_name); }); 

		all_classes.clear();
		all_enums.clear();

		all_classes.insert(all_classes.end(), set_all_classes.begin(), set_all_classes.end());
		all_enums.insert(all_enums.end(), set_all_enums.begin(), set_all_enums.end());
	}

	///
	/// Finally, for each of those previously translated guys, load an include directory in there
	///

	for (int i = 0; i < _already_translated_dirs.size(); i++) {
		_include_dirs.insert(_include_dirs.begin(), _already_translated_dirs[i] + "\\include");
	}

	///
	/// Remove classes that violate the library linkages. This is not an issue
	/// if we are pushing everything out to a single library. Log them as classes
	/// we can't translate in the end.
	///

	if (_single_library_name.size() == 0) {
		vector<string> bad_inherit_classes;
		for_each (all_classes.begin(), all_classes.end(),
			[&bad_inherit_classes] (const string &class_name) {
				RootClassInfo &master_class (RootClassInfoCollection::GetRootClassInfo(class_name));
				if (WrapperConfigurationInfo::IsLibraryLinkRestricted(master_class.LibraryName())) {
					vector<string> inher_classes (master_class.GetInheritedClassesDeep());
					vector<string> bad_classes (WrapperConfigurationInfo::BadClassLibraryCrossReference(master_class.LibraryName(), inher_classes));
					if (bad_classes.size() > 0) {
						bad_inherit_classes.push_back(class_name);
					}
				}
		});

		for (unsigned int i = 0; i < bad_inherit_classes.size(); i++) {
			vector<string>::iterator itr;
			ConverterErrorLog::log_type_error (bad_inherit_classes[i], "Can't wrap this class because one of its inherited classes will cause a circular library dependency.");
			while ((itr = find(all_classes.begin(), all_classes.end(), bad_inherit_classes[i])) != all_classes.end())
			{
				all_classes.erase(itr);
			}
		}
	}

	cout << "Going to convert " << all_classes.size() << " classes." << endl;
	cout << "Going to convert " << all_enums.size() << " enums." << endl;

	///
	/// Create the holder that tracks all classes and enums we are translating.
	/// Queue up the ones we need to translate, and also the ones we should assume
	/// we already know about.
	///

	ClassInterfaceRepositoryState rep_state;
	for_each(all_classes.begin(), all_classes.end(),
		[&rep_state] (const string &s) { rep_state.request_class_translation(s); });
	for_each(all_enums.begin(), all_enums.end(),
		[&rep_state] (const string &s) { rep_state.request_enum_translation(s); });

	for_each(prev_classes.begin(), prev_classes.end(),
		[&rep_state] (const string &s) { rep_state.register_class_translation(s); });
	for_each(prev_enums.begin(), prev_enums.end(),
		[&rep_state] (const string &s) { rep_state.register_enum_translation(s); });

	///
	/// Next, look for class enums - enums that are defined at class level, and get this
	/// marked as things that can be translated.
	///

	for (unsigned int i = 0; i < all_classes.size(); i++) {
		auto &info = RootClassInfoCollection::GetRootClassInfo(all_classes[i]);
		auto &enums = info.GetClassEnums();
		for (unsigned ienum = 0; ienum < enums.size(); ienum++) {
			auto &enumInfo = enums[ienum];
			if (enumInfo.NameUnqualified().size() > 0) {
				rep_state.register_enum_translation(enumInfo.NameQualified());
			}
		}
	}

	///
	/// Now translate the enums. Unfortunately, we can't tell what libraries they
	/// are actually in. The good news is that they are dependency free, so we can
	/// put them where we like. When translating everything we put them in the core
	/// library. If we are doing just one library then... well, it is easy. :-)
	///

	LibraryClassTranslator translator (_output_dir);
	map<string, vector<string> > files_by_library;

	while (rep_state.enums_to_translate()) {
		string enum_name = rep_state.next_enum();
		cout << "Translating enum " << enum_name << endl;
		RootEnum info (enum_name);
		string libName = info.LibraryName();
		if (_single_library_name.size() > 0) {
			libName = _single_library_name;
		}
		string output_dir = _output_dir + "\\" + libName + "\\Source";
		files_by_library[libName].push_back(info.NameUnqualified());
		check_dir (output_dir);
		translator.SetOutputDir (output_dir);
		translator.translate (info);
	}

	///
	/// Now translate the classes. Same deal applies when we are targeting a
	/// single class!
	///

	string class_name; // So the error message makes more sense! :-)
	map<string, set<string> > other_library_includes;
	try{
		while (rep_state.classes_to_translate()) {
			class_name = rep_state.next_class();
			cout << "Translating " << class_name << endl;
			RootClassInfo &info (RootClassInfoCollection::GetRootClassInfo(class_name));

			if (_use_class_header_locations) {
				_include_dirs.insert(_include_dirs.begin(), info.include_directory());
			}

			if (_single_library_name.size() > 0) {
				info.ForceLibraryName(_single_library_name);
			}

			string libName = info.LibraryName();
			files_by_library[libName].push_back(info.source_filename_stem());

			string output_dir = _output_dir + "\\" + libName + "\\Source";
			check_dir (output_dir);
			translator.SetOutputDir (output_dir);
			translator.translate (info);
		}
	} catch (exception &e) {
		cout << "Major error processing class '" << class_name << "' - message: '" << e.what() << "'." << endl;
		throw;
	}

	///
	/// Global variables are tracked by library
	///

	for_each(files_by_library.begin(), files_by_library.end(), [&] (const pair<string, vector<string> > &info)
	{
		auto output_dir = _output_dir + "\\" + info.first + "\\Source";
		translator.SetOutputDir(output_dir);
		translator.translate_global_variables(info.first.substr(3)); // Get rid of the lib in front of the library name.
		files_by_library[info.first].push_back("Globals");
	});

	///
	/// Add some assembly info (version numbers, etc.) in an AssemblyInfo file for every library.
	///

	for_each(files_by_library.begin(), files_by_library.end(), [&] (const pair<string, vector<string> > &info)
	{
		auto output_dir = _output_dir + "\\" + info.first + "\\Source";
		write_assembly_info (output_dir, info.first, _wrapper_version);
		files_by_library[info.first].push_back("AssemblyInfo");
	});


	///
	/// Write out everything that is translated into the base directory so
	/// that if anyone wants to follow on with further translations they know
	/// what already exists.
	///

	history.save_history(_output_dir, all_classes, all_enums, files_by_library);

	//
	// Next we will need to understand the dependent libraries when we do the packaging. So write out
	// a file with that info in it.
	//

	write_library_dependencies (_output_dir, translator);

	///
	/// One trick is that we have to make sure that everyone is sharing the same
	/// C++ types - this is b/c we may need to access the C++ pointer types in one
	/// library from the other. The only way to do this is to export the types
	/// and make them exposed.
	///

	translator.finalize_make_publics();

	///
	/// Write out any .hpp files that are going to get required... Again, this
	/// is done only to deal with multiple projects.
	///

	if (files_by_library.size() > 1) {
	  string output_dir = _output_dir + "\\libCore\\Source";
	  SourceEmitter helper_output_hpp (output_dir + "/root_type_helpers.hpp");
	  SourceEmitter helper_output_cpp (output_dir + "/root_type_helpers.cpp");

	  helper_output_cpp.include_file("root_type_helpers.hpp");

	  helper_output_hpp.start_line() << "#ifndef __root_type_helpers__" << endl;
	  helper_output_hpp.start_line() << "#define __root_type_helpers__" << endl;
	  CPPNetTypeMapper::instance()->write_out_clr_type_support_files(helper_output_hpp);
	  helper_output_hpp.start_line() << "#endif" << endl;

	  helper_output_cpp.close();
	  helper_output_hpp.close();
	}

	///
	/// We want all internal symbols visibile to all the projects we are building right now. Sort
	/// of like a nice sandbox. :-) Create the "visibility" file. The source file that will give
	/// permission to every other assembly to look into this one.
	///

	if (files_by_library.size() > 1) {
		string assembly_export_file = _output_dir + "\\make_visible_to_all.cpp";
		ofstream output (assembly_export_file.c_str());
		output << "/// Generated by wrapper utilities -- make all assemblies visible to each other" << endl;
		output << "using namespace System::Runtime::CompilerServices;" << endl;
		for_each (files_by_library.begin(), files_by_library.end(),
			[&output] (const pair<string, vector<string> > &item) { output << "[assembly:InternalsVisibleTo(\"" << item.first << "Wrapper\")];" << endl;});
		output.close();
	}

	///
	/// If we are re-building this, then GUID's for all these projects already exist, and we should re-use them. So, see if we can load them up.
	///

	vector<pair<string, string> > initial_guids;
	for_each(files_by_library.begin(), files_by_library.end(),
		[this, &initial_guids] (const pair<string, vector<string> > &info) {
			string g = load_project_guids (_output_dir + "\\" + info.first + "\\guid.txt");
			if (g.size() > 0) {
				initial_guids.push_back(make_pair(info.first, g));
			}
	});

	///
	/// Next, build a solution for VS for each project we are looking at.
	/// Root has some funny dependencies in it. For example, Hist seems to depend on Matrix. So, we'll just force
	/// the issue to remain safe.
	///

	vector<string> extra_files;

	vector<string> extra_include_dirs (_include_dirs.begin(), _include_dirs.end());
	vector<string> link_library_dirs (_library_dirs.begin(), _library_dirs.end());

	create_project_files proj_maker(_output_dir, translator, _libs_to_translate, extra_files, extra_include_dirs, other_library_includes, link_library_dirs);
	proj_maker.add_guids(initial_guids);
	create_project_files result(for_each (files_by_library.begin(), files_by_library.end(), proj_maker));
	
	auto guids = result.ProjectGuids();
	for_each(files_by_library.begin(), files_by_library.end(), fill_in_project_references(_output_dir, translator, guids, history.librarys_in_dirs(), _single_library_name.size() > 0 ? "" : "Wrapper"));

	for_each (guids.begin(), guids.end(),
		[this] (const pair<string, string> &info)
	{
		save_project_guid (_output_dir + "\\" + info.first + "\\guid.txt", info.second);
	});

	if (_write_solution) {
		make_solution_file (_output_dir, result.ProjectGuids());
	}

	///
	/// Finally, dump the error summary. They are always dumped to a local file for looking...
	///

	if (_print_error_report) {
		ConverterErrorLog::dump(cout);
		cout << endl << endl << "In order of least frequent to most frequent: " << endl;
		ConverterErrorLog::dump_by_error_order(cout);
	}
	ofstream errors_out ("conversion_errors.txt");
	ConverterErrorLog::dump(errors_out);
	errors_out << endl << endl << "In order of least frequent to most frequent: " << endl;
	ConverterErrorLog::dump_by_error_order(errors_out);
	errors_out.close();

}

///
/// Set the major and minor version numbers.
///
void LibraryConverterDriver::set_version (int major, int minor)
{
	ostringstream mj;
	mj << major << "." << minor;
	_wrapper_version = mj.str();
}
void LibraryConverterDriver::set_version (const string &v)
{
	_wrapper_version = v;
}

