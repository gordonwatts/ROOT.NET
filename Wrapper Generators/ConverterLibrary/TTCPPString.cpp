#include "TTCPPString.hpp"

#include "SourceEmitter.hpp"

using namespace std;

namespace {
	string cpp_char_type(bool is_char, bool is_const, bool is_reference)
	{
		string result;
		if (is_const) {
			result += "const ";
		}
		if (is_char) {
			result += "char*";
		}
		else {
			result += "std::string";
		}
		if (is_reference) {
			result += "&";
		}

		return result;
	}
}

// ctor for this guy
TTCPPString::TTCPPString(bool is_char, bool is_const, bool is_reference)
	: TypeTranslator("::System::String ^", cpp_char_type(is_char, is_const, is_reference)),
	_is_char (is_char)
{

}

TTCPPString::~TTCPPString(void)
{
}

///
/// Emit the code that will move from the .NET world to the CPP world
///
void TTCPPString::translate_to_cpp(const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const
{
	if (_is_char) {
		emitter.start_line() << "ROOTNET::Utility::NetStringToConstCPP " << cpp_name << "(" << net_name << ");" << endl;
	}
	else {
		emitter.start_line() << "ROOTNET::Utility::NetStringToConstCPP " << cpp_name << "_s(" << net_name << ");" << endl;
		emitter.start_line() << "std::string " << cpp_name << "(" << cpp_name << "_s);" << endl;
	}
}

///
/// Emit the code that will move from the CPP world to the .NET world
///
void TTCPPString::translate_to_net(const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter, bool use_interface, bool is_static) const
{
	emitter.start_line() << "::System::String ^" << net_name << " = gcnew ::System::String(" << cpp_name;
	if (!_is_char) {
		emitter() << ".c_str()";
	}
	emitter() << ");" << endl;
}
