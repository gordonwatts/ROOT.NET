///
/// Implementation for a void pointer. We assume it is of type TObject and work from there
///
#include "TVoidPointer.hpp"
#include "SourceEmitter.hpp"

#include <vector>

using std::string;
using std::endl;
using std::vector;

///
/// Setup is very simple - just declare the type
///
TVoidPointer::TVoidPointer(void)
	: TypeTranslator ("ROOTNET::Interface::NTObject ^", "void*")
{
}

TVoidPointer::~TVoidPointer(void)
{
}

///
/// We are coming from the .NET world and want to go to the C++ world. In this case we need
/// only to declrae a pointer and set that up and we are "done". Recall, b/c of the way we've
/// defined this, only a TObject sort of thing is going to show up here.
///
void TVoidPointer::translate_to_cpp (const string &name_net, const string &name_cpp, SourceEmitter &emitter) const
{
	emitter.start_line() << "void *" << name_cpp << " = " << name_net << "->CPP_Instance_TObject();" << endl;
}

///
/// This is a bit more dangerous. Here we make the assumption that the void* coming back here is
/// a TObject* or better. If it isn't we are bound to go boom here. So the user must beware of this!
///
void TVoidPointer::translate_to_net(const std::string &name_net, const std::string &name_cpp, SourceEmitter &emitter, bool use_interface, bool is_static) const
{
	emitter.start_line() << "auto " << name_net << " = ROOTNET::Utility::ROOTObjectServices::GetBestObject<ROOTNET::Interface::NTObject^>(static_cast<::TObject*>(" << name_cpp << "));" << endl;
}

///
/// void* - but we still need TObject around. So make sure that makes it onto the list of referenced types!
///
vector<string> TVoidPointer::referenced_root_types(void) const
{
	vector<string> result;
	result.push_back("TObject");
	return result;
}
