#include "SourceEmitter.hpp"

#include <iostream>
#include <fstream>

using std::endl;
using std::string;
using std::istringstream;
using std::ofstream;
using std::ifstream;

///
/// Start by opening a file for output.
///
SourceEmitter::SourceEmitter(const string &filename)
: _file_path (filename)
{
}

SourceEmitter::~SourceEmitter (void)
{
	close();
}

///
/// If the file on disk is different from what we have in memory, then write it out. Otherwise
/// just quietly forget we even tried. :-)
///
void SourceEmitter::close(void)
{
	///
	/// First get the original and the new version paths set.
	///

	bool do_replace = true;
	ifstream input_original (_file_path.c_str());
	if (input_original.is_open()) {
		istringstream input_new(_file.str());

		bool found_difference = false;
		string line_orig, line_new;
		while (!found_difference && !input_original.eof() && !input_new.eof()) {
			getline(input_original, line_orig);
			getline(input_new, line_new);

			found_difference = line_new != line_orig;
		}

		if (!found_difference) {
			found_difference = input_original.eof() != input_new.eof();
		}

		do_replace = found_difference;
		input_original.close();
	}

	///
	/// Do the replacement if we need to.
	///

	if (do_replace) {
		ofstream output (_file_path.c_str());
		output << _file.str();
		output.close();
	}
}

///
/// Emit a new include statement. Make the assumption we are at the
/// beginning of the line.
///
void SourceEmitter::include_file (const string &filename)
{
	if (_included_files.find(filename) == _included_files.end()) {
		_file << "#include \"" << filename << "\"" << endl;
		_included_files.insert(filename);
	}
}

///
/// Open or close a brace. This also involves changing the indent depth
///
void SourceEmitter::brace_open()
{
	start_line() << "{" << endl;
	indent_increase();
}

void SourceEmitter::brace_close(bool add_semicolon)
{
	indent_decrease();
	start_line() << "}";
	if (add_semicolon) {
		_file << ";";
	}
	_file << endl;
}

///
/// Change the size of the indent
///
void SourceEmitter::indent_increase (void)
{
	_indent += "  ";
}

void SourceEmitter::indent_decrease(void)
{
	size_t s = _indent.size() - 2;
	if (s < 0) {
		_indent == "";
	} else {
		_indent = _indent.substr(0, s);
	}
}

///
/// Start a namespace
///
void SourceEmitter::start_namespace(const std::string &namespace_name)
{
	start_line() << "namespace " << namespace_name << " {" << endl;
	indent_increase();
}

///
/// Make a forward class reference
///
void SourceEmitter::forward_class_reference(const std::string &class_name)
{
	start_line() << "ref class " << class_name << ";" << endl;
}

///
/// Make a forward interface reference
///
void SourceEmitter::forward_interface_reference(const std::string &class_name)
{
	start_line() << "interface class " << class_name << ";" << endl;
}
