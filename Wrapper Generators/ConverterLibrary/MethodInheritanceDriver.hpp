///
/// The driver for tracking method inherritance.
///
#pragma once

#include "RootClassInfo.hpp"

#include <map>
#include <string>

class MethodInheritanceDriver
{
public:
  MethodInheritanceDriver(void);
  ~MethodInheritanceDriver(void);

  /// Get a the RootClassInfo. Cache to speed things up
  const RootClassInfo &GetClassInfo (const std::string &className);

  /// Get the list of methods that we should be translated.
  const std::vector<RootClassMethod> GetMethodsToImplement (const std::string &name);

private:
  std::map<std::string, RootClassInfo*> _class_info_cache;
  std::map<std::string, std::vector<RootClassMethod> > _class_methods_to_implement;
};
