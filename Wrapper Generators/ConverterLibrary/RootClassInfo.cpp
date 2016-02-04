#include "RootClassInfo.hpp"
#include "ClassTraversal.h"
#include "ROOTHelpers.h"
#include "WrapperConfigurationInfo.hpp"
#include "ConverterErrorLog.hpp"
#include "RootClassInfoCollection.hpp"
#include "FieldHandlingUtils.hpp"
#include "StringUtils.h"

#include "TMethod.h"
#include "TROOT.h"
#include "TClass.h"
#include "TDataMember.h"
#include "Api.h"

#include <vector>
#include <string>
#include <set>
#include <map>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <functional>

using std::string;
using std::vector;
using std::set;
using std::map;
using std::pair;
using std::runtime_error;
using std::cout;
using std::endl;
using std::transform;
using std::find;
using std::for_each;
using std::ostringstream;
using std::ostream_iterator;
using std::copy;
using std::back_inserter;
using std::inserter;
using std::mem_fun_ref;
using std::bind2nd;
using std::copy;
using std::copy_if;
using std::find_if;

namespace {
	/// Join a split string using the "::" as a seperator.
	template<class T>
	string join_with_namespace(T begin, T end, const string &joiner = "::")
	{
		string result("");
		while (begin != end) {
			if (!result.empty()) {
				result += joiner;
			}
			result += *begin;
			begin++;
		}
		return result;
	}
}

/// The normal initializer
RootClassInfo::RootClassInfo(const std::string &name)
: _inherited_good (false), _methods_good (false), _methods_clean_good(false),
_referenced_classes_good(false), _inherited_deep_good(false),
_enum_info_valid (false), _class_properties_good(false), _methods_implemented_good(false),
_methods_implemented_good_clean(false), _fields_good(false), _fields_clean_good(false), _fields_for_class_good(false),
_best_class_to_inherrit_good(false)
{

	// parse all the naming.
	auto cppparts = split(name, "::");
	auto netparts = vector<string>(cppparts);
	transform(netparts.begin(), netparts.end(), netparts.begin(), [](string &s) {return "N" + s; });

	// Figure out what is sub class and what is namespace.
	auto last_namespace = cppparts.begin();
	if (cppparts.size() > 1) {
		// Just see where we suddenly start getting class
		for (auto part = cppparts.begin() + 1; part != cppparts.end(); part++) {
			auto qualified_class = join_with_namespace(cppparts.begin(), part);
			auto c = TClass::GetClass(qualified_class.c_str());
			if (c != nullptr && (c->Property() & kIsClass)) {
				break;
			}
			last_namespace = part;
		}
	}

	_cpp_namespace = join_with_namespace(cppparts.begin(), last_namespace);
	_net_namespace = join_with_namespace(netparts.begin(), netparts.begin() + (last_namespace - cppparts.begin()));

	// The qualified class name can be done now that we know what is namepace and what isn't.
	// The net class name is the same as the fully qualified class name since we really aren't supporting
	// subclasses (though .NET allows them - just too much work!).
	_cpp_qualified_class_name = join_with_namespace(last_namespace, cppparts.end());
	_net_qualified_class_name = join_with_namespace(netparts.begin() + (last_namespace - cppparts.begin()), netparts.end(), "__");

	// The fully qualified name. In the case of .NET, this is the name space plus the qualified class name.
	_cpp_qualified_name = name;
	_net_qualified_name = _net_namespace + (_net_namespace.size() > 0 ? "::" : "") + _net_qualified_class_name;

	// The CPP class name. We aren't supporting nested class translation in NET for now.
	_cpp_class_name = *(cppparts.end() - 1);
	_net_class_name = _net_qualified_class_name;
}

/// Default ctor -- used only for stl containers!
RootClassInfo::RootClassInfo()
: _inherited_good (false), _methods_good (false),
_referenced_classes_good(false), _inherited_deep_good(false),
_enum_info_valid (false), _class_properties_good(false), _methods_implemented_good(false),
_methods_implemented_good_clean(false), _fields_good(false), _fields_clean_good(false), _fields_for_class_good (false),
_best_class_to_inherrit_good(false)
{
}

RootClassInfo::~RootClassInfo(void)
{
}

void GetInheritedClassesRec (string class_name, set<string> &class_list, bool deep_list)
{
	vector<string> direct_inher (ClassTraversal::FindInheritedClasses(class_name));
	for (unsigned int i = 0; i < direct_inher.size(); i++) {
		class_list.insert(direct_inher[i]);
		if (deep_list) {
			GetInheritedClassesRec (direct_inher[i], class_list, deep_list);
		}
	}
}

///
/// Returns a list of inherited classes.
///
const vector<string> &RootClassInfo::GetDirectInheritedClasses(void) const
{
	if (!_inherited_good) {
		_inherited_classes = ClassTraversal::FindInheritedClasses (CPPQualifiedName());
		_inherited_good = true;
	}
	return _inherited_classes;
}

