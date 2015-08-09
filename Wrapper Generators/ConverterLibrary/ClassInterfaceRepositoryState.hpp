#pragma once
///
/// ClassInterfaceRepositoryState
///
///  Keeps track of what classes have been written out and what ones need to be written out.
///

#include <string>
#include <set>

class ClassInterfaceRepositoryState
{
public:
	ClassInterfaceRepositoryState(void);

	/// Request this class to be translated. If it already has, then ignore the request.
	void request_class_translation (const std::string &class_name);
	void request_enum_translation (const std::string &enum_name);

	/// Register a class - if we want to assume it has been already translated via some
	/// other magic.
	void register_class_translation (const std::string &class_name);
	void register_enum_translation (const std::string &enum_name);

	/// Return the name of the next class to translate
	std::string next_class (void);
	std::string next_enum (void);

	/// Return true if there are any classes to translate
	bool classes_to_translate (void) const;
	bool enums_to_translate (void) const;

private:
	std::set<std::string> _classes_to_translate;
	std::set<std::string> _classes_translated;

	std::set<std::string> _enums_to_translate;
	std::set<std::string> _enums_translated;
};
