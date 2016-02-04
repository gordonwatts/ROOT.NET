///
/// Implement the additonal code to add IEnumerable interface to ITter and friends.
///
#include "FeatureTIterEnumerable.hpp"
#include "RootClassInfo.hpp"
#include "SourceEmitter.hpp"

#include <algorithm>

using std::vector;
using std::string;
using std::find;
using std::endl;

FeatureTIterEnumerable::FeatureTIterEnumerable(void)
{
}


FeatureTIterEnumerable::~FeatureTIterEnumerable(void)
{
}

///
/// We want to work on any class that is TIter or sub-class of that guy.
///
bool FeatureTIterEnumerable::is_applicable (const RootClassInfo &info)
{
	if (info.CPPQualifiedName() == "TIter")
		return true;

	return false;
}

///
/// If this is the TIter interface we are making, then we want to add enumerability to it. Everyone
/// else will inherrit from it, so we don't need to do anything.
///
std::vector<std::string> FeatureTIterEnumerable::get_additional_interfaces (const RootClassInfo &info)
{
	vector<string> result;
	if (info.CPPQualifiedName() == "TIter")
		result.push_back("System::Collections::Generic::IEnumerable<Interface::NTObject^>");

	return result;
}

///
/// Emit the header definitions for the enumerable guys!
///
void FeatureTIterEnumerable::emit_header_method_definitions (const RootClassInfo &info, SourceEmitter &emitter)
{
	emitter.start_line() << "virtual System::Collections::IEnumerator ^GetEnumerator2() sealed = System::Collections::IEnumerable::GetEnumerator" << endl;
	emitter.start_line() << "  { return GetEnumerator(); }" << endl;
	emitter.start_line() << "virtual System::Collections::Generic::IEnumerator<Interface::NTObject^> ^GetEnumerator();" << endl;
}

///
/// Emit the class we are going to use along with the methods that return it
///
void FeatureTIterEnumerable::emit_class_methods (const RootClassInfo &info, SourceEmitter &emitter)
{
	emitter.start_line() << "ref class NTIterEnumerator : System::Collections::Generic::IEnumerator<Interface::NTObject^>" << endl;
	emitter.brace_open();
	emitter.start_line() << "public:" << endl;
	emitter.start_line() << "NTIterEnumerator (NTIter ^itr)" << endl;
	emitter.start_line() << "    : _iterator(itr), _cachedValue (nullptr), _done (false)" << endl;
	emitter.start_line() << "{ }" << endl << endl;

	emitter.start_line() << "~NTIterEnumerator(void)" << endl;
	emitter.start_line() << "{ }" << endl << endl;

	emitter.start_line() << "bool MoveNext()" << endl;
	emitter.brace_open();
	emitter.start_line() << "if (!_done) _cachedValue = _iterator->Next();" << endl;
	emitter.start_line() << "_done = _cachedValue == nullptr;" << endl;
	emitter.start_line() << "return !_done;" << endl;
	emitter.brace_close();
	emitter() << endl;

	emitter.start_line() << "virtual bool MoveNext2() sealed = System::Collections::IEnumerator::MoveNext" << endl;
	emitter.start_line() << "{ return MoveNext(); }" << endl << endl;

	emitter.start_line() << "property Interface::NTObject ^Current" << endl;
	emitter.brace_open();
	emitter.start_line() << "virtual Interface::NTObject ^get()" << endl;
	emitter.start_line() << "{ return _cachedValue; }" << endl;
	emitter.brace_close();
	emitter() << endl;

	emitter.start_line() << "property Object^ Current2" << endl;
	emitter.brace_open();
	emitter.start_line() << "virtual Object^ get() sealed = System::Collections::IEnumerator::Current::get" << endl;
	emitter.start_line() << "{ return _cachedValue; }" << endl;
	emitter.brace_close();
	emitter() << endl;

	emitter.start_line() << "void Reset()" << endl;
	emitter.brace_open();
	emitter.start_line() << "_done = false;" << endl;
	emitter.start_line() << "_iterator->Reset();" << endl;
	emitter.brace_close();
	emitter() << endl;

	emitter.start_line() << "virtual void Reset2() sealed = System::Collections::IEnumerator::Reset" << endl;
	emitter.start_line() << "{ Reset(); }" << endl << endl;

	emitter.start_line() << "private:" << endl;
	emitter.start_line() << "NTIter ^_iterator;" << endl;
	emitter.start_line() << "Interface::NTObject ^_cachedValue;" << endl;
	emitter.start_line() << "bool _done;" << endl;
	emitter.brace_close(true);
	emitter()<<endl;

	emitter.start_line() << "System::Collections::Generic::IEnumerator<Interface::NTObject^> ^NTIter::GetEnumerator()" << endl;
	emitter.start_line() << "  { return gcnew NTIterEnumerator (this); }" << endl;
	emitter() << endl;
}
