///
/// Code to deal with setting GetXXX and SetXXX as class properties.
///

#include "RootClassProperty.hpp"
#include "RootClassMethod.hpp"
#include "CPPNetTypeMapper.hpp"

using std::string;

///
/// Do we have a type conflict? This happens when the C++ return type
/// for Get and Set are the same, but the .NET arn't. For example,
/// arrays. This checks for the conflict. Doesn't do anything with it,
/// however. Someone above this pay grade must make the decision.
///
const bool RootClassProperty::has_type_confict (void) const
{
  const CPPNetTypeMapper::TypeTranslator *tt = type_translator();
  return tt->net_interface_name() != tt->net_return_type_name();
}

/// Return a type translator.
const CPPNetTypeMapper::TypeTranslator *RootClassProperty::type_translator(void) const
{
  if (!_tt) {
    _tt = CPPNetTypeMapper::instance()->get_translator_from_cpp(_property_type);
  }
  return _tt;
}

/// If we were returning this guy from a "get" operation, this is the type we'd use.
const std::string RootClassProperty::as_return_type(void) const
{
  return type_translator()->net_return_type_name();
}

/// If we were passing this as an argumet (i.e. the set) then this is what we would use
const std::string RootClassProperty::as_argument_type(void) const
{
  return type_translator()->net_interface_name();
}

/// If we wanted to use this as a property type, then this is what we would use
const std::string RootClassProperty::property_type (void) const
{
  if (isSetter()) {
    return as_argument_type();
  }
  return as_return_type();
}

/// Return the name of the getter
string RootClassProperty::getter_name(void) const
{
  return _getter_m.CPPName();
}

/// Return the name of the setter
string RootClassProperty::setter_name(void) const
{
  return _setter_m.CPPName();
}