///
/// Return the best class to inherrit from when we move to the .NET world.
/// -> base class, return the empty string
/// -> single inherritance, return it
/// -> multiple inherritance, return the class with the most methods
///
string RootClassInfo::GetBestClassToInherrit(void) const
{
	if (_best_class_to_inherrit_good)
		return _best_class_to_inherrit;
	_best_class_to_inherrit_good = true;

	auto &myParents (GetDirectInheritedClasses());
	if (myParents.size() == 0) {
		_best_class_to_inherrit = "";
		return "";
	}
	if (myParents.size() == 1) {
		_best_class_to_inherrit = myParents[0];
		return myParents[0];
	}

	// Now we have to look at how many methods are in each of the various subclasses.
	string bestClass;
	int bestNumberMethods = 0;

	for_each (myParents.begin(), myParents.end(), [&] (string superName)
	{
		auto &cSuperInfo (RootClassInfoCollection::GetRootClassInfo(superName));
		int numMethods = cSuperInfo.GetAllPrototypesForThisClass(true).size();
		if (numMethods > bestNumberMethods)
		{
			bestNumberMethods = numMethods;
			bestClass = superName;
		}
	});

	_best_class_to_inherrit = bestClass;
	return bestClass;
}

///
/// Remove a list from the inherited class list
///
void RootClassInfo::RemoveInheritedClass (const std::string &class_name)
{
	/// Make sure we have already built the list!
	GetDirectInheritedClasses();

	/// Now, if it is there, remove it.
	vector<string>::iterator itr = find(_inherited_classes.begin(), _inherited_classes.end(), class_name);
	if (itr != _inherited_classes.end()) {
		_inherited_classes.erase(itr);
	}
}

///
/// Returns a deep list of all inherited classes.
///
const vector<string> &RootClassInfo::GetInheritedClassesDeep(void) const
{
  if (!_inherited_deep_good) {
	  set<string> all_classes;
	  GetInheritedClassesRec (CPPQualifiedName(), all_classes, true);
	  _inherited_classes_deep.insert(_inherited_classes_deep.end(), all_classes.begin(), all_classes.end());
	  _inherited_deep_good = true;
  }
  return _inherited_classes_deep;
}

namespace {

  ///
  /// This helper method will fetch the methods for the named class.
  /// There is one trick we need to watch here:
  ///   when a method has default arguments we need to generate a prototype for
  ///   every single one. :-) A bummer, I know. I know.
  ///
  vector<RootClassMethod> GetPrototypesForClass (const string &class_name, const RootClassInfo *parent)
  {
	  const vector<TMethod*> &methods = ClassTraversal::FindClassMethods (class_name);

	  vector<RootClassMethod> result;
	  for (unsigned int i = 0; i < methods.size(); i++) {
		  TMethod *method = methods[i];
		  if (method != 0) {
			  result.push_back (RootClassMethod(method, parent));
			  if (method->GetNargsOpt() > 0) {
				  for (int i_opt_args = 1; i_opt_args <= method->GetNargsOpt(); i_opt_args++) {
					  result.push_back(RootClassMethod(method, parent, method->GetNargs()-i_opt_args));
				  }
			  }
		  }
	  }

	  return result;
  }

  ///
  /// This helper method will fetch the fields for the named class.
  /// One trick we need to watch out for - some of these data fields are enum
  /// definition. We want to avoid those - and drop them here right at the start!
  ///
  vector<RootClassField> GetFieldsForClass (const string &class_name, const RootClassInfo *parent)
  {
	  auto &fields = ClassTraversal::FindClassFields (class_name);
	  vector<RootClassField> result;
	  for (unsigned int i = 0; i < fields.size(); i++) {
		  result.push_back(RootClassField(fields[i]));
	  }
	  return result;
  }

  ///
  /// This helper method will fetch the protected methods for the named class.
  ///
  vector<RootClassMethod> GetProtectedPrototypesForClass (const string &class_name, const RootClassInfo *parent)
  {
	  const vector<TMethod*> &methods = ClassTraversal::FindProtectedClassMethods (class_name);

	  vector<RootClassMethod> result;
	  for (unsigned int i = 0; i < methods.size(); i++) {
		  TMethod *method = methods[i];
		  if (method != 0) {
			  result.push_back (RootClassMethod(method, parent));
		  }
	  }

	  return result;
  }
}

///
/// Given a list of methods, return another list that contains only the methods that
/// are clean to implement.
///
	vector<RootClassMethod> make_method_list_clean (const RootClassInfo &thisclass, const vector<RootClassMethod> &methods)
	{
	  vector<RootClassMethod> result;
	  for (int i = 0; i < methods.size(); i++) {
		const RootClassMethod &method (methods[i]);
		if (!method.can_be_translated()) {
		  continue;
		}

		vector<string> bad_classes (WrapperConfigurationInfo::BadClassLibraryCrossReference(thisclass.LibraryName(), method.get_all_referenced_raw_types()));
		if (bad_classes.size() > 0) {
		  for (int i = 0; i < bad_classes.size(); i++) {
			ConverterErrorLog::log_type_error(bad_classes[i], "Method " + thisclass.CPPQualifiedName() + "::" + method.CPPName() + " can't be translated because it would cause an incorrect reference from this library (" + thisclass.LibraryName() + ")");
		  }
		  continue;
		}
		result.push_back(method);
	  }
	  return result;
	}


