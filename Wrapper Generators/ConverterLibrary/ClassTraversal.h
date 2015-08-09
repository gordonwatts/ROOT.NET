#pragma once

#include <string>
#include <vector>

class TMethod;
class TDataMember;

class ClassTraversal
{
public:

	ClassTraversal(void)
	{
	}

	~ClassTraversal(void)
	{
	}

	/// True if one inherits from the other or vice versa.
	static bool SharedInheritance (const std::string &class1, const std::string &class2);

	/// Does subclass inherit from the base one?
	static bool ClassesInherit (const std::string &base_class, const std::string &inherited_class);

	/// True if c1 and c2 are at the same brach (A, B and then C: public A, B -- A and B are at same branch).
	static bool ClassAtSameInheritnaceBranch (const std::string &c1, const std::string &c2, const std::string &high_level_class);

	static std::vector<std::string> FindAllRelatedClasses (const std::vector<std::string> &all_requested_classes);
	static std::vector<std::string> FindInheritedClasses (const std::string &class_name, bool protected_inherritance_ok = false);
	static std::vector<std::string> FindInheritedClassesDeep (const std::string &class_name);

	static std::vector<TMethod*> FindClassMethods (const std::string &name);
	static std::vector<TMethod*> FindProtectedClassMethods (const std::string &name);

	static std::vector<TDataMember*> FindClassFields (const std::string &name);

};
