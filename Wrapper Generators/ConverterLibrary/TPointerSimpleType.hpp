#pragma once
///
/// A simple pointer type (int *).
///
///  -> We always assume that we are passing in an array -- this makes it "safe" from the point of view of
/// the .NET system coming in (and we can't tell from the method signature what is wanted, unfortunately).
/// On the way out use a special array thingy that will either do the index lookup or translate if the user
/// tells us how long the array is.
///
///  When we return an array, we have to use some sort of CLR wrapper class. Sadly, a generic
/// or template won't work. So we are also responsible for generating these things on occasion.
/// Ick.
///
#include "CPPNetTypeMapper.hpp"

#include <set>

class TPointerSimpleType : public CPPNetTypeMapper::TypeTranslator
{
public:
  inline TPointerSimpleType(void)
  {
  }

  TPointerSimpleType(const std::string &simple_type, bool is_const = false, bool array_reference = false);

  /// We will have to generate some code for this!
  inline bool requires_translation (void) const {
    return true;
  }

  /// We are going to return a special array object...
  std::string net_return_type_name (void) const;

  /// Do the translation...
  void translate_to_cpp (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const;
  void translate_to_cpp_cleanup (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const;
  void translate_to_net (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter, bool use_interface, bool is_static) const;

  /// We want to write out a small customized array accessor type
  void write_out_clr_types (SourceEmitter &output);

  /// Make sure to always return the pointer version of things
  std::string cpp_code_typename(void) const;

private:
  bool _is_const;
  const std::string _base_type;

  /// WIthout the pointer.
  std::string net_return_type_name_object (void) const;

  /// Generates the class name that will access this array in the output CLR code
  std::string return_net_array_wrapper_name (bool with_namespace = true) const;

  /// A non-const version of the type name
  std::string non_const_base_type (void) const;

  /// What classes were written out by everyone. Because there can be duplicates due to
  /// const-ness.
  static std::set<std::string> _types_written_out;
};