///
/// Look at the methods we know about - public ones only - and return only the methods that are
/// implemented as part of our class. If clean is true, then only return methods that are
/// "good" - that is, they can be translated.
///
const std::vector<RootClassMethod> &RootClassInfo::GetPrototypesImplementedByThisClass (bool clean) const
{
  if (!_methods_implemented_good) {

	///
	/// Get all the ROOT methods for this class.
	///

	vector<RootClassMethod> methods (GetPrototypesForClass (CPPQualifiedName(), this));

	///
	/// Now, look through them to see what class they are defined in. If ours, then keep them!
	/// Also make sure they aren't on the list of bad methods.
	///

	for (int i = 0; i < methods.size(); i++) {
	  const RootClassMethod &method (methods[i]);
	  string temp (method.ClassOfMethodDefinition());
	  if (method.ClassOfMethodDefinition() == CPPQualifiedName()
		&& (_bad_method_names.find(method.CPPName()) == _bad_method_names.end())
		&& (_bad_method_names.find(CPPQualifiedName() + "::" + method.CPPName()) == _bad_method_names.end())
		) {
		_methods_implemented.push_back(method);
	  }
	}

	_methods_implemented_good = true;
  }

  ///
  /// Do the cleaning if requested. This removes methods that we can't translate because we don't know the
  /// class or because they violate cross-library translation issues.
  ///

  if (clean) {
	if (!_methods_implemented_good_clean) {
	  _methods_implemented_clean = make_method_list_clean (*this, _methods_implemented);
	  _methods_implemented_good_clean = true;
	}
	return _methods_implemented_clean;
  } else {
	return _methods_implemented;
  }
}

namespace {
  /// Less operator for ordering class methods in a container like a set.
  /// This ordering ignores the return value of a method - so covar returns
  /// will look identical.
  struct RCMOrdering : public std::less<RootClassMethod>
  {
	bool operator() (const RootClassMethod &left, const RootClassMethod &right) const
	{
	  return left.is_less_than(right);
	}
  };
  typedef set<const RootClassMethod, RCMOrdering> method_set_covar;

  /// Less operator for ordering class methods in a container like a set.
  /// This ordering does not ignores the return value of a method - so covar returns
  /// will look different.
  struct RCMROrdering : public std::less<RootClassMethod>
  {
	bool operator() (const RootClassMethod &left, const RootClassMethod &right) const
	{
	  return left.is_less_than(right, true);
	}
  };
  typedef set<const RootClassMethod, RCMROrdering> method_set;

  class convert_methods_to_set {
  public:
	inline convert_methods_to_set(bool clean) : _clean (clean) {}

	method_set operator() (const RootClassInfo *info) const {
	  const vector<RootClassMethod> &protos(info->GetAllPrototypesForThisClass(_clean));
	  return method_set(protos.begin(), protos.end());
	}
	method_set operator() (const string &class_name) const {
	  RootClassInfo &cinfo (RootClassInfoCollection::GetRootClassInfo(class_name));
	  return this->operator ()(&cinfo);
	}
  private:
	bool _clean;
  };

  class remove_tors
  {
  public:
	bool operator() (const RootClassMethod &method)
	{
	  return !(method.IsCtor() || method.IsDtor());
	}
  };

  /// Helper function to filter a set of methods from various inherrited methods
  typedef vector<method_set> method_set_list;
  template<class T>
  method_set_list method_filter (method_set_list &methods, T &tester, bool exclude = false)
  {
	method_set_list result;
	for (method_set_list::const_iterator itr = methods.begin(); itr != methods.end(); itr++)
	{
	  method_set this_set;
	  for (method_set::const_iterator sitr = itr->begin(); sitr != itr->end(); sitr++) {
		if (tester(*sitr) == !exclude) {
		  this_set.insert(*sitr);
		}
	  }
	  result.push_back(this_set);
	}
	return result;
  }

  class change_method_parent
  {
  public:
	inline change_method_parent (const RootClassInfo *info)
	  : _info (info)
	{
	}
	inline method_set operator() (const method_set &item)
	{
	  method_set result;
	  transform (item.begin(), item.end(), std::inserter (result, result.begin()), change_method_parent(_info));
	  return result;
	}
	inline RootClassMethod operator() (const RootClassMethod &method)
	{
	  RootClassMethod mnew (method);
	  mnew.ResetParent(_info);
	  return mnew;
	}
  private:
	const RootClassInfo *_info;
  };

  /// Search through the list of methods from a list of classes and find all of them
  /// that have the same name. Return the names...
  set<string> FindCommonMethodNames (const method_set_list &mset)
  {
	set<string> used_method_names;
	set<string> duplicate_method_names;
	for (method_set_list::const_iterator itr = mset.begin(); itr != mset.end(); itr++) {
	  set<string> this_class_method_names;
	  transform (itr->begin(), itr->end(), inserter(this_class_method_names, this_class_method_names.begin()), mem_fun_ref(&RootClassMethod::CPPName));

	  std::set_intersection(this_class_method_names.begin(), this_class_method_names.end(),
		used_method_names.begin(), used_method_names.end(),
		inserter(duplicate_method_names, duplicate_method_names.begin()));
	  copy(this_class_method_names.begin(), this_class_method_names.end(),
		inserter(used_method_names, used_method_names.end()));
	}
	return duplicate_method_names;
  }

  /// Filter out only those items that have a method name in the given
  /// list.
  class methods_with_name {
  public:
	inline methods_with_name (const set<string> &method_names)
	  : _method_names(method_names)
	{
	}

	inline bool operator() (const RootClassMethod &m)
	{
	  return _method_names.find(m.CPPName()) != _method_names.end();
	}

  private:
	const set<string> &_method_names;
  };

  template<class C>
  void flatten_method_set_list (const method_set_list &set_of_methods, C &methods)
  {
	for(method_set_list::const_iterator itr = set_of_methods.begin(); itr != set_of_methods.end(); itr++) {
	  copy(itr->begin(), itr->end(), inserter(methods, methods.end()));
	}
  }
}

