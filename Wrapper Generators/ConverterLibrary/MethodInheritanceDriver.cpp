///
/// Method inherritance driver. Makes it possible to track
/// what is happending with the method inherritance!
///
#include "MethodInheritanceDriver.hpp"
#include "RootClassInfoCollection.hpp"

#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <iterator>

using std::string;
using std::vector;
using std::for_each;
using std::map;
using std::set;
using std::copy;
using std::back_inserter;

MethodInheritanceDriver::MethodInheritanceDriver(void)
{
}

MethodInheritanceDriver::~MethodInheritanceDriver(void)
{
}

///
/// Return the root class info
///
const RootClassInfo &MethodInheritanceDriver::GetClassInfo(const std::string &className)
{
  if (_class_info_cache.find(className) == _class_info_cache.end()) {
    _class_info_cache[className] = RootClassInfoCollection::GetRootClassInfoPtr (className);
  }
  return *_class_info_cache[className];
}

namespace {
  struct MOLess : public std::less<RootClassMethod>
  {
    bool operator() (const RootClassMethod &left, const RootClassMethod &right) const
    {
	return left.is_less_than(right);
    }
  };

  /// Put all the methods into a set.
  class same_object_level_method_combiner {
  public:
    inline same_object_level_method_combiner (MethodInheritanceDriver &mh)
      : _mh(mh)
    {}
    inline void operator() (const string &class_name)
    {
      vector<RootClassMethod> methods (_mh.GetMethodsToImplement(class_name));

      for (int i = 0; i < methods.size(); i++) {
	set<RootClassMethod, MOLess>::iterator itr = _all_methods.find(methods[i]);
	if (itr != _all_methods.end()) {
	  _all_methods.erase(itr);
	}
	_all_methods.insert(methods[i]);
      }
    }
    inline set<RootClassMethod, MOLess> &GetResult (void)
    {
      return _all_methods;
    }
  private:
    MethodInheritanceDriver &_mh;
    set<RootClassMethod, MOLess> _all_methods;
  };
}

///
/// Get the methods that we can implement. This does all the C++ work. Ick!
///
const std::vector<RootClassMethod> MethodInheritanceDriver::GetMethodsToImplement (const std::string &name)
{
  ///
  /// We make the basic assumption that things don't change over time.
  /// So, if we have already done this, then we can just go and fetch out
  /// the results of the previous run.
  ///

  if (_class_methods_to_implement.find(name) != _class_methods_to_implement.end()) {
    return _class_methods_to_implement[name];
  }

  ///
  /// First, get a list of all the methods we are supposed to implement from our
  /// inherrited classes. The below is just going to be a list of methods. Hopefully
  /// there will be few or no collisions!
  ///

  const RootClassInfo &rci (GetClassInfo(name));
  const vector<string> &direct_inheritors (rci.GetDirectInheritedClasses());
  same_object_level_method_combiner comb (for_each(direct_inheritors.begin(), direct_inheritors.end(),
    same_object_level_method_combiner(*this)));

  ///
  /// The above gives a complete list of methods we need to implement as a large set.
  /// These are all from the base class(es) of this class. Next, we look at the methods
  /// that are being implemented by this specific class.
  ///  - Name hiding: if we implement one method with namd "A", then anything inherrited
  ///    with name "A" is no longer visible. And must be marked as "hidden".
  ///
  /// We have to do dual linear look ups to get at this (we really need a list that has different sorted
  /// ways of looking at it). So we do this in a fairly contorted fasion.
  ///

  typedef set<RootClassMethod, MOLess> setrcm;
  set<RootClassMethod, MOLess> mtable (comb.GetResult());
  vector<RootClassMethod> ourmethods (rci.GetPrototypesImplementedByThisClass(true));

  set<string> methodsnames_inherrited;
  for (setrcm::const_iterator itr = mtable.begin(); itr != mtable.end(); itr++) {
    methodsnames_inherrited.insert(itr->CPPName());
  }

  set<string> methods_to_hide;
  for (int i = 0; i < ourmethods.size(); i++) {
    if (methodsnames_inherrited.find(ourmethods[i].CPPName()) != methodsnames_inherrited.end()) {
      methods_to_hide.insert(ourmethods[i].CPPName());
    }
  }

  for (setrcm::iterator itr = mtable.begin(); itr != mtable.end(); itr++) {
    if (methods_to_hide.find(itr->CPPName()) != methods_to_hide.end()) {
      itr->SetHidden(true);
    }
  }

  ///
  /// Now that everything has been marked as hidden that is needed, we can go in and
  /// override the various methods we need to override.
  ///

  for (int i = 0; i < ourmethods.size(); i++) {
    set<RootClassMethod, MOLess>::iterator itr = mtable.find(ourmethods[i]);
    if (itr != mtable.end()) {
      mtable.erase(itr);
    }
    mtable.insert(ourmethods[i]);
  }
      //mtable.insert(ourmethods.begin(), ourmethods.end());

  vector<RootClassMethod> all_methods;
  copy(mtable.begin(), mtable.end(), back_inserter(all_methods));

  _class_methods_to_implement[name] = all_methods;

  ///
  /// Done. Return the methods!
  ///

  return _class_methods_to_implement[name];
}

