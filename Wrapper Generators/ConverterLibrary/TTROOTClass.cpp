#include "TTROOTClass.hpp"
#include "SourceEmitter.hpp"
#include "RootClassInfo.hpp"
#include "RootClassInfoCollection.hpp"

#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>

using std::string;
using std::ostringstream;
using std::endl;
using std::runtime_error;
using std::vector;

string build_lookup_typename (const string &class_name, const string modifiers, bool is_const)
{
	string base_class_name = class_name;
	if (is_const) {
		base_class_name = "const " + base_class_name;
	}
	return base_class_name + modifiers;
}

///
/// We will translate a root class. Yes!
TTROOTClass::TTROOTClass(const string &class_name, bool is_from_tobject, const string modifiers, bool is_const)
: _class_name (class_name), _modifiers(modifiers),
_inherits_from_TObject(is_from_tobject), _is_const(is_const),
TypeTranslator (RootClassInfoCollection::GetRootClassInfo(class_name).NETName() + " ^", build_lookup_typename(class_name, modifiers, is_const))
{
}

TTROOTClass::~TTROOTClass(void)
{
}

///
/// Switch from .NET to CPP.
///
/// This doesn't really make sense if they ask for an object with no modifier, but
/// we will just do it. This means the object will get copied twice -- and won't
/// be very efficient!!! Sorry!
///
/// The key to understanding this code is that all the .NET objects hold is a pointer.
/// So "TObject" means pass a copy of the object!
///
/// There is one special case: if the pointer is null in the .NET world we need to make an
/// explicit translation to the C++ world as a 0 if this is a pointer...
///
void TTROOTClass::translate_to_cpp (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const
{
	///
	/// Do some quick checks for null objects. There are some circumstances where that is not legal to pass such
	/// a beast.
	///

	bool isSafe = false; // Do we need to check for an incomming null?
	if (_modifiers == "") {
		emitter.start_line() << "if (" << net_name << " == nullptr) "
			<< "throw gcnew System::ArgumentNullException(\"Argument " << net_name << " cannot be null.\");" << endl;
		isSafe = true;
	}

	///
	/// Now, emit the code to declare what we are translating to
	///

	emitter.start_line() << "::" << _class_name;
	if (_modifiers == "*&") { // Not clear what this means to me!!
		emitter() << "*";
	} else {
		emitter() << _modifiers;
	}
	emitter() << " " << cpp_name << " = ";

	///
	/// Special case if we are dealing with a pointer, check for null.
	///

	if (_modifiers == "") {
		emitter() << "*"; // Deref the pointer to make it a full blown object.
	} else if (_modifiers == "&") {
		emitter() << "*"; // Again, to assign a reference
	} else if (_modifiers == "*" || _modifiers == "*&") {
	} else {
		throw runtime_error ("Unknown set of modifiers - " + _modifiers);
	}

	//
	// Emit the final reference. If we are 100% sure this guy isn't going to be null, don't waste anytime
	// or space checking for that.
	//

	if (!isSafe) {
		emitter() << "(" << net_name << " == nullptr ? 0 : " << net_name << "->CPP_Instance_" << _class_name << "());" << endl;
	} else {
		emitter() << net_name << "->CPP_Instance_" << _class_name << "();" << endl;
	}
}

///
/// cpp_code_typename
///
///  Return a cpp name -- built for code. Mostly that means putting a "::" in front of it!
///
string TTROOTClass::cpp_code_typename (void) const
{
	ostringstream result;
	if (_is_const) {
		result << "const ";
	}
	result << "::" << _class_name << _modifiers;
	return result.str();
}

///
/// cpp_core_typename
///
///  The core class, without any extra stuff in it.
///
string TTROOTClass::cpp_core_typename() const
{
	return _class_name;
}

///
/// Switch from CPP world to the .NET world (i.e. for a return type from
/// some sort of call.
///
/// WARNING: If this object doesn't inherit from TObject yet, then 
/// this won't correctly identify the same object that comes through twice!!
/// this means that it is possible to have two wrapper objects pointing to the
/// same real object, and when both try to do the delete... BOOM!
///
/// This conversion is a bit tricky because we need to
/// translate from the TObject type world into a good representative object
/// in our work. For example, TFile::Get("hi") might return a TH1F -- but it
/// will look a lot like a TObject from the signature. For this to be useful
/// we need to create a TH1F object, and return the interface to TObject from
/// Get. Then the user can re-cast the object as they expect.
//
// TODO: check to see if there is a subtle error here. The type we use get best object on
// might not be the same though the objects are the same depending on use_interface. For now
// the use case makes it ok, but...
///
void TTROOTClass::translate_to_net (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter, bool use_interface, bool is_static) const
{
	RootClassInfo info (RootClassInfoCollection::GetRootClassInfo(_class_name));
	string tname ("ROOTNET::Interface::" + info.NETName());
	if (!use_interface)
		tname = "ROOTNET::" + info.NETName();

	emitter.start_line() << tname << " ^"
		<< net_name;

	if (_inherits_from_TObject) {
		emitter() << " = ROOTNET::Utility::ROOTObjectServices::GetBestObject<" << tname <<"^>"
			<< "(";

		if (_modifiers == "&") {
			emitter() << "&" << cpp_name << ");" << endl;
		} else if (_modifiers == "") {
			if (is_static) {
				emitter() << "&" << cpp_name << ");" << endl;
			}
			else {
				emitter() << "new ::" << _class_name << "(";
				emitter() << cpp_name << "));" << endl;
			}
		} else {
			emitter() << cpp_name << ");" << endl;
		}
	} else {
		emitter() << " = gcnew ROOTNET::" << info.NETName() << " (";
		if (_modifiers == "&") {
		  if (_is_const) {
		    emitter() << "const_cast<::" << _class_name << "*>(";
		  } else {
		    emitter() << "(";
		  }
		  emitter() << "&" << cpp_name << "));" << endl;
		} else if (_modifiers == "") {
		  emitter() << "new ::" << _class_name << "(";
		  emitter() << cpp_name << "));" << endl;
		} else {
		  if (_is_const) {
		    emitter() << "const_cast<::" << _class_name << _modifiers << ">(" << cpp_name << "));" << endl;
		  } else {
		    emitter() << cpp_name << ");" << endl;
		  }
		}		
	}
}

///
/// Make sure that forward referencing knows about this object!
///
vector<string> TTROOTClass::referenced_root_types(void) const
{
	vector<string> result;
	result.push_back(_class_name);
	return result;
}

