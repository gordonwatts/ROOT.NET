///
/// Write out code to be attached to a TTree so that we can go after a
/// a tree's items.
///

#include "FeatureTreeIterator.hpp"
#include "RootClassInfo.hpp"
#include "SourceEmitter.hpp"

#include <vector>
#include <string>

using std::vector;
using std::string;
using std::endl;

///
/// We want to work on any class that is a TTree.
///
bool FeatureTreeIterator::is_applicable (const RootClassInfo &info)
{
	return info.CPPQualifiedName() == "TTree";
}

///
/// Add enumerability to this, so we can do foreach and the like.
///
std::vector<std::string> FeatureTreeIterator::get_additional_interfaces (const RootClassInfo &info)
{
	vector<string> result;
	result.push_back("System::Collections::Generic::IEnumerable<ROOTNET::Utility::TreeEntry ^>");
	return result;
}

///
/// Emit the header definitions for the enumerable guys!
///
void FeatureTreeIterator::emit_header_method_definitions (const RootClassInfo &info, SourceEmitter &emitter)
{
	emitter.start_line() << "virtual System::Collections::IEnumerator ^GetEnumerator2() sealed = System::Collections::IEnumerable::GetEnumerator" << endl;
	emitter.start_line() << "  { return GetEnumerator(); }" << endl;
	emitter.start_line() << "virtual System::Collections::Generic::IEnumerator<ROOTNET::Utility::TreeEntry^> ^GetEnumerator();" << endl;
}

///
/// Emit the class we are going to use along with the methods that return it
///
void FeatureTreeIterator::emit_class_methods (const RootClassInfo &info, SourceEmitter &emitter)
{
	emitter.start_line() << "System::Collections::Generic::IEnumerator<ROOTNET::Utility::TreeEntry^> ^NTTree::GetEnumerator()" << endl;
	emitter.start_line() << "  { return gcnew ROOTNET::Utility::TreeEntryEnumerator (_instance); }" << endl;
	emitter() << endl;
}
