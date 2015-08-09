#pragma once

#include "RootClassMethod.hpp"
#include "RootClassInfo.hpp"
#include "RootEnum.hpp"
#include "RootClassProperty.hpp"
#include "RootClassField.hpp"

#include <string>
#include <set>
#include <vector>
#include <map>

class TMethod;

/// Gets (and sometimes caches) info about a class. Makes for easy access
/// and a way to pass things around uniformly.
class RootClassInfo
{
public:
	RootClassInfo(const std::string &class_name);
	RootClassInfo();
	~RootClassInfo(void);

	/// Return the name
	const std::string &CPPName (void) const {return _name;}
	/// Return the name in the .NET world, in case it is different.
	const std::string &NETName (void) const {return _netname;}

	/// The include file that this class resides in
	std::string include_filename (void) const;

	/// Returns the directory where the include directoy exists.
	std::string include_directory (void) const;

	/// The name of the include file, w/o the .h
	std::string include_filename_stub (void) const;

	/// Returns a list of ROOT classes this class inherits directly from (no deep inherritance is calculated).
	const std::vector<std::string> &GetDirectInheritedClasses (void) const;

	/// Returns the name of the class we should inherrit from if we had to choose one. Returns zero length
	/// string if this is a base class.
	std::string GetBestClassToInherrit (void) const;

	/// Removes a class for the list of inherited classes. :-)
	void RemoveInheritedClass (const std::string &class_name);

	/// Returns a list of ROOT classes this class inherits from
	const std::vector<std::string> &GetInheritedClassesDeep (void) const;

	/// Returns a list of all classes this guy references. Ouch!
	const std::vector<std::string> &GetReferencedClasses (void) const;
	const std::vector<std::string> &GetReferencedEnums (void) const;

	/// Returns a list of ROOT methods implemented by this class (no inherrited
	/// classes are examined). If "clean" is set to true then all methods
	/// involving a class we don't know about will be removed. Results are
	/// cached. Note that only the methods that are implemented by this class are
	/// returned. Even unhidden methods that are implemented by a superclass are not
	/// returned.
	const std::vector<RootClassMethod> &GetPrototypesImplementedByThisClass (bool clean = false) const;

	/// Returns a list of ROOT methods associated with this class and any class
	/// that is above it on the class chain. If clean is set to true, only
	/// those methods that can be implemented are returned. Results are cached.
	const std::vector<RootClassMethod> &GetAllPrototypesForThisClass (bool clean) const;

	/// Check to see if there is a method we know about by this name
	bool has_method (const std::string &method_name) const;

	/// Return all methods that have a particular name.
	std::vector<RootClassMethod> methods_of_name(const std::string &method_name) const;

	/// Returns a list of all ROOT fields associated with this class and any
	/// class that is above it on the class chain. if clean is set to true, only
	/// those methods that can be translated are returned. Results are cached.
	const std::vector<RootClassField> &GetAllDataFields (bool clean) const;

	/// True if this object comes from TObject
	bool InheritsFromTObject (void) const;

	/// True if the dtor of this object is not protected or private
	bool CanDelete (void) const;

	/// Set methods that this class should not allow to exist
	inline void set_bad_method_names (const std::set<std::string> &bad_method_names) {
		_bad_method_names = bad_method_names;
	}

	/// Gets the library name for this class
	std::string LibraryName (void) const;

	/// Forces the library name to be the given one - this overrides and allows us
	/// to deal with putting classes in different libraries
	void ForceLibraryName (const std::string &libraryname);

	/// Returns a list of enums that are defined within this class.
	const std::vector<RootEnum> &GetClassEnums (void) const;

	/// Returns a list of properties
	const std::vector<RootClassProperty> &GetProperties(void) const;

	/// Sometimes we need to reference somethign that requires us to
	/// force an include... from another library. We return the list here.
	std::vector<std::string> OtherReferencedLibraries() const;

private:
	mutable std::vector<std::string> _inherited_classes;
	mutable bool _inherited_good;

	mutable std::vector<std::string> _inherited_classes_deep;
	mutable bool _inherited_deep_good;

	mutable std::string _best_class_to_inherrit;
	mutable bool _best_class_to_inherrit_good;

	std::vector<RootClassMethod> RootClassInfo::GetAllPrototypesForThisClassImpl (bool clean) const;
	mutable std::vector<RootClassMethod> _methods;
	mutable bool _methods_good;
	mutable std::vector<RootClassMethod> _methods_clean;
	mutable bool _methods_clean_good;

	mutable bool _methods_implemented_good;
	mutable std::vector<RootClassMethod> _methods_implemented;
	mutable bool _methods_implemented_good_clean;
	mutable std::vector<RootClassMethod> _methods_implemented_clean;

	std::vector<RootClassField> GetAllDataFieldsForThisClassImpl () const;
	mutable bool _fields_for_class_good;
	mutable std::vector<RootClassField> _fields_for_class;

	const std::vector<RootClassField> GetFieldsImplementedByThisClass () const;
	mutable bool _fields_good;
	mutable std::vector<RootClassField> _fields;
	mutable bool _fields_clean_good;
	mutable std::vector<RootClassField> _fields_clean;

	mutable std::vector<std::string> _referenced_classes;
	mutable std::vector<std::string> _referenced_enums;
	mutable bool _referenced_classes_good;
	void init_referenced_items (void) const;

	mutable std::vector<RootClassProperty> _class_properties;
	mutable bool _class_properties_good;

	std::string _name;
	std::string _netname;

	/// TMethod::Name strings that we should never allow into our list of methods!
	std::set<std::string> _bad_method_names;

	/// Cache for enum info
	mutable bool _enum_info_valid;
	mutable std::vector<RootEnum> _enum_info;
};