///
/// Retursn the list of prototypes that are visible for this class. Everything!
///

const vector<RootClassField> &RootClassInfo::GetAllDataFields(bool clean) const
{
	if (!_fields_good) {
		_fields = GetAllDataFieldsForThisClassImpl();
		_fields_good = true;
	}

	if (!clean) {
		return _fields;
	}

	if (!_fields_clean_good) {
		_fields_clean = make_field_list_clean (*this, _fields);
		_fields_clean_good = true;
	}

	return _fields_clean;

}

///
/// Returns the list of prototypes that are visible for this class. Returns
/// everything, including those that are down-level!!
///
const vector<RootClassMethod> &RootClassInfo::GetAllPrototypesForThisClass (bool clean) const
{
  ///
  /// We always start with all methods (to make sure we get name
  /// hiding, etc., correct
  ///

  if (!_methods_good) {
	_methods = GetAllPrototypesForThisClassImpl (false);
	_methods_good = true;
  }

  if (!clean) {
	return _methods;
  }

  ///
  /// Clean the methods and keep only those we know how to translate
  ///

  if (!_methods_clean_good) {
	_methods_clean = make_method_list_clean (*this, _methods);
	_methods_clean_good = true;
  }

  return _methods_clean;
}

///
/// Get the list of fields for this class.
///
vector<RootClassField> RootClassInfo::GetAllDataFieldsForThisClassImpl() const
{
	///
	/// Get all the fields for this class and the inherrited classes. Things
	/// are a bit tricky b/c we have to deal with multiple inherritance and also
	/// make sure that anything in sub-classes might be hidden. Basically, we
	/// have to implement the same resolution logic as for methods.
	///
	const vector<RootClassField> &theClassFields (GetFieldsImplementedByThisClass());
	field_set thisClass (theClassFields.begin(), theClassFields.end());

	vector<field_set> inherrited_fields;
	transform (GetDirectInheritedClasses().begin(), GetDirectInheritedClasses().end(),
		back_inserter(inherrited_fields), convert_fields_to_set());

	///
	/// Reset the owners of these fields to be this class
	///

	vector<field_set> changed_parents_all;
	transform(inherrited_fields.begin(), inherrited_fields.end(),
		back_inserter(changed_parents_all),
		change_field_parent(this));

	///
	/// Fields are a lot like methods - they hide things below. Again, we use the
	/// set to implement this - we will copy in the current set items and then
	/// go one deeper and one deeper adding them. That will only add new fields.
	/// Ouch. :-)
	///

	field_set fields_as_set;
	copy (thisClass.begin(), thisClass.end(), inserter(fields_as_set, fields_as_set.begin()));

	///
	/// We don't worry about covariant returns or anything like that - we just
	/// compress everything. Also, for now, we are ignoring public instance variables
	/// that are protected (when we run into this we will deal with it).
	///

	flatten_field_set_list(changed_parents_all, fields_as_set);

	return vector<RootClassField>(fields_as_set.begin(), fields_as_set.end());
}

///
/// Get the fields that are implemented explicitly by this class
///
const vector<RootClassField> RootClassInfo::GetFieldsImplementedByThisClass() const
{
	if (!_fields_for_class_good) {
		_fields_for_class_good = true;
		vector<RootClassField> allfields(GetFieldsForClass (CPPQualifiedName(), this));

		for (int i = 0; i < allfields.size(); i++) {
			const RootClassField &f (allfields[i]);
			if (f.ClassOfFieldDefinition() == CPPQualifiedName()) {
				_fields_for_class.push_back(f);
			}
		}
	}

	return _fields_for_class;
}

