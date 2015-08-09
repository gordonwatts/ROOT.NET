#pragma once
///
/// Does the translation for an array of char strings
///
#include "CPPNetTypeMapper.hpp"

class TArrayOfChar : public CPPNetTypeMapper::TypeTranslator
{
public:
  inline TArrayOfChar(void)
  {
  }

  /// Whatever the type is, it should go to a char** with just default compiler conversions.
  inline TArrayOfChar(const std::string &cpp_type, bool is_const)
    : TypeTranslator ("array<System::String^>^", cpp_type), _is_const(is_const)
  {
  }

  /// We will have to generate some code for this!
  inline bool requires_translation (void) const {
    return true;
  }

  /// We are going to return a special array object...
  std::string net_return_type_name (void) const;

  /// Do the translation (and some cleanup)...
  void translate_to_cpp (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const;
  void translate_to_cpp_cleanup (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const;
  void translate_to_net (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter, bool use_interface, bool is_static) const;
private:
  bool _is_const;
};

