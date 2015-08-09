#pragma once
///
/// Helps with writing out to a C++ source file/header file.
/// We keep track of what is to be written in the file, and then only write it out
/// if the file has changed! We are so cool! :-)
///

#include <sstream>
#include <string>
#include <set>

class SourceEmitter
{
public:
	SourceEmitter(const std::string &filename);
	~SourceEmitter(void);

	inline std::ostream &operator() (void) {
		return _file;
	}
	inline std::ostream &start_line (void) {
		_file << _indent;
		return _file;
	}

	/// Write a # include statement.
	void include_file (const std::string &include_filename);

	/// Open a namespace and do a bracket open too!
	void start_namespace (const std::string &namespace_name);

	/// Open/close brace. Tracks indenting too
	void brace_open(void);
	void brace_close(bool add_semicolon = false);

	/// Change the indent level (done automatically by some of the thigns above)
	void indent_increase(void);
	void indent_decrease(void);

	/// Insert a forward class reference
	void forward_class_reference (const std::string &class_name);

	/// Insert a forward interface reference
	void forward_interface_reference (const std::string &class_name);

	/// close our file handle.
	void close (void);

private:
	/// Output stream.
	std::ostringstream _file;

	/// Output path
	const std::string _file_path;

	/// What the current indent is
	std::string _indent;

	std::set<std::string> _included_files;
};
