#pragma once
///
/// Translate a root class into a managed wrapper
///
#include "RootGlobalVariable.hpp"

#include <string>
#include <vector>
#include <set>
#include <string>
#include <map>

class RootClassInfo;
class RootClassMethod;
class SourceEmitter;
class RootEnum;

class ClassTranslator
{
public:
	ClassTranslator(const std::string &base_dir);
	~ClassTranslator(void);

	/// Reset the output dir
	inline void SetOutputDir (const std::string &output_dir) {
		_base_directory = output_dir;
	}
	inline const std::string &OutputDir(void) const {
	  return _base_directory;
	}

	/// Translate the class
	void translate (RootClassInfo &class_info);
	/// Translate an enum
	void translate (RootEnum &enum_info);
	/// Translate all global variables
	void translate_global_variables(const std::string &libname = "*");

	// How does one library depend on the rest?
	std::vector<std::string> get_dependent_libraries (const std::string &library_name) const;

	// Get a list of libraries
	std::vector<std::string> get_all_library_names() const;

	/// Write out all the make_public guys required for inter-library linking. You _have_ to do this before you shut down!
	void finalize_make_publics();

	/// Include these files in all headers we generate.
	inline void include_in_header (const std::string &include_name)
	{
	  _default_header_includes.push_back(include_name);
	}

protected:
	/// Return true if the dependent class's .hpp file should be included in the .cpp file. Base implementation is always
	/// yes...
	virtual bool emit_this_header (const RootClassInfo &class_being_wrapped, const RootClassInfo &dependent_class);
	virtual bool emit_this_enum (const RootClassInfo &class_being_wrapped, const RootEnum &dependent_class);

	virtual void emit_hpp_headers (SourceEmitter &emitter);

	virtual void emit_function_body(const RootClassMethod &method, const RootClassInfo &info, SourceEmitter &emitter);
private:
	std::string _base_directory;
	std::map<std::string, std::string> _base_directories; // Keeps track of what we do by librayr name for base directories

	std::vector<std::string> _default_header_includes;

	void generate_interface (RootClassInfo &info, SourceEmitter &emitter);
	void generate_class_header (RootClassInfo &info, SourceEmitter &emitter);
	void generate_class_methods (RootClassInfo &info, SourceEmitter &emitter);
	void generate_interface_static_methods (RootClassInfo &info, SourceEmitter &emitter);

	void emit_registration(const RootClassInfo &info, SourceEmitter &emitter, bool we_own);

	/// Remove bad classes from the inheritance list
	void clean_inheritance_list (RootClassInfo &info);
	/// If inheritanec list contains somethign we can't deal with (like a template):
	bool check_inheritance_list (const RootClassInfo &info);

	/// Keep track of library inter-dependencies.
	std::map<std::string, std::set<std::string> > _library_dependencies;

	/// Keep track of what objects are in what header
	std::map<std::string, std::set<std::string> > _objects_in_library;

	/// Load up globals that we (might or might not) translate.
	void load_globals(void);
	/// Keep a list of the globals, indexed by the type of the global.
	std::map<std::string, std::vector<RootGlobalVariable> > _globals_by_type;
	/// Does this type have any globals?
	bool type_has_globals (const std::string &type_name) const;
	/// Return all the names of the globals that have a particular type
	const std::vector<RootGlobalVariable> &list_of_globals_of_type (const std::string &type_name) const;
};
