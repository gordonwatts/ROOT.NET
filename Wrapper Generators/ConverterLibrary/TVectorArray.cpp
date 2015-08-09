#include "TVectorArray.hpp"
#include "SourceEmitter.hpp"

#include <string>
using std::string;
using std::endl;

string build_net_type (const string &name)
{
	return string("array<") + name + ">^";
}

string build_cpp_type (const string &name)
{
	return string ("vector<") + name + ">";
}

///
/// Get ourselves setup
///
TVectorArray::TVectorArray(const string &simpleTypeName)
	: TypeTranslator(build_net_type(simpleTypeName), build_cpp_type(simpleTypeName)), _simple_name(simpleTypeName)
{
}

TVectorArray::~TVectorArray(void)
{
}

///
/// We are given an array coming in. So we must now convert it to a vector
///
void TVectorArray::translate_to_cpp (const std::string &name_net, const std::string &name_cpp, SourceEmitter &emitter) const
{
	emitter.start_line() << cpp_code_typename() << " " << name_cpp << "(" << name_net << "->Length);" << endl;;
	emitter.start_line() << "for (int index = 0; index < " << name_net << "->Length; index++)" << endl;
	emitter.start_line() << "  " << name_cpp << ".push_back(" << name_net << "[index]);" << endl;
}

///
/// Given a vertex, convert it into an array!
///
void TVectorArray::translate_to_net(const std::string &name_net, const std::string &name_cpp, SourceEmitter &emitter, bool use_interface, bool is_static) const
{
	emitter.start_line() << net_typename() << " " << name_net << " = gcnew array<" << _simple_name << ">(" << name_cpp << ".size());" << endl;
	emitter.start_line() << "for (unsigned int index = 0; index < " << name_cpp << ".size(); index++)" << endl;
	emitter.start_line() << "  " << name_net << "[index] = " << name_cpp << "[index];" << endl;
}