///
/// Internal imp that returns the methods (clean or not) that this class implementeds...
///
std::vector<RootClassMethod> RootClassInfo::GetAllPrototypesForThisClassImpl (bool clean) const
{
  ///
  /// Get all the basics of what we need. Fast lookup! :-)
  ///

  const vector<RootClassMethod> &thisClassProtos (GetPrototypesImplementedByThisClass(clean));
  method_set thisClass (thisClassProtos.begin(), thisClassProtos.end());

  method_set_list inherrited_methods;
  transform (GetDirectInheritedClasses().begin(), GetDirectInheritedClasses().end(),
	std::back_inserter (inherrited_methods), convert_methods_to_set(clean));

  ///
  /// First, clean up the parents of all the subclass methods
  ///

  method_set_list changed_parents_all;
  transform (inherrited_methods.begin(), inherrited_methods.end(), back_inserter(changed_parents_all), change_method_parent(this));

  ///
  /// The methods that we are going to implement for this class are "top-level". They can hide other
  /// methods below. So the first task is to get them into the list so they are there first
  /// [note: the std::set will not re-insert an item that is already in the set - the below
  /// algorithm depends on that fact].
  ///

  method_set methods_as_set;
  copy (thisClass.begin(), thisClass.end(), inserter(methods_as_set, methods_as_set.begin()));

  ///
  /// Covarient methods are evil. These are methods with the same name and same return symantics. These are not
  /// allowed in the interface symantics (but will be in C# 4.0 I think!??). But we have to remove them here.
  /// We do this by looking for any method in the current class that is also in the methods below. If we find one,
  /// we have to promote the subclass to this version, sadly.
  ///
  /// NOTE: set_intersection inserts the element from the first range into the destination.
  ///

  {
	method_set subclass_methods_for_covar;
	flatten_method_set_list (changed_parents_all, subclass_methods_for_covar);
	method_set duplicate_methods_from_subclasses, duplicate_methods_from_thisclass;
	std::set_intersection(
	  subclass_methods_for_covar.begin(), subclass_methods_for_covar.end(),
	  methods_as_set.begin(), methods_as_set.end(),
	  inserter(duplicate_methods_from_subclasses, duplicate_methods_from_subclasses.begin()),
	  RCMOrdering());
	std::set_intersection(
	  methods_as_set.begin(), methods_as_set.end(),
	  subclass_methods_for_covar.begin(), subclass_methods_for_covar.end(),
	  inserter(duplicate_methods_from_thisclass, duplicate_methods_from_subclasses.begin()),
	  RCMOrdering());
	method_set::const_iterator itr_tc, itr_sc;
	for (itr_tc = duplicate_methods_from_thisclass.begin(), itr_sc = duplicate_methods_from_subclasses.begin();
	  itr_tc != duplicate_methods_from_thisclass.end();
	  itr_tc++, itr_sc++) {
		if (!CPPNetTypeMapper::instance()->AreSameType(itr_tc->return_type(), itr_sc->return_type())) {
		  RootClassMethod sc_version (*itr_sc);
		  sc_version.SetCovarReturn(true);
		  methods_as_set.erase(*itr_tc);
		  methods_as_set.insert(sc_version);
		}
	}
  }

  ///
  /// Next job is to take care of hidden methods. These are methods that this top level class
  /// has (like SetTitle in TH1) that are the same as a subclass' method (like TNamed::SetTitle).
  /// The trick is that total name hiding occurs. However, due to C# interface symatntics, it
  /// could well be there is a version of the method not hidden. So we have to mark it explicity
  /// as such!
  ///

  set<string> methods_in_top_level_class;
  transform (methods_as_set.begin(), methods_as_set.end(), inserter(methods_in_top_level_class, methods_in_top_level_class.begin()), mem_fun_ref(&RootClassMethod::CPPName));
  method_set_list nonhidden_methods (method_filter(changed_parents_all,
	methods_with_name(methods_in_top_level_class), true));
  method_set_list hidden_methods (method_filter(changed_parents_all,
	methods_with_name(methods_in_top_level_class)));
  method_set hidden_method_list;
  flatten_method_set_list (hidden_methods, hidden_method_list);
  for (method_set::iterator itr = hidden_method_list.begin(); itr != hidden_method_list.end(); itr++) {
	  if (WrapperConfigurationInfo::MakeMethodHidden(*itr)) {
		  itr->SetHidden(true);
	  }
  }
  copy(hidden_method_list.begin(), hidden_method_list.end(), inserter(methods_as_set, methods_as_set.begin()));

  ///
  /// The same thing can happen due to protected methods. If this class has a protected method that is the same name as
  /// a method in the base class, the base class method is now rendered invisible... but we have to keep the method
  /// around, of course, and mark it as hidden
  ///

  vector<RootClassMethod> protected_methods (GetProtectedPrototypesForClass (CPPQualifiedName(), this));
  set<string> protected_method_names;
  transform(protected_methods.begin(), protected_methods.end(), inserter(protected_method_names, protected_method_names.begin()),
	mem_fun_ref(&RootClassMethod::NETName));
  method_set_list protected_subclass_methods (method_filter(nonhidden_methods, methods_with_name(protected_method_names)));
  method_set_list unprotected_subclass_methods (method_filter(nonhidden_methods, methods_with_name(protected_method_names), true));
  method_set flattened_protected_methods;
  flatten_method_set_list(protected_subclass_methods, flattened_protected_methods);
  for_each(flattened_protected_methods.begin(), flattened_protected_methods.end(),
	bind2nd(mem_fun_ref(&RootClassMethod::SetHidden), true));
  copy (flattened_protected_methods.begin(), flattened_protected_methods.end(), inserter(methods_as_set, methods_as_set.begin()));

  ///
  /// For the inherrited methods we want to kill all ctor's in the inherrited objects (and dtors, actually).
  /// Those guys are hidden by the C++ sub-classing mechanism.
  ///

  method_set_list clean_ctor_methods (method_filter(unprotected_subclass_methods, remove_tors()));

  ///
  /// If we more than one base class, and one there is a method with the same name in those base classes
  /// (that isn't in our class) the method is then hidden. However, becuase of interface symantics we
  /// we have to track it. However, it makes no sense to call it (which subclass were they expecting to
  /// call!?) so we mark it as ambiguous...
  ///

  set<string> ambig_class_names (FindCommonMethodNames(clean_ctor_methods));

  method_set_list ambig_methods (method_filter(clean_ctor_methods, methods_with_name (ambig_class_names)));
  method_set ambig_method_list;
  flatten_method_set_list (ambig_methods, ambig_method_list);
  for_each (ambig_method_list.begin(), ambig_method_list.end(), bind2nd(mem_fun_ref(&RootClassMethod::SetAmbiguous), true));
  copy (ambig_method_list.begin(), ambig_method_list.end(), inserter(methods_as_set, methods_as_set.begin()));

  method_set_list unambig_methods (method_filter(clean_ctor_methods, methods_with_name(ambig_class_names), true));

  ///
  /// The methods left in each of the inherrited guys are what the methods
  /// that we will implement.
  ///

  flatten_method_set_list (unambig_methods, methods_as_set);

  ///
  /// Done. Return the result.
  ///

  vector<RootClassMethod> methods;
  copy(methods_as_set.begin(), methods_as_set.end(), back_inserter(methods));
  return methods;
}

