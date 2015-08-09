#include "TTCPPString.hpp"

#include "SourceEmitter.hpp"

using std::endl;

TTCPPString::~TTCPPString(void)
{
}

///
/// Emit the code that will move from the .NET world to the CPP world
///
void TTCPPString::translate_to_cpp(const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const
{
	emitter.start_line() << "ROOTNET::Utility::NetStringToConstCPP " << cpp_name << "(" << net_name << ");" << endl;
}

///
/// Emit the code that will move from the CPP world to the .NET world
///
void TTCPPString::translate_to_net(const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter, bool use_interface, bool is_static) const
{
  emitter.start_line() << "::System::String ^" << net_name << " = gcnew ::System::String(" << cpp_name << ");" << endl;
}
