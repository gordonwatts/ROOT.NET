#include "RootClassMethod.hpp"
#include "RootClassMethodArg.hpp"
#include "RootClassInfo.hpp"
#include "RootClassInfoCollection.hpp"
#include "CPPNetTypeMapper.hpp"
#include "ROOTHelpers.h"
#include "ConverterErrorLog.hpp"
#include "WrapperConfigurationInfo.hpp"

#include "TMethod.h"
#include "TMethodArg.h"
#include "TClass.h"
#include "TList.h"

#include <sstream>
#include <cassert>
#include <locale>
#include <algorithm>

using std::locale;
using std::isalpha;
using std::string;
using std::ostringstream;
using std::vector;
using std::set;
using std::find_if;

RootClassMethod::RootClassMethod(void)
: _root_method_info (0), _args_good (false), _is_ambiguous(false), _is_hidden(false), _skip_method (false), _covar_method(false),
  _parent(0), _superclass_ptr_good(false), _is_operator(false), _is_math_operator(false), _is_dtor (false)
{
}

RootClassMethod::RootClassMethod(TMethod *method, const RootClassInfo *parent_class, int max_args)
: _root_method_info (method), _parent (parent_class), _args_good(false), _is_ambiguous(false), _is_hidden(false), _is_user_conversion_good(false), _skip_method(false), _covar_method(false),
_superclass_ptr_good(false)
{
	if (max_args < 0) {
		_number_args = method->GetNargs();
	} else {
		_number_args = max_args;
	}
	ParseMethodInfo();
}

///
/// Return a type translator for the return type
///
const CPPNetTypeMapper::TypeTranslator *RootClassMethod::get_return_type_translator(void) const
{
	if (has_return_value()) {
		return CPPNetTypeMapper::instance()->get_translator_from_cpp(return_type());
	} else {
		return 0;
	}
}

///
/// Generate a header that is normalized -- dosen't have argument names...
///
string RootClassMethod::generate_normalized_method_header (void) const
{
	return generate_method_header (false, false);
}

///
/// Just get the stuff between the "(" and the ")".
///
string RootClassMethod::generate_normalized_method_arguments(bool use_argument_names) const
{
	const vector<RootClassMethodArg> &args = arguments();

	ostringstream result;
	for (unsigned int i = 0; i < args.size(); i++) {
		if (i > 0) {
			result << ", ";
		}
		result << args[i].NETInterfaceTypeName();
		if (use_argument_names) {
			result << " " << args[i].get_argname();
		}
	}

	return result.str();
}

///
/// Generate a header for this function.
///
string RootClassMethod::generate_method_header(bool add_object_qualifier, bool use_argument_names) const
{
	ostringstream result;

	///
	/// There is always a return type unless this is a ctor!
	///

	if (!IsCtor() && !IsDtor() && !IsUserConversion()) {
		if (has_return_value()) {
			const CPPNetTypeMapper::TypeTranslator *return_translator = get_return_type_translator();
			result << return_translator->net_return_type_name() << " ";
		} else {
			result << "void ";
		}
	}

	///
	/// The method name. If this is a conversion operator, then it has a return type and it has to become part of the
	/// operator name (yeah, weird).
	///

	if (add_object_qualifier) {
		result << _parent->NETName() << "::";
	}

	if (IsUserConversion()) {
		string method (NETName());
		result << "operator ";
		const CPPNetTypeMapper::TypeTranslator *return_translator = get_return_type_translator();
		result << return_translator->net_return_type_name();
	} else if (IsCtor()) {
		result << _parent->NETName_ClassOnly();
	} else {
		/// Simply just the name...
		result << NETName();
	}

	///
	/// Now do the arguments
	///

	result << " (" << generate_normalized_method_arguments(use_argument_names) << ")";

	return result.str();
}

///
/// Return true if this method is an override of a parent method.
///
bool RootClassMethod::IsDefaultOverride() const
{
	//
	// If this is an over-ride, then we need to emit the "override" keyword.
	// We do a bunch of caching here b/c it cna be a little expensive doing all the lookups.
	//

	if (!_superclass_ptr_good) {
		_superclass_ptr_good = true;
		_superclass_ptr = nullptr;

		if (!IsStatic()) {
			auto superClass (_parent->GetBestClassToInherrit());
			if (superClass.size() > 0) {
				_superclass_ptr = RootClassInfoCollection::GetRootClassInfoPtr(superClass);
			}
		}
	}

	//
	// Now, check to see if the method exists in the super class. If so, see
	// if the exact method exists.
	//
	if (_superclass_ptr != nullptr) {
		auto methods (_superclass_ptr->methods_of_name(NETName()));
		const auto me = *this;
		auto found = find_if(methods.begin(), methods.end(), [&] (const RootClassMethod &method) {
			return method.is_equal(me);
		});
		return found != methods.end();
	}

	return false;
}


