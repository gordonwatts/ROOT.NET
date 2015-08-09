///
/// Deal with a type translator that handles working with a vector of objects (vector<objectName>) where objectName comes from a TObject!!
///

#include "TTROOTClassVector.hpp"
#include "CPPNetTypeMapper.hpp"
#include "SourceEmitter.hpp"

#include <string>

using std::string;
using std::endl;
using std::vector;

namespace {
	string build_cpp_name (const string &arg)
	{
		return "vector<" + arg + ">";
	}

	string build_net_name (const string &arg)
	{
		return "ROOTNET::Utility::VectorObject<" + CPPNetTypeMapper::instance()->GetNetInterfaceTypename(arg) + "> ^";
	}
}

TTROOTClassVector::TTROOTClassVector(const string &objectName)
	: TypeTranslator (build_net_name(objectName), build_cpp_name(objectName)), _object_name(objectName)
{
}

TTROOTClassVector::~TTROOTClassVector(void)
{
}

///
/// Move from the .NET world into the C++ world. For this we can't really do anything - so we throw a boom. :-)
///
void TTROOTClassVector::translate_to_cpp (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const
{
	emitter.start_line() << "throw gcnew System::NotImplementedException(\"Can't translate a vector<> into .NET\");" << endl;
	emitter.start_line() << cpp_code_typename() << " " << cpp_name << ";" << endl;
}

///
/// Generate the code so that someone in the .NET world can deal with this
/// object.
///
void TTROOTClassVector::translate_to_net (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter, bool use_interface, bool is_static) const
{
	emitter.start_line() << "auto " << net_name << " = gcnew ROOTNET::Utility::VectorObject<"
		<< CPPNetTypeMapper::instance()->GetNetInterfaceTypename(_object_name) << ">("
		<< "new VOHolderObject<" << _object_name << ">(" << cpp_name << "));" << endl;
}

///
/// Return the ROOT Types that we are going to be referencing. This makes sure that all forward declares
/// are set up ok!
///
vector<string> TTROOTClassVector::referenced_root_types(void) const
{
	vector<string> result;
	result.push_back(_object_name);
	return result;
}
