#pragma once

///
/// Helper class to do the work of generating the project
/// files.
///

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <set>

class ClassTranslator;

class create_project_files
{
public:
	inline create_project_files (const std::string &proj_dir, const ClassTranslator &trans,
		const std::vector<std::string> &global_link_dep, 
		const std::vector<std::string> &extra_files,
		const std::vector<std::string> &extra_include_dirs,
		const std::map<std::string, std::set<std::string> > &includes_by_library,
		const std::vector<std::string> &link_libs)
		: _base_dir (proj_dir), _translator(trans), _global_link_dependencies(global_link_dep), _extra_files (extra_files),
		_extra_include_dirs(extra_include_dirs), _link_libs(link_libs), _includes_by_library(includes_by_library)
	{}
	void operator() (const std::pair<std::string, std::vector<std::string> > &library_classes);
	std::vector<std::pair<std::string, std::string> > ProjectGuids (void) const {return _project_guid;}
	inline void add_guids (const std::vector<std::pair<std::string, std::string> > &guids)
	{
		std::copy (guids.begin(), guids.end(), std::back_inserter(_project_guid));
	}
private:
	std::string get_guid (const std::string &libname);

	const std::string _base_dir;
	const std::vector<std::string> &_extra_files;
	std::vector<std::pair<std::string, std::string> > _project_guid;
	const ClassTranslator &_translator;
	const std::vector<std::string> &_global_link_dependencies;
	const std::vector<std::string> &_extra_include_dirs;
	const std::vector<std::string> &_link_libs;
	const std::map<std::string, std::set<std::string> > &_includes_by_library;
};