/// Check to see if we can translate this method header. This amounts to making usre that
/// all the types are "good".
bool RootClassMethod::can_be_translated (void) const
{
  vector<string> all_types (get_all_referenced_types());

  for (int i = 0; i < all_types.size(); i++) {
	if (!CPPNetTypeMapper::instance()->has_mapping(all_types[i])) {
	  ConverterErrorLog::log_type_error(all_types[i], "No type converter");
	  return false;
	}
  }

  ///
  /// Finally, we have some special cases...
  ///

  if (IsOperator()) {
	if (CPPName() == "operator const char*") {
	  return false;
	}
	if (CPPName().find("operator new") == 0) {
	  return false;
	}
	if (CPPName().find("operator delete") == 0) {
		return false;
	}

	if (CPPName() == "operator=" && return_type() == "void") {
	  return false;
	}
  }

  ///
  /// If we got this far, we are cool!
  ///

  return true;
}

/// Simple test to see if we are a ctor!
bool RootClassMethod::IsCtor() const
{
	string name = _root_method_info->GetName();
	if (name == _parent->CPPName_ClassOnly()) {
		return true;
	}
	return false;
}

/// Is the method a const method in C++?
bool RootClassMethod::IsConst() const
{
  return _root_method_info->Property() & kIsMethConst;
}

/// See if the method name and the return name are the same...
bool RootClassMethod::IsOtherObjectCtor(void) const
{
	string name = _root_method_info->GetName();
	return name == return_type() && (name != _parent->CPPName_ClassOnly());
}

/// Simple test to see if we are a dtor
bool RootClassMethod::IsDtor(void) const
{
	return _is_dtor;
}

/// Simple test to see if this method is an operator
bool RootClassMethod::IsOperator(void) const
{
	return _is_operator;
}

void RootClassMethod::ParseMethodInfo()
{
	// Operator?

	string name = _root_method_info->GetName();
	_is_operator = name.find("operator") == 0;

	// dtor?
	_is_dtor = name[0] == '~';

	//
	// We will need to examine the # of arguments, so...
	//

	arguments();

	// Math operator
	// Binary or unary only. If something else is going on, then get
	// out of here!
	_is_math_operator = false;
	if (_args.size() <= 2 && name.find("operator") != name.npos)
	{
		string op (name.substr(8));

		// Unary only operators: !, ~, ++, --
		if (_args.size() <= 1
			&& (op == "!" || op == "~" || op == "++" || op == "--"))
		{
			_is_math_operator = true;
		} else {

			// The operator- can have no arguments - so it is just applied to itself. It can
			// also have 1 or 2 arguments...
			if (op == "-"
				|| op == "+") {
					_is_math_operator = true;
			} else {
				// If binary operator, then we are a go! +, -, *, /, %, &, |, ^, <<, >>
				// (+, - taken care of above).
				if (_args.size() < 1) {
					_is_math_operator = false;
				} else {
					if (
						op == "/"
						|| op == "*"
						|| op == "%"
						|| op == "&"
						|| op == "|"
						|| op == "^"
						|| op == "<<"
						|| op == ">>"
						)
						_is_math_operator = true;
				}
			}
		}
	}
}

/// Is this a math like operator?
bool RootClassMethod::IsMathOperator(void) const
{
	return _is_math_operator;
}

/// Simple test to see if this method is an array lookup operator
bool RootClassMethod::IsIndexer() const
{
  string name = _root_method_info->GetName();
  return name.find("operator[]") == 0;
}

/// If this is a user defined converastion operator -- return true. The prototype
/// we endup generating is a bit different in this case (you can't specify the return
/// type). Perhaps other uses as well.
bool RootClassMethod::IsUserConversion(void) const
{
	///
	/// Can we do the cache?
	///

	if (_is_user_conversion_good) {
		return _is_user_conversion;
	}
	_is_user_conversion_good = true;

	///
	/// Nope. Have to do the work!
	///

	_is_user_conversion = false;
	string name = _root_method_info->GetName();
	if (!IsOperator()) {
		return false;
	}
	if (name == "operator delete" || name == "operator delete[]") {
		return false;
	}

	/// All operators start with a funny character, so look for that

	int index = 8;
	while (name[index] == ' ') {
		index++;
	}

	_is_user_conversion = isalpha(name[index]) != 0;
	return _is_user_conversion;
}

