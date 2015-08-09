#include "RootClassInfoCollection.hpp"
#include "RootClassInfo.hpp"

#include <map>
#include <string>

using std::string;
using std::map;
using std::set;

namespace {
  typedef map<string, RootClassInfo*> t_class_map;
  t_class_map g_known_classes;
  set<string> g_bad_method_names;
}

///
/// Get an object. If it doesn't yet exist, then create it.
///
RootClassInfo &RootClassInfoCollection::GetRootClassInfo (const std::string &class_name)
{
  return *GetRootClassInfoPtr(class_name);
}

///
/// Return a pointer to the RootClassInfo object
///
RootClassInfo *RootClassInfoCollection::GetRootClassInfoPtr (const std::string &class_name)
{
  t_class_map::iterator itr = g_known_classes.find(class_name);
  if (itr != g_known_classes.end()) {
	return itr->second;
  }
  RootClassInfo *result = new RootClassInfo(class_name);
  result->set_bad_method_names(g_bad_method_names);
  g_known_classes[class_name] = result;
  return result;
}

///
/// Clear everything out!
///
void RootClassInfoCollection::Reset()
{
  for (t_class_map::iterator itr = g_known_classes.begin(); itr != g_known_classes.end(); itr++) {
	delete itr->second;
	itr->second = 0;
  }
 g_known_classes.clear();
 g_bad_method_names.clear();
}

///
/// Remember the list of bad methods!
///
void RootClassInfoCollection::SetBadMethods(const set<string> &bad)
{
  g_bad_method_names = bad;
}

