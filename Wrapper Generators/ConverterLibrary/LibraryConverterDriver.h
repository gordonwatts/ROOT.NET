#pragma once
///
/// Drive the translation process for a set of libraries.
///

#include <string>
#include <vector>
#include <set>

class LibraryConverterDriver
{
public:
	LibraryConverterDriver(void);
	~LibraryConverterDriver(void);

	//// Configuration

	/// Add a list of previously converted objects that we can safely use in our translation.
	void add_converted_info(const std::string &dir_path);

	/// Load this library of ROOT objects, and translate everythign in it!
	void translate_classes_in_library (const std::string &dir_path);

	/// Where should we put the solutoin and the directories with the projects for
	/// each library?
	void set_output_solution_directory (const std::string &sol_path);

	/// Write all objects into a single library rather than mutlple ones.
	void write_all_in_single_library (const std::string &library_name);

	/// Add this directory to the include path of all libraries we make. Multiple
	/// ones are allowed.
	void include_directory (const std::string &dir);

	/// Write a solution file (by default)
	void write_solution (bool dowrite);

	/// Use the header directories we find from the classes as default library headers
	void use_class_header_locations (bool use_header_locations);

	/// Set where we can find the template files for use in generating.
	void set_template_files_location (const std::string &dir_loc);

	/// Print out the error report?
	void print_error_report (bool printit);

	/// Set the version number. defaults to 0.0
	void set_version (int major, int minor);
	void set_version (const std::string &version);

	//// Run

	void translate();

private:

	bool _write_solution;
	bool _use_class_header_locations;
	bool _print_error_report;
	std::vector<std::string> _libs_to_translate;
	std::vector<std::string> _already_translated_dirs;
	std::set<std::string> _include_dirs;
	std::set<std::string> _library_dirs;
	std::string _output_dir;
	std::string _single_library_name;
	std::string _wrapper_version;
};

