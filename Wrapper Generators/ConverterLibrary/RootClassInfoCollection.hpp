#pragma once

#include "RootClassInfo.hpp"

#include <string>
#include <set>

///
/// RootClassInfoCollection
///
///  The collection of RootClassInfo's that are being held. This caches the objects that are re-used.
///

class RootClassInfoCollection
{
public:
  static RootClassInfo &GetRootClassInfo (const std::string &class_name);
  static RootClassInfo *GetRootClassInfoPtr (const std::string &class_name);
  static void SetBadMethods (const std::set<std::string> &bad_method_names);

  static void Reset(void);
};
