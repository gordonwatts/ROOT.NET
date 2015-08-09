///
/// Translate a reference to a simple object (like int&).
#pragma once
#include "CPPNetTypeMapper.hpp"

class TTSimpleReference : public CPPNetTypeMapper::TypeTranslator
{
public:
  /// Sometimes peopel pass refs as consts -- but there is no good reason (and it may be slower),
  /// but we will deal with it anyway - though we will make some efficiencies if that is the case...
  TTSimpleReference(const std::string &simple_type, bool is_const = false);

  /// We will have to generate some code for this!
  inline bool requires_translation (void) const {
    return true;
  }

  /// Only for return types, and we don't do tracking references for those in .NET!
  inline bool requires_translation_to_net (void) const {
      return false;
  }

  /// We are definately a reference to another object!
  inline bool is_reference_to_object (void) const
  {
    return true;
  }

  /// The base type...
  std::string cpp_core_typename() const
  {
      return _base_type;
  }

  /// We don't do references for return types...
  std::string net_return_type_name() const
  {
      return _base_type;
  }

  /// We don't need to do clean up code if our return value doesn't matter.
  bool clean_up_matters_for_return_value_only (void) const
  {
      return true;
  }

  /// Do the translation...
  void translate_to_cpp (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const;
  void translate_to_cpp_cleanup (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const;
  void translate_to_net(const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter, bool use_interface, bool is_static) const;

public:
  bool _is_const;
  const std::string _base_type;
};

