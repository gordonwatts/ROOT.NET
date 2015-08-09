///
/// Code to implement an iterator for TCollection. We build on the iterator that we've done
/// for TIter.
///
#include "FeatureTCollectionEnumerable.hpp"
#include "RootClassInfo.hpp"
#include "SourceEmitter.hpp"

#include <vector>
#include <string>
#include <algorithm>

using std::vector;
using std::string;
using std::endl;
using std::for_each;

FeatureTCollectionEnumerable::FeatureTCollectionEnumerable(void)
{
}


FeatureTCollectionEnumerable::~FeatureTCollectionEnumerable(void)
{
}

///
/// We want to work on any class that is TCollection or sub-class of that guy.
///
bool FeatureTCollectionEnumerable::is_applicable (const RootClassInfo &info)
{
	if (info.CPPName() == "TCollection")
		return true;

	return false;
}

///
/// If this is the TIter interface we are making, then we want to add enumerability to it. Everyone
/// else will inherrit from it, so we don't need to do anything.
///
std::vector<std::string> FeatureTCollectionEnumerable::get_additional_interfaces (const RootClassInfo &info)
{
	vector<string> result;
	if (info.CPPName() == "TCollection")
		result.push_back("System::Collections::Generic::IEnumerable<Interface::NTObject^>");

	return result;
}

///
/// Emit the header definitions for the enumerable guys!
///
void FeatureTCollectionEnumerable::emit_header_method_definitions (const RootClassInfo &info, SourceEmitter &emitter)
{
	emitter.start_line() << "virtual System::Collections::IEnumerator ^GetEnumerator2() sealed = System::Collections::IEnumerable::GetEnumerator" << endl;
	emitter.start_line() << "  { return GetEnumerator(); }" << endl;
	emitter.start_line() << "virtual System::Collections::Generic::IEnumerator<Interface::NTObject^> ^GetEnumerator();" << endl;
}

///
/// Emit the code we use to create a NTIter and use it.
///
void FeatureTCollectionEnumerable::emit_class_methods (const RootClassInfo &info, SourceEmitter &emitter)
{
	emitter.start_line() << "System::Collections::Generic::IEnumerator<Interface::NTObject^> ^" << info.NETName() << "::GetEnumerator()" << endl;
	emitter.brace_open();
	emitter.start_line() << "auto itr = gcnew ROOTNET::NTIter(this);" << endl;
	emitter.start_line() << "return itr->GetEnumerator();" << endl;
	emitter.brace_close();
	emitter() << endl;
}

///
/// We reference the TIter class - so that needs to get sucked in here so we can access it!
///
vector<string> FeatureTCollectionEnumerable::get_additional_root_class_references(const RootClassInfo &info)
{
	vector<string> result;
	result.push_back("TIter");
	return result;
}
