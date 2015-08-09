///
/// Code to implement a reference to a simple type. This is a type that we don't expect to
/// have any trouble translating...
///
#include "TTSimpleReference.hpp"
#include "SourceEmitter.hpp"
#include "ConverterErrorLog.hpp"

#include <stdexcept>

using std::string;
using std::endl;
using std::runtime_error;

namespace
{
  string create_name (const string &name, const string &non_const_dec, bool is_const)
  {
    if (is_const) {
      return name;
    }
    return name + non_const_dec;
  }
}

///
/// If the type is a const reference, then we treat it as a normal .NET input variable -- and the user
/// doesn't need to put a "ref" or similar to indicate it is an in/out variable. If it isn't a const
/// variable, then we need to put the ref in.
///
TTSimpleReference::TTSimpleReference(const std::string &simple_type, bool is_const)
: TypeTranslator(create_name (simple_type, "%", is_const), simple_type + "&"), _is_const(is_const), _base_type(simple_type)
{
}

///
/// Translate from the .NET world to the CPP world. This is mearly defining a regular integer...
///
void TTSimpleReference::translate_to_cpp(const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const
{
  emitter.start_line() << _base_type << " " << cpp_name << " = " << net_name << ";" << endl;
}

///
/// And if we are doing updates, then do the update!
///
void TTSimpleReference::translate_to_cpp_cleanup(const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const
{
  if (!_is_const)
  {
    emitter.start_line() << net_name << " = " << cpp_name << ";" << endl;
  }
}

///
/// Translate back. This is werid -- really we would need to hold onto the object if this was non-const. So, since
/// I don't know what to do, I'm just going to print out a warning!
///
void TTSimpleReference::translate_to_net(const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter, bool use_interface, bool is_static) const
{
  if (!_is_const) {
    emitter.start_line() << "/// WARNING: Don't really translate a non-const reference correctly!" << endl;
    ConverterErrorLog::log_type_error (cpp_typename(), "Don't translate a non-const reference correctly.");
  }
  emitter.start_line() << _base_type << " %" << net_name << " = " << cpp_name << ";" << endl;
}
