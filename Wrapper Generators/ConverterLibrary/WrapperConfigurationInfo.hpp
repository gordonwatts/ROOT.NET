#pragma once
///
/// Code to help with confiruation -- like the list of methods that shouldn't be translated or a list of classes that
/// we are not capable of doing.
///

#include <vector>
#include <string>
#include <set>
#include <map>

class RootClassInfo;
class RootClassMethodArg;
class RootClassMethod;

class WrapperConfigurationInfo
{
public:
	/// Returns a list of classes that the infrastructure isn't capable of handling.
	static std::vector<std::string> RemoveBrokenClasses(const std::vector<std::string> &class_list);

	/// A list of methods that we should skip for whatever reason.
	static std::set<std::string> GetListOfBadMethods (void);

	/// Return a list of bad fields.
	static std::set<std::string> WrapperConfigurationInfo::GetListOfBadFields();

	/// Init the type translation system....
	static void InitTypeTranslators(void);

	/// Init the list of type defs
	static void InitTypeDefs(void);

	/// List of libraries that are no good - this is if you scan a complete list of the root libraries.
	static std::vector<std::string> RemoveBadLibraries (const std::vector<std::string> &library_list);

	/// Return the libraries that are bad.
	static std::vector<std::string> GetListOfBadLibraries(void);

	/// Is this library restricted to what it is allowed to link to?
	static bool IsLibraryLinkRestricted(const std::string &library_name);

	/// What is the list it can link to? A null list means it cna't link to anything!!
	static std::vector<std::string> AllowedLibraryLinks (const std::string &library_name);

	/// What libraries are explicitly blocked?
	static std::vector<std::string> DisallowedLibraryLinks (const std::string &library_name);

	/// What needs to be checked?
	static bool CheckAllowedLibraries (const std::string &library_name);
	static bool CheckDisallowedLibraries (const std::string &library_name);

	/// Return a list of classes that violate the cross referencing rule for a given library name.
	static std::vector<std::string> BadClassLibraryCrossReference (const std::string &library_name, const std::vector<std::string> &class_list);

	/// Return a list of all root dll's we can find by scanning the root library
	static std::vector<std::string> GetAllRootDLLS (void);

	/// Fix up method arguments for "special cases"
	static void FixUpMethodArguments (const RootClassInfo *class_info, const std::string &method_name, std::vector<RootClassMethodArg> &methods);

	/// Fix up method return type for "special cases"
	static std::string FixupMethodReturnType(const RootClassInfo *class_info, const RootClassMethod *method_info, const std::string &return_typename);

	/// Return false if a particular method shouldn't be made hidden accoring to some specialized rules.
	static bool MakeMethodHidden (const RootClassMethod &method);

	/// Some properties are "bad"
	static bool CheckPropertyNameBad (const RootClassInfo *class_info, const std::string &property_name);

	/// Sets the directory where we can find templates for translation
	static void SetTemplateLocation (const std::string &dir);

	/// Given the name of a template, return the path to it that can be opened.
	static std::string TemplatePath (const std::string &template_name);

	/// Give back a list of bad global variables
	static std::vector<std::string> GetListOfBadGlobalVariables();

private:
	static std::map<std::string, std::vector<std::string> > _allowed_library_links;
	static std::map<std::string, std::vector<std::string> > _disallowed_library_links;
	static bool _allowed_library_links_ok;
	static void init_allowed_library_links(void);

	static std::string _template_directory;
};
