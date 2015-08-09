///
/// Do a simple pointer type translation. So, for example, int * will get translated this way. We, currently,
/// only allow arrays to be passed in back and forth. We can make them "const" arrays or not, depending on how
/// things are done.
///
#include "TPointerSimpleType.hpp"
#include "SourceEmitter.hpp"

#include <iostream>
#include <algorithm>

using std::cout;
using std::endl;
using std::replace;
using std::string;
using std::set;

set<string> TPointerSimpleType::_types_written_out;

/// Place some local routines in here...
namespace {
  /// Figure out the proper net type for the incoming array.
  string net_type (const string simple_type)
  {
	string result("");

	result+= "array<" + simple_type + ">^";

	return result;
  }

  string cpp_type (const string simple_type, bool is_array)
  {
	  string result("");
	  result += simple_type;

	  if (is_array)
	  {
		  result += "[]";
	  }
	  else
	  {
		  result += "*";
	  }

	  return result;
  }
}

///
/// Declare ourselves as moving arrays back and forth between .NET and C++...
///
TPointerSimpleType::TPointerSimpleType(const std::string &base_type, bool is_const, bool is_array)
	: TypeTranslator (net_type(base_type), cpp_type(base_type, is_array)), _is_const(is_const), _base_type(base_type)
  {
  }

///
/// Translate from the .NET world to .cpp. So the item is incoming and we need to ship it off to
/// the routine.
///
void TPointerSimpleType::translate_to_cpp (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const
{
  string non_const_type (_base_type);
  if (_is_const) {
	non_const_type = _base_type.substr(string("const ").length());
  }
  emitter.start_line() << "ROOTNET::Utility::NetArrayTranslator" << (_is_const ? "" : "U") << "<" + non_const_type + "> " << cpp_name << "N (" << net_name << ");" << endl;
  emitter.start_line() << non_const_type << " *" << cpp_name << " = (" << non_const_type << "*) " << cpp_name << "N;" << endl;
}

///
/// always return the pointer type
///
string TPointerSimpleType::cpp_code_typename(void) const
{
	return _base_type + "*";
}

///
/// The method call is done. Now fetch back the set value and set it to the .net value so it can be sent back out.
///
void TPointerSimpleType::translate_to_cpp_cleanup (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const
{
  if (!_is_const) {
	emitter.start_line() << cpp_name << "N.Update(" << net_name << ");" << endl;
  }
}

///
/// The int *(or whatever) has come back as  a return time. Do the translation here.
/// We translate it into a special array holder. We assume that we _DO NOT OWN THE DATA_ so if this is going
/// to cause a memory leak... then it will cause a memory leak. :(
///
void TPointerSimpleType::translate_to_net (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter, bool use_interface, bool is_static) const
{
  emitter.start_line() << net_return_type_name() << " " << net_name << " = gcnew " << net_return_type_name_object() << "(" << cpp_name << ");" << endl;
}


///
/// Poor user is stuck with this special array... Further, if it is marked as const,
/// then we pretend otherwise -- so it is up to the symantics of CPPArray for now, which is not writable
/// when this was coded up... :-)
///
string TPointerSimpleType::net_return_type_name_object() const
{
  return return_net_array_wrapper_name();
}

/// Returns the net type that shoudl be used for return statements, etc.
string TPointerSimpleType::net_return_type_name() const
{
  return net_return_type_name_object() + "^";
}

/// Return a non-const version of the type.
string TPointerSimpleType::non_const_base_type(void) const
{
  string condensed_base_type (_base_type);
  if (_is_const) {
	condensed_base_type = condensed_base_type.substr(string("const ").length());
  }
  return condensed_base_type;
}

/// Return the name of the root object that will get used by the user to access
/// this return type.
string TPointerSimpleType::return_net_array_wrapper_name(bool with_namespace) const
{
  string condensed_base_type(non_const_base_type());
  replace (condensed_base_type.begin(), condensed_base_type.end(), ' ', '_');
  string result("");
  if (with_namespace) {
	  result += "ROOTNET::Utility::";
  }
  result += "CPPArray" + condensed_base_type;
  return result;
}

/// Write out the CPPArray wrapper code for this guy.
void TPointerSimpleType::write_out_clr_types(SourceEmitter &output)
{
  const string my_net_type (return_net_array_wrapper_name(false));
  if (_types_written_out.find(my_net_type) == _types_written_out.end()) {
	_types_written_out.insert(my_net_type);
	output.start_namespace("ROOTNET");
	output.start_namespace("Utility");

	string non_const (non_const_base_type());

	output.start_line() << "public ref class " << my_net_type << endl;
	output.brace_open();
	output.start_line() << "public:" << endl;
	output.start_line() << my_net_type << "(" << _base_type << " *cpp_array)" << endl;
	output.start_line() << ": _cpp_array (cpp_array)" << endl;
	output.brace_open();
	output.brace_close();
	output.start_line()	<< "property " << non_const << " default[int]" << endl;
	output.brace_open();
	output.start_line() << non_const << " get(int index)" << endl;
	output.brace_open();
	output.start_line() << "return _cpp_array[index];" << endl;
	output.brace_close();
	output.brace_close();
	output.start_line() << "array<" << non_const << ">^ as_array(int array_size)" << endl;
	output.brace_open();
	output.start_line() << "array<" << non_const << "> ^result = gcnew array<" << non_const << ">(array_size);" << endl;
	output.start_line() << "for (int i = 0; i < array_size; i++)" << endl;
	output.brace_open();
	output.start_line() << "result[i] = _cpp_array[i];" << endl;
	output.brace_close();
	output.start_line() << "return result;" << endl;
	output.brace_close();
	output.start_line() << "private:" << endl;
	output.start_line() << _base_type << " *_cpp_array;" << endl;

	output.brace_close(true);
	output.brace_close();
	output.brace_close();
  }
}
