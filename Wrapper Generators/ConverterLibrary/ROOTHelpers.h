#pragma once
#include "RootGlobalVariable.hpp"

#include <string>
#include <vector>
#include <map>

class ROOTHelpers
{
public:
	static std::vector<std::string> GetAllClassesInLibraries (const std::vector<std::string> &library_names);

	static std::string GetClassLibraryName (const std::string &class_name);
	static void ForceClassLibraryname (const std::string &class_name, const std::string &library_name);

	static bool IsClass (const std::string &class_name);
	static bool IsEnum (const std::string &enum_name);

	static std::map<std::string, std::vector<RootGlobalVariable> > GetAllGlobals (void);

	/// Return all enums sorted by type/value.
	static std::vector<std::string> GetAllEnums (void);
	static const std::vector<std::pair<std::string, int> > &GetEnumValues (const std::string &enum_type);

	/// Returns template arguments
	static std::vector<std::string> GetTemplateArguments (const std::string &template_type);

	/// Get all headers we are dealing with, or get all headers that exist
	static const std::vector<std::string> &GetAllHeaders (void);
	static const std::vector<std::string> &GetAllExistingHeaders (void);

private:
	static std::map<std::string, std::vector<std::pair<std::string, int> > > _all_enums;
	static std::vector<std::string> _all_headers;
	static std::vector<std::string> _all_existing_headers;
	static void LoadAllEnums();
};
