///
/// Code to implement a small tracker of classes we are going to translate.
///

#include "ClassInterfaceRepositoryState.hpp"
#include "CPPNetTypeMapper.hpp"
#include "TTROOTClass.hpp"
#include "TTROOTenum.hpp"

#include "TClass.h"
#include "TROOT.h"

#include <stdexcept>

using std::string;
using std::runtime_error;

ClassInterfaceRepositoryState::ClassInterfaceRepositoryState(void)
{
}

///
/// Add a class to translate. Don't do it twice. Make sure the type system knows about the class.
///
void ClassInterfaceRepositoryState::request_class_translation(const std::string &class_name)
{
	TClass *cls = gROOT->GetClass(class_name.c_str());
	if (cls == 0) {
		return;
	}

	///
	/// Already there?
	///

	if (_classes_to_translate.find(class_name) != _classes_to_translate.end()
		|| _classes_translated.find(class_name) != _classes_translated.end())
	{
		return;
	}

	///
	/// Nope! Add it to the list.
	///

	_classes_to_translate.insert(class_name);

	///
	/// Finally, make sure we know how to translate the thing properly when it gets referenced in code.
	///

	register_class_translation(class_name);
}

///
/// Register a class for translation. We do this to make sure that
/// later on when the class gets referenced our type system knows about
/// it and will register it.
///
void ClassInterfaceRepositoryState::register_class_translation (const string &class_name)
{
	TClass *cls = gROOT->GetClass(class_name.c_str());
	if (cls == 0) {
		return;
	}

	bool is_from_tobj = cls->InheritsFrom("TObject");

	CPPNetTypeMapper::instance()->AddTypeMapper (new TTROOTClass(class_name, is_from_tobj));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTROOTClass(class_name, is_from_tobj, "", true));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTROOTClass(class_name, is_from_tobj, "&"));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTROOTClass(class_name, is_from_tobj, "&", true));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTROOTClass(class_name, is_from_tobj, "*"));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTROOTClass(class_name, is_from_tobj, "*", true));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTROOTClass(class_name, is_from_tobj, "*&"));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTROOTClass(class_name, is_from_tobj, "*&", true));
}

///
/// Request a particular enum to be translated
///
void ClassInterfaceRepositoryState::request_enum_translation(const std::string &enum_name)
{
	///
	/// Already there? Otherwise, we should add it!
	///

	if (_enums_to_translate.find(enum_name) != _enums_to_translate.end()
		|| _enums_translated.find(enum_name) != _enums_translated.end())
	{
		return;
	}
	_enums_to_translate.insert(enum_name);

	///
	/// Now, add a transltor for the enum.
	///

	register_enum_translation(enum_name);
}

///
/// Register an emum so the type system knows about it
///
void ClassInterfaceRepositoryState::register_enum_translation (const std::string &enum_name)
{
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTROOTenum (enum_name));
}

/// Are there any classes to translate?
bool ClassInterfaceRepositoryState::classes_to_translate() const
{
	return _classes_to_translate.size() > 0;
}
bool ClassInterfaceRepositoryState::enums_to_translate() const
{
	return _enums_to_translate.size() > 0;
}

/// Get a class to translate, and pop it off the list
string ClassInterfaceRepositoryState::next_class()
{
	if (_classes_to_translate.size() == 0) {
		throw runtime_error ("Ask to translate class and no classes left to translate in ClassInterfaceRepositoryState::next_class");
	}

	string result = *_classes_to_translate.begin();
	_classes_to_translate.erase(result);
	_classes_translated.insert(result);
	return result;
}
/// Get a class to translate, and pop it off the list
string ClassInterfaceRepositoryState::next_enum()
{
	if (_enums_to_translate.size() == 0) {
		throw runtime_error ("Ask to translate enum and no enums left to translate in ClassInterfaceRepositoryState::next_enum");
	}

	string result = *_enums_to_translate.begin();
	_enums_to_translate.erase(result);
	_enums_translated.insert(result);
	return result;
}
