///
/// Map between enum's in C++ and NET
///
#include "TTROOTenum.hpp"
#include "SourceEmitter.hpp"

using std::string;
using std::endl;

namespace {
	string createNetName (const string &name)
	{		
		int lastID = name.rfind("::");
		if (lastID == string::npos) {
			return name;
		}

		auto ns_and_class = DetermineNetNameFromCPP(name.substr(0, lastID));
		return ns_and_class + name.substr(lastID);
	}
}

TTROOTenum::TTROOTenum(const string &enum_name)
: TypeTranslator (createNetName(enum_name), enum_name)
{
}

TTROOTenum::~TTROOTenum(void)
{
}

///
/// Always make sure we refer to the global namespace for the enums here...
///
string TTROOTenum::cpp_code_typename() const
{
	return "::" + cpp_typename();
}

///
/// Given a .NET name, translate the guy to CPP.
///
void TTROOTenum::translate_to_cpp(const std::string &name_net, const std::string &name_cpp, SourceEmitter &emitter) const
{
	emitter.start_line() << "::" << cpp_typename() << " " << name_cpp << " = (::" << cpp_typename() << ") " << name_net << ";" << endl;
}

///
/// Given a CPP name, translate back to a NET guy
///
void TTROOTenum::translate_to_net(const std::string &name_net, const std::string &name_cpp, SourceEmitter &emitter, bool use_interface, bool is_static) const
{
	emitter.start_line() << net_interface_name() << " " << name_net << " = (" << net_interface_name() << ") " << name_cpp << ";" << endl;
}

///
/// Return the interface name. For a global guy i tis in the ROOT namespace... Otherwise, it is
/// in the interface.
std::string TTROOTenum::net_interface_name (void) const
{
	if (net_typename().find("::") == net_typename().npos)
		return "ROOTNET::" + net_typename();

	return "ROOTNET::Interface::" + net_typename();
}