///
/// Return the name of the method. This is the name that we can use for output in the .NET world. Sometimes the ROOT folks use
/// a reserved word...
///
const string &RootClassMethod::NETName() const
{
	if (_netname.size() == 0) {
		_netname = CPPName();
		if (_netname == "array") {
			_netname = "array_rootnet";
		} else if (_netname == "Finalize") {
			_netname = "Finalize_rootnet";
		}
	}
	return _netname;
}
string RootClassMethod::CPPName() const
{
	return _root_method_info->GetName();
}

///
/// Return the class of where the method definiton occurs. If the method is
/// inherited then this will be one step up in the hierarchy.
///
string RootClassMethod::ClassOfMethodDefinition() const
{
	return _root_method_info->GetClass()->GetName();
}

///
/// Return the full name -- qualified by a class if it was inherited.
/// If it is a user converstion operator, then add in the global namespace qualifier, but
/// only if it refers to a class and not to a random item like "long".
///
string RootClassMethod::FullName() const
{
	string method_class (ClassOfMethodDefinition());
	if (method_class != _parent->CPPName() && !IsVirtual()) {
		return method_class + "::" + CPPName();
	}
	if (IsUserConversion()) {
		// Make sure it is a root type.
		if (is_root_class(return_type())) {
			return "operator ::" + CPPName().substr(9);
		} else {
			return CPPName();
		}
	}
	return CPPName();
}

///
/// Return a list of the arugments to this method
///
const vector<RootClassMethodArg> &RootClassMethod::arguments() const
{
	if (_args_good) {
		return _args;
	}

	_args_good = true;

	/// Reset the arg counter so we label unamed arguments in a sensible way
	RootClassMethodArg::reset_arg_counter();

	/// Loop over all args and create a "nice" interface for them.
	TList *l = _root_method_info->GetListOfMethodArgs();
	assert (l->GetSize() >= _number_args);
	for (int i_arg = 0; i_arg < _number_args; i_arg++) {
		TMethodArg *marg = static_cast<TMethodArg*> (l->At(i_arg));
		_args.push_back (RootClassMethodArg (marg));
	}

	/// Finally, we have to deal with some special cases - places where CINT lies to us,
	/// and the type it reports is not what the compiler sees back! Very UGH!
	///

	WrapperConfigurationInfo::FixUpMethodArguments(_parent, _root_method_info->GetName(), _args);

	return _args;
}

///
/// If the return type is other than "void"...
///
bool RootClassMethod::has_return_value (void) const
{
  if (IsCtor()
	|| IsDtor()
	|| return_type() == "void") {
	return false;
  }
  return true;
}

///
/// The return type. Can be "void", so watch it!
///
string RootClassMethod::return_type (void) const
{
	if (_return_type.size() != 0) {
		return _return_type;
	}
	_return_type = WrapperConfigurationInfo::FixupMethodReturnType(_parent, this, _root_method_info->GetReturnTypeName());
	return _return_type;
}

///
/// Reset the return type locally
///
void RootClassMethod::SetReturnType (const string &rtn_type)
{
	_return_type = rtn_type;
}

///
/// The return type. Can be "void", so watch it! And remove any "&", "const", etc.
///
string RootClassMethod::raw_return_type (void) const
{
	string result(return_type());
	if (result.find('&') != result.npos) {
		result.erase(result.find('&'));
	}
	if (result.find('*') != result.npos) {
		result.erase(result.find('*'));
	}
	if (result.find("const ") == 0) {
		result = result.substr(6);
	}
	return result;
}

///
/// Return true if this method is a virtual method
///
bool RootClassMethod::IsVirtual (void) const
{
	return (_root_method_info->Property() & kIsVirtual) != 0;
}

///
/// Return true if this method is a static method
///
bool RootClassMethod::IsStatic (void) const
{
	return (_root_method_info->Property() & kIsStatic) != 0;
}

///
/// Return the list of raw types all at once!
///
vector<string> RootClassMethod::get_all_referenced_raw_types() const
{
	vector<string> result;
	if (has_return_value()) {
	  result.push_back(raw_return_type());
	}

	const vector<RootClassMethodArg> &args(arguments());
	for (unsigned int j = 0; j < args.size(); j++) {
		const RootClassMethodArg &arg (args[j]);
		result.push_back(arg.RawCPPTypeName());
	}
	return result;
}

///
/// Return a list of all types at once!
///
vector<string> RootClassMethod::get_all_referenced_types() const
{
  vector<string> result;
  if (has_return_value()) {
	result.push_back(return_type());
  }

  const vector<RootClassMethodArg> &args(arguments());
  for (unsigned int j = 0; j < args.size(); j++) {
	  const RootClassMethodArg &arg (args[j]);
	  result.push_back(arg.CPPTypeName());
  }
  return result;
}

