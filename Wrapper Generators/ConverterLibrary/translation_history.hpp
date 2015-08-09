#pragma once
///
/// Handles the recording and read/back of the translation history.
/// A way of keeping all this code close by, even though it is used
/// in different sections.
///

#include <vector>
#include <string>
#include <set>
#include <map>

class translation_history
{
public:
	translation_history(void);
	~translation_history(void);

	void save_history (const std::string &dir,
		const std::vector<std::string> &classes_translated,
		const std::vector<std::string> &enums_translated,
		const std::map<std::string, std::vector<std::string>> &filesByLibrary);

	void load_history (const std::string &dir);

	const std::set<std::string> &previous_classes(void) const
	{ return _classes; }
	const std::set<std::string> &previous_enums(void) const
	{ return _enums; }

	const std::map<std::string, std::string> &librarys_in_dirs (void) const
	{ return _lib_to_dir; }

private:
	std::set<std::string> _enums, _classes;
	std::map<std::string, std::string> _lib_to_dir;

};

