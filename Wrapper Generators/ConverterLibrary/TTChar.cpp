// Do the translation for a straight char
// THis is a byte, or a normal ASCII character. We will use it as the latter .
#include "TTChar.hpp"

#include "SourceEmitter.hpp"

using std::endl;

TTChar::~TTChar(void)
{
}

///
/// Emit the code that will move from the .NET world to the CPP world
///
void TTChar::translate_to_cpp(const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const
{
	emitter.start_line() << _char_type << " " << cpp_name << " = (" << _char_type << ") " << net_name << ";" << endl;
}

///
/// Emit the code that will move from the CPP world to the .NET world
///
void TTChar::translate_to_net(const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter, bool use_interface, bool is_static) const
{
	emitter.start_line() << "::System::Char " << net_name << " = ::System::Char(" << cpp_name << ");" << endl;
}