///
/// Returns the name of the include file that this class is associated with (or whose definition can be found in).
/// ROOT returns the name of the file with path (like hist/inc) -- so we strip that out becuase evyerthing is put into
/// a common directory for the distribution.
///
string RootClassInfo::include_filename() const
{
	TClass *c = gROOT->GetClass(CPPQualifiedName().c_str());
	string full_name (c->GetDeclFileName());
	auto index = full_name.find("/inc/");
	if (index == string::npos) {
		return full_name;
	}
	return full_name.substr (index+5);
}

///
/// Returns the directory where the include file can be found.
///
string RootClassInfo::include_directory() const
{
	TClass *c = gROOT->GetClass(CPPQualifiedName().c_str());
	string full_name (c->GetDeclFileName());

	auto index = full_name.find("/include/");
	if (index == string::npos) {
		return ".";
	}
	return full_name.substr(0, index);
}

///
/// Return the stub of the fliename -- with out the .h or .hpp (or whatever) extension.
///
string RootClassInfo::include_filename_stub() const
{
	string include_filename (include_filename());
	int dot = include_filename.find_last_of(".");
	if (dot == include_filename.npos) {
		return include_filename;
	}
	return include_filename.substr(0, dot);
}

///
/// Return the sanitized name for the filesystem - where we will write
/// out this object.
///
string RootClassInfo::source_filename_stem(void) const
{
	auto r(NETQualifiedName());
	replace(r.begin(), r.end(), ':', '_');
	return r;
}

///
/// Look through the complete interface and get every single class that we reference
/// as either the return type or as an argument.
///
const vector<string> &RootClassInfo::GetReferencedClasses() const
{
	init_referenced_items();
	return _referenced_classes;
}

///
/// Return the referenced enums
///
const vector<string> &RootClassInfo::GetReferencedEnums(void) const
{
	init_referenced_items();
	return _referenced_enums;
}

///
/// Scan and calculate referenced enums and classes
///
void RootClassInfo::init_referenced_items() const
{
	if (_referenced_classes_good) {
		return;
	}

	_referenced_classes_good = true;

	///
	/// Generate by looking through every method and looking at it. We don't
	/// care if a class we will be translating or not.
	///

	const vector<RootClassMethod> &methods (GetAllPrototypesForThisClass(true));
	set<string> all_types;
	set<string> all_enums;
	for (unsigned int i = 0; i < methods.size(); i++) {
		const RootClassMethod &method (methods[i]);

		vector<string> method_classes (method.get_all_referenced_raw_root_types());
		all_types.insert(method_classes.begin(), method_classes.end());

		vector<string> method_enums (method.get_all_referenced_enums());
		all_enums.insert(method_enums.begin(), method_enums.end());
	}

	///
	/// And get everything from the data fields
	///

	auto &fields (GetAllDataFields(true));
	for (unsigned int i = 0; i < fields.size(); i++) {
		auto &f (fields[i]);
		auto used_classes (f.get_all_referenced_root_types());
		all_types.insert(used_classes.begin(), used_classes.end());
	}

	///
	/// Copy the set over to the master list, and return it!
	///

	_referenced_classes.insert(_referenced_classes.end(), all_types.begin(), all_types.end());
	_referenced_enums.insert(_referenced_enums.end(), all_enums.begin(), all_enums.end());
}

///
/// Returns true if this class has TObject in its inheritance path
///
bool RootClassInfo::InheritsFromTObject() const
{
	const vector<string> &classes = GetInheritedClassesDeep();
	return find(classes.begin(), classes.end(), "TObject") != classes.end();
}

///
/// Returns the name of the library that is associated with the class
///
string RootClassInfo::LibraryName() const
{
	return ROOTHelpers::GetClassLibraryName (CPPQualifiedName());
}

///
/// Force the class to be in a particular library
///
void RootClassInfo::ForceLibraryName(const std::string &libname)
{
	ROOTHelpers::ForceClassLibraryname(CPPQualifiedName(), libname);
}

///
/// Check the accesability of the object -- can we get at the dtor?
///
bool RootClassInfo::CanDelete() const
{
	const vector<RootClassMethod> &methods (GetAllPrototypesForThisClass(true));
	string dtor_name = "~" + CPPClassName();
	for (unsigned int i = 0; i < methods.size(); i++) {
		const RootClassMethod &method (methods[i]);
		if (method.IsDtor() && method.CPPName() == dtor_name) {
			return true;
		}
	}
	return false;
}

class insert_enum_info_into_array
{
public:
	inline insert_enum_info_into_array (vector<RootEnum> &enum_info)
		: _enum(enum_info)
	{}
	inline void operator() (const pair<string, RootEnum> &item)
	{
		_enum.push_back(item.second);
	}
private:
	vector<RootEnum> &_enum;
};

