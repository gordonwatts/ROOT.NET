///
/// Main code to help with walking the ROOT dependency chain
///

#include "ClassTraversal.h"

#include "TClass.h"
#include "TBaseClass.h"
#include "TMethod.h"
#include "TROOT.h"
#include <TDataMember.h>

#include <set>
#include <deque>
#include <iostream>
#include <stdexcept>
#include <algorithm>

using std::string;
using std::vector;
using std::set;
using std::deque;
using std::cout;
using std::find;
using std::endl;

using std::runtime_error;

///
/// FindAllRelatedClasses
///
///  Given a particular class, find all related classes that we would have to import if we wanted
/// wrap the requested class.
///
///  1) Make sure each requested class exists (throw if now)
///  2) Add it to the list
///  3) Repeat process for all inherited classes until we reach the bottom.
///
vector<string> ClassTraversal::FindAllRelatedClasses(const std::vector<std::string> &request_list)
{
	set<string> result;
	deque<string> new_objects (request_list.begin(), request_list.end());
	while (new_objects.size() > 0) {
		string object_name = new_objects.front();
		new_objects.pop_front();
		if (result.find(object_name) != result.end()) {
			continue;
		}

		result.insert (object_name);
		vector<string> subclasses = FindInheritedClasses (object_name);
		for (unsigned int i = 0; i < subclasses.size(); i++) {
			new_objects.push_back (subclasses[i]);
		}
	}

	///
	/// Done. Copy over the result to a vector for better later use.
	///

	vector<string> v_result (result.begin(), result.end());
	return v_result;
}

TClass *FindRootClass (const std::string &name)
{
	TClass *root_class = gROOT->GetClass (name.c_str());
	if (root_class == NULL) {
		throw runtime_error ("Unable to locate class " + name + " in ROOT meta-data.");
	}
	return root_class;
}


/// Get the list of inherited classes.
vector<string> ClassTraversal::FindInheritedClasses(const std::string &class_name, bool protected_ok)
{
	TClass *root_class = FindRootClass (class_name);
	TList *inherited_list = root_class->GetListOfBases();
	TIter next (inherited_list);
	vector<string> result;
	while (TBaseClass *bobj = static_cast<TBaseClass*>(next()))
	{
	  if (
		  ((bobj->Property() & kIsPrivate) == 0)
		  && (protected_ok || (!protected_ok && (bobj->Property() & kIsProtected) == 0))
			  ) {
		TClass *cl = bobj->GetClassPointer();
		string subname (cl->GetName());
		result.push_back (subname);
	  }
	}

	return result;
}

///
/// Get a list of all methods implemented for this class
///
vector<TMethod*> ClassTraversal::FindClassMethods (const std::string &class_name)
{
	TClass *root_class = FindRootClass(class_name);
	TList *all_methods = root_class->GetListOfAllPublicMethods();
	TIter next (all_methods);
	vector<TMethod*> results;
	TMethod *method;
	while ((method = static_cast<TMethod*>(next.Next()))) {
		results.push_back(method);
	}
	return results;
}

///
/// Grab all the fields for the class. Filter out the bogus ones - enum's, for example,
/// are also stored here.
///
vector<TDataMember*> ClassTraversal::FindClassFields(const string &class_name)
{
	TClass *root_class = FindRootClass(class_name);
	TList *all_public_fields = root_class->GetListOfAllPublicDataMembers();
	TIter next (all_public_fields);
	vector<TDataMember*> results;
	TDataMember *field;
	while ((field = static_cast<TDataMember*>(next.Next()))) {
		bool isenum = (field->Property() & (kIsEnum | kIsStatic)) == (kIsEnum | kIsStatic);
		bool isstatic = (field->Property() & kIsStatic) == kIsStatic;
		if (!(isenum || isstatic)) {
			results.push_back(field);
		}
	}
	return results;
}

///
/// Get a list of all protected and private methods for a class
///
vector<TMethod*> ClassTraversal::FindProtectedClassMethods (const string &class_name)
{
	TClass *root_class = FindRootClass(class_name);
	TList *all_methods = root_class->GetListOfMethods();
	TIter next (all_methods);
	vector<TMethod*> results;
	TMethod *method;
	while ((method = static_cast<TMethod*>(next.Next()))) {
		if (((method->Property() & kIsProtected) != 0) || ((method->Property() & kIsPrivate) != 0)) {
			results.push_back(method);
		}
	}
	return results;
}

///
/// Return true if there are any common classes in the two inheritance hierarchys...
///
bool ClassTraversal::SharedInheritance(const std::string &class1, const std::string &class2)
{
	std::vector<string> inherited_classes_1 (FindInheritedClassesDeep (class1));
	std::vector<string> inherited_classes_2 (FindInheritedClassesDeep (class2));

	set<string> set_for_classes_1 (inherited_classes_1.begin(), inherited_classes_1.end());

	for (unsigned int i = 0; i < inherited_classes_2.size(); i++) {
		if (set_for_classes_1.find(inherited_classes_2[i]) != set_for_classes_1.end()) {
			return true;
		}
	}

	set<string> set_for_classes_2 (inherited_classes_2.begin(), inherited_classes_2.end());

	for (unsigned int i = 0; i < inherited_classes_1.size(); i++) {
		if (set_for_classes_2.find(inherited_classes_1[i]) != set_for_classes_2.end()) {
			return true;
		}
	}

	return false;
}

///
/// Return true if there are any common classes in the two inheritance hierarchys...
///
bool ClassTraversal::ClassesInherit(const std::string &base_class, const std::string &inherited_class)
{
	std::vector<string> inherited_classes (FindInheritedClassesDeep (inherited_class));
	set<string> set_for_inherited_classes (inherited_classes.begin(), inherited_classes.end());

	return set_for_inherited_classes.find(base_class) != set_for_inherited_classes.end();
}

///
/// Return true if these two classes are common inheritance level in the stuff from high_level_class.
///
bool ClassTraversal::ClassAtSameInheritnaceBranch (const std::string &c1, const std::string &c2, const std::string &high_level_class)
{
	if (c1 == c2) {
		return true;
	}
	if (c1 == high_level_class || c2 == high_level_class) {
		return false;
	}

	///
	/// Check out the classes that inherit directly to high_level_class.
	///

	vector<string> inherited_classes (FindInheritedClasses(high_level_class));
	bool c1_found = find(inherited_classes.begin(), inherited_classes.end(), c1) != inherited_classes.end();
	bool c2_found = find(inherited_classes.begin(), inherited_classes.end(), c2) != inherited_classes.end();

	if (c1_found && c2_found) {
		return true;
	}
	if (c1_found || c2_found) {
		return false;
	}

	for (unsigned int i = 0; i < inherited_classes.size(); i++) {
		if (ClassAtSameInheritnaceBranch(c1, c2, inherited_classes[i])) {
			return true;
		}
	}

	return false;

}


///
/// Parse the list of classes deeply...
///
vector<string> ClassTraversal::FindInheritedClassesDeep(const std::string &class_name)
{
	set<string> all_classes;
	deque<string> unprocessed;
	unprocessed.push_back (class_name);

	while (unprocessed.size() > 0) {
		string c_name (unprocessed.front());
		unprocessed.pop_front();
		all_classes.insert(c_name);

		vector<string> inherited_classes (FindInheritedClasses (c_name));
		for (unsigned int i = 0; i < inherited_classes.size(); i++) {
			if (all_classes.find(inherited_classes[i]) == all_classes.end()) {
				unprocessed.push_back(inherited_classes[i]);
			}
		}
	}

	vector<string> result (all_classes.begin(), all_classes.end());
	return result;
}
