///
/// Translates an array of strings. I would not call this
/// translation cheap. :-)
///
#include "TArrayOfChar.hpp"
#include "SourceEmitter.hpp"

#include <iostream>

using std::cout;
using std::endl;
using std::string;

///
/// Translate from the .NET world to .cpp. So the item is incoming and we need to ship it off to
/// the routine.
///
void TArrayOfChar::translate_to_cpp (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const
{
    emitter.start_line() << "ROOTNET::Utility::NetStringArrayTranslator " << cpp_name << "(" << net_name << ");" << endl;
}

///
/// The method call is done. Now fetch back the set value and set it to the .net value so it can be sent back out.
///
void TArrayOfChar::translate_to_cpp_cleanup (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const
{
  if (!_is_const) {
    emitter.start_line() << cpp_name << ".Update(" << net_name << ");" << endl;
  }
}

///
/// The int * has come back as  a return time. Do the translation here.
/// Not sure what this is supposed to mean at the moment -- so I'm going to leave this alone for
/// now -- hold nose and close eyes -- but warn...
///
void TArrayOfChar::translate_to_net (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter, bool use_interface, bool is_static) const
{
    emitter.start_line() << "ROOTNET::Utility::CPPStringArray ^" << net_name << " = gcnew ROOTNET::Utility::CPPStringArray(" << cpp_name << ");" << endl;
}

///
/// Poor use is stuck with this special array...
///
string TArrayOfChar::net_return_type_name() const
{
    return "ROOTNET::Utility::CPPStringArray ^";
}