///
/// Return all enum info for this class.
///
const vector<RootEnum> &RootClassInfo::GetClassEnums() const
{
	///
	/// If we've not scanned the class, then do it.
	///
	if (!_enum_info_valid) {
		_enum_info_valid = true;

		map<string, RootEnum> info;
		TClass *c = gROOT->GetClass(CPPQualifiedName().c_str());
		TList *member_list = c->GetListOfAllPublicDataMembers();
		for (int i = 0; i < member_list->GetEntries(); i++) {
			TDataMember *d = static_cast<TDataMember*> (member_list->At(i));
			if ((d->Property() & (kIsEnum | kIsStatic)) == (kIsEnum | kIsStatic)) {
				///
				/// Get the name of the enum... blank if it is class level. Also, remove all qualifiers
				///

				string enum_name = d->GetFullTypeName();
				int class_start = enum_name.find(CPPQualifiedName() + "::");
				if (class_start == string::npos) {
					break;
				}

				unsigned int space = enum_name.rfind(" ");
				if (space != enum_name.npos) {
					enum_name = enum_name.substr(space+1);
				}

				/// If it is a class level enum, i tis constants and shouldn't be
				/// listed at all.
				if (enum_name.find("::") == enum_name.size()-2) {
					enum_name = "";
				}

				//enum_name = enum_name.substr(class_start+2 + _name.size());
				string const_name = d->GetName();

				///
				/// Get the value
				///

				Cint::G__ClassInfo *c_info = reinterpret_cast<Cint::G__ClassInfo*>(d->GetClass()->GetClassInfo());
				long offset;
				unsigned int *addr = (unsigned int*) c_info->GetDataMember (d->GetName(), &offset).Offset();
				unsigned int value = *addr;

				///
				/// Add it to our list
				///

				if (info.find(enum_name) == info.end())
				{
					info[enum_name] = RootEnum(enum_name);
				}
				info[enum_name].add(const_name, value);
			}
		}

		///
		/// Copy them over!
		///

		for_each (info.begin(), info.end(),
			insert_enum_info_into_array (_enum_info));
	}

	return _enum_info;
}

///
/// Returns a list of all the other libraries that will need to be on our include path
/// so that any generated includes can be grabbed.
///
/// Sources of this are:
///   - class based enums
///
vector<string> RootClassInfo::OtherReferencedLibraries() const
{
	auto &enums = GetReferencedEnums();
	vector<string> libraries;
	for_each(enums.begin(), enums.end(), [&libraries, this] (const string &enum_name) {
		RootEnum aEnum (enum_name);
		if (aEnum.IsClassDefined()) {
			if (aEnum.LibraryName() != LibraryName()) {
				libraries.push_back(aEnum.LibraryName());
			}
		}
	});
	return libraries;
}

namespace {
  template <class T>
  RootClassMethod set_as_hidden_if_not_in_list(RootClassMethod source, T list)
  {
	RootClassMethod result;
	method_set::const_iterator itr = list.find(source);
	if (itr == list.end()) {
	  result = source;
	  result.SetHidden(true);
	} else {
	  result = *itr;
	}
	return result;
  }
}