///
/// Same, but make sure they are "ROOT" types
///
vector<string> RootClassMethod::get_all_referenced_raw_root_types() const
{
	vector<string> classes (get_all_referenced_types());
	set<string> result;
	for (unsigned int i = 0; i < classes.size(); i++) {
		if (CPPNetTypeMapper::instance()->has_mapping(classes[i])) {
			auto t = CPPNetTypeMapper::instance()->get_translator_from_cpp(classes[i]);
			auto cls = t->referenced_root_types();
			result.insert(cls.begin(), cls.end());
		}
	}
	return vector<string> (result.begin(), result.end());
}

///
/// For enums now
///
vector<string> RootClassMethod::get_all_referenced_enums() const
{
	vector<string> classes (get_all_referenced_raw_types());
	vector<string> result;
	for (unsigned int i = 0; i < classes.size(); i++) {
		if (ROOTHelpers::IsEnum(classes[i])) {
			result.push_back(classes[i]);
		}
	}
	return result;
}

///
/// Returns true if this class name is probably a root class name. Pretty
/// darn basic for now!
///
bool RootClassMethod::is_root_class(const std::string &class_name) const
{
	return ROOTHelpers::IsClass(class_name);
}

/// Certian types of methods can't be emitted as an interface. This is a nice place
/// that collects all of that information in one place.
bool RootClassMethod::IsGoodForInterface (void) const
{
	/// If we aren't going to emit code, then we had better drop it from here too!
	if (!IsGoodForClass()) {
		return false;
	}

	/// You can't create a object ctor in an interface. You have to do it via the raw object!
	if (IsCtor()) {
		return false;
	}

	/// Operators are not allowed as interface methods...
	if (IsOperator()) {
		return false;
	}

	return true;
}

/// Check to see if the method is ok to be emitted as a .NET method.
bool RootClassMethod::IsGoodForClass (void) const
{
	/// dtor's aren't wrapped. They will automatically get called when the garbage collector
	/// cleans up the wrapper object (as part of the finalize pattern).
	if (IsDtor()) {
		return false;
	}

	/// Weird-o ctors should just be killed.
	if (IsOtherObjectCtor()) {
		return false;
	}

	/// Skip it!
	if (IsToBeSkipped()) {
		return false;
	}

	return true;
}

/// Returns true if the two methods are 100% equal. That is -- 
/// the same in the way you might want to override this method
/// with the other.
bool RootClassMethod::is_equal(const RootClassMethod &other, bool consider_return_type) const
{
  /// Are the names the same?
  if (other.CPPName() != CPPName()) {
	return false;
  }

  /// Are the number of arguments the same?
  if (other.arguments().size() != arguments().size()) {
	return false;
  }

  /// Make sure the argument types are the same!
  for (int i = 0; i < arguments().size(); i++) {
	if (arguments()[i].NETTypeName() != other.arguments()[i].NETTypeName()) {
	  return false;
	}
  }

  if (consider_return_type && other.return_type() != return_type()) {
	return false;
  }

  /// If we got this far, then we are good!
  return true;
}

/// Return true if this < other.
bool RootClassMethod::is_less_than(const RootClassMethod &other, bool consider_return_type) const
{
  /// By name?
  if (NETName() < other.NETName()) {
	return true;
  }
  if (NETName() > other.NETName()) {
	return false;
  }

  /// By number of args?
  if (arguments().size() < other.arguments().size()) {
	return true;
  }
  if (arguments().size() > other.arguments().size()) {
	return false;
  }

  try {
	/// By the type of the args?
	for (int i = 0; i < arguments().size(); i++) {
	  const RootClassMethodArg &arg (arguments()[i]);
	  const RootClassMethodArg &oarg (other.arguments()[i]);

	  string argme = arg.CPPTypeName();
	  string argother = oarg.CPPTypeName();

	  if (arg.can_be_translated()) {
		argme = arg.NETTypeName();
	  }
	  if (oarg.can_be_translated()) {
		argother = oarg.NETTypeName();
	  }

	  if (argme < argother) {
		return true;
	  }
	  if (argme > argother) {
		return false;
	  }
	}
  } catch (std::exception &)
  {
	/// Failed to convert some of those types. What a mess!
	/// Pretend they are equal. No one should be translating this anyway...
	return false;
  }

  if (consider_return_type) {
	string rtn (return_type());
	string ortn (other.return_type());

	if (rtn < ortn) {
	  return true;
	}
	if (rtn > ortn) {
	  return false;
	}
  }

  /// If we drop through -- then it must == or >. This should be enough to
  /// establish weak ordering -- so we return false.

  return false;
}