///
/// GetProperties
///
///   Returns a list of properties for this class
///
const std::vector<RootClassProperty> &RootClassInfo::GetProperties(void) const
{
  ///
  /// Return a cache quick
  ///

  if (_class_properties_good) {
	return _class_properties;
  }
  _class_properties_good = true;

  ///
  /// Look at all the prototypes, look for ones that get or return an item
  /// and have no arguments at all.
  ///

  map<string, RootClassProperty> found_properties;
  set<string> method_names, getset_type_conflicts;

  const vector<RootClassMethod> &protos (GetAllPrototypesForThisClass(true));
  for (int i = 0; i < protos.size(); i++) {
	const RootClassMethod &method = protos[i];

	method_names.insert(method.NETName());

	if (method.NETName().size() <= 3) {
	  continue;
	}

	bool is_getter = false;
	bool is_setter = false;
	string return_type;

	/// Is this a getter?

	if (method.arguments().size() == 0
	  && method.return_type() != "void"
	  && method.NETName().find("Get") == 0) {
		is_getter = true;
		return_type = method.return_type();
	}

	if (method.arguments().size() == 1
	  && method.return_type() == "void"
	  && method.NETName().find("Set") == 0) {
		is_setter = true;
		return_type = method.arguments()[0].CPPTypeName();
	}

	/// Only move along if we are a setter or getter!
	if (!is_setter && !is_getter) {
	  continue;
	}

	/// If we don't know about the return type, then we should just ignore this property!
	if (!CPPNetTypeMapper::instance()->has_mapping(return_type)) {
	  continue;
	}

	/// See if there is a conflicting method around for its name, or it is marked as "bad" from
	/// the pov of the configuration

	string prop_name = method.NETName().substr(3);
	if (has_method(prop_name)
		|| TClass::GetClass(prop_name.c_str()) != 0
		|| WrapperConfigurationInfo::CheckPropertyNameBad(this, prop_name)) {
		prop_name = prop_name + "_GetSetProperty";
	}

	/// Now, record it.

	if (found_properties.find(prop_name) == found_properties.end()) {
	  found_properties[prop_name] = RootClassProperty(prop_name, return_type);
	}

	if (!CPPNetTypeMapper::instance()->AreSameType(found_properties[prop_name].raw_cpp_type(), return_type)) {
	  /// The set and getters aren't doing the same type -- they probably mean different
	  /// things. So we are going to trash this for now. Perhaps on revisit we can
	  /// figure out how to deal with this better?? Are overrides with different
	  /// types for setters allowed!?
	  getset_type_conflicts.insert(prop_name);
	}

	if (is_getter) {
	  found_properties[prop_name].SetAsGetter(method);
	}
	if (is_setter) {
	  found_properties[prop_name].SetAsSetter(method);
	}
  }

  ///
  /// Transfer to an array. If a property has the same name as an already defined method, then
  /// we can skip it.
  ///

  map<string, RootClassProperty> property_map;
  for (map<string,RootClassProperty>::const_iterator itr = found_properties.begin(); itr != found_properties.end(); itr++) {
	if (method_names.find(itr->first) == method_names.end() && getset_type_conflicts.find(itr->first) == getset_type_conflicts.end()) {
	  RootClassProperty prop (itr->second);
	  if (prop.has_type_confict() && prop.isGetter()) {
		/// Get get/set .NET translations aren't the same. Bummer. We are going to have
		/// to arbitrarily just use the Get for this guy. This happens with things like
		/// arrays. Perhaps if we can figure out how to do overrides this will "go away".
		/// If we are only a setter, then we don't want to do this (see if statement!).
		prop.ResetSetter();
	  }

	  ///
	  /// Alter the name if the name is not legal C#
	  ///

	  if (prop.name().find_first_of("1234567890") == 0) {
		prop.ChangePropertyName("A" + prop.name());
	  }

	  ///
	  /// Make sure the property doesn't have the same name as a method
	  ///

	  if (method_names.find(prop.name()) != method_names.end()) {
		continue;
	  }

	  ///
	  /// Ok -- this is a good property - remember it!
	  ///

	  property_map[prop.name()] = prop;
	}
  }

  ///
  /// Finally we have a tricky thing to deal with -- hidden properties from sub classes. For example, take TLeafL:SetMaximum and
  /// TLeaf::SetMaximum. TLeaf takes "int" as an argument, and TLeafL's takes "long long". The result is that the above code will
  /// be missing a "setter" for the Maximum property in TLeafL. This will cause problems because when it comes to code generation
  /// the "set" will be missing, though it is specified in TLeafL's interface (of TLeaf).
  ///
  /// The solution is to look at inherrited classes and pull out anything missing. To keep things working correctly, we will
  /// reference the old method as well.
  ///

  const vector<string> &parent_classes (GetDirectInheritedClasses());
  method_set set_of_class_methods (protos.begin(), protos.end());
  for (int i = 0; i < parent_classes.size(); i++) {
	RootClassInfo &cparent(RootClassInfoCollection::GetRootClassInfo(parent_classes[i]));
	const vector<RootClassProperty> properties (cparent.GetProperties());

	for (int j = 0; j < properties.size(); j++) {
	  const RootClassProperty &prop(properties[j]);
	  if (property_map.find(prop.name()) == property_map.end()) {
		/// Not even there -- totally hidden. Add it in
		RootClassProperty myprop (prop);
		if (myprop.isGetter()) {
		  myprop.SetAsGetter(set_as_hidden_if_not_in_list(*prop.getter_method(), set_of_class_methods));
		}
		if (myprop.isSetter()) {
		  myprop.SetAsSetter(set_as_hidden_if_not_in_list(*prop.setter_method(), set_of_class_methods));
		}
		property_map[prop.name()] = myprop;

		///
		/// One very bad thing can happen here, it turns out. It is possible that in an inherrited class we've
		/// created a property (i.e. TVirtualPad::GetSelection creates Selection property) and in this class we now
		/// have the Selection method (TCanvas). That conflict can't be resolved - and needs to be blocked earlire. If
		/// that happens here then we need to flag it - so we will crash.
		///

		if (method_names.find(prop.name()) != method_names.end()) {
			throw runtime_error("An inherrited class has defined the property '" 
				+ prop.name() 
				+ "'. But in this class '" + CPPQualifiedName()
				+ "' a method by the same name is defined. This collision will lead to bad C++/CLI code - please mark this property bad in WrapperConfigInfo::CheckPropertyNameBad.");
		}

	  } else {
		RootClassProperty thisprop(property_map[prop.name()]);
		if (!thisprop.isGetter() && prop.isGetter()) {
		  thisprop.SetAsGetter(set_as_hidden_if_not_in_list(*prop.getter_method(), set_of_class_methods));
		}
		if (!thisprop.isSetter() && prop.isSetter()) {
		  thisprop.SetAsSetter(set_as_hidden_if_not_in_list(*prop.getter_method(), set_of_class_methods));
		}
	  }
	}
  }

  ///
  /// Place them in the final vector, and we are done!
  ///

  for (map<string,RootClassProperty>::const_iterator itr = property_map.begin(); itr != property_map.end(); itr++) {
	_class_properties.push_back(itr->second);
  }

  ///
  /// Done!
  ///

  return _class_properties;
}

///
/// See if there exists a method with this property name
///
bool RootClassInfo::has_method (const std::string &method_name) const
{
	auto &m = this->GetAllPrototypesForThisClass(true);
	if (m.end() != std::find_if (m.begin(), m.end(),
		[&method_name] (const RootClassMethod &meth) { return meth.NETName() == method_name; }))
	{
		return true;
	}
	return false;
}

///
/// Return a list of all methods that satisfy a particular name.
///
vector<RootClassMethod> RootClassInfo::methods_of_name(const std::string &method_name) const
{
	vector<RootClassMethod> result;
	auto &m = GetAllPrototypesForThisClass(true);
	copy_if(m.begin(), m.end(), back_inserter(result), [&] (const RootClassMethod &m) { return m.NETName() == method_name;});
	return result;
}

/// Rename a full class name and namespaces correctly into the .NET world.
std::string DetermineNetNameFromCPP(const std::string &cppname)
{
	auto parts = split(cppname, "::");
	string result;
	for (auto s : parts) {
		if (result.size() > 0)
			result += "::";
		result += "N" + s;
	}
	return result;
}
