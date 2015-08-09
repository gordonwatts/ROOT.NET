///
/// Implementation for a field that we are keeping track of
///

#include "RootClassField.hpp"
#include "CPPNetTypeMapper.hpp"
#include "ConverterErrorLog.hpp"
#include "ROOTHelpers.h"

#include <TDataMember.h>
#include <TClass.h>

#include <string>
#include <vector>
#include <stdexcept>

using std::string;
using std::vector;
using std::exception;

///
/// Null initalizer. Normally this shouldn't happen. :-)
///
RootClassField::RootClassField(void)
	: _root_field (0), _trans(0)
{
}

///
/// Track a field that we can (we hope) access.
///
RootClassField::RootClassField(TDataMember *f)
	: _root_field(f), _trans(0)
{
	if (!(_root_field->Property() & kIsArray)) {
		if (CPPNetTypeMapper::instance()->has_mapping(_root_field->GetTrueTypeName())) {
			_trans = CPPNetTypeMapper::instance()->get_translator_from_cpp(_root_field->GetTrueTypeName());
		}
	}
}

RootClassField::~RootClassField(void)
{
}

string RootClassField::NETName() const
{
	return _root_field->GetName();
}

///
/// The .NET name of this field is actually a little funny - the normal ROOT.NET infrastructure
/// allows for different types when going into .NET and coming out. Since these are property setters, however,
/// we can't do it that way - we have to have the same type going in and out.
///
string RootClassField::NETType() const
{
	return _trans->net_return_type_name();
}

///
/// Can we do a "getter" for this guy? Standard is yes - we can always
///
bool RootClassField::GetterOK() const
{
	return _trans != 0;
}

///
/// Can we do a setter? That is fine as long as the .NET type we pass in and return are the same. Otherwise we have to bail
/// on the setter.
///
bool RootClassField::SetterOK() const
{
	return (_trans != 0) && _trans->net_return_type_name() == _trans->net_typename();
}

string RootClassField::CPPType() const
{
	return _root_field->GetTrueTypeName();
}

///
/// Returns the name of the class that this guy is defined in.
///
string RootClassField::ClassOfFieldDefinition(void) const
{
	return _root_field->GetClass()->GetName();
}

///
/// Record who our parent class is
///
void RootClassField::ResetParent (const RootClassInfo *parent)
{
}

///
/// is_less_than
///
///  Compare left and right and decide who is greater. For now we are using
/// a straight name based comparison.
///
bool RootClassField::is_less_than(const RootClassField &right) const
{
	return string(_root_field->GetName()) < string(right._root_field->GetName());
}

///
/// CPPName - the name that is valid in C++
///
string RootClassField::CPPName() const
{
	return _root_field->GetName();
}

///
/// Get all the referenced class types. This is actually pretty simple here - as
/// there is only the field type (no worries about arguments, etc.). The C++ names
/// are returned.
///
vector<string> RootClassField::get_all_referenced_raw_types(void) const
{
	vector<string> result;
	result.push_back (_root_field->GetTrueTypeName());
	return result;
}

///
/// Return the referenced root types - if this is a template argument we
/// assume only the first one is interesting... and prey! :-) This will
/// have to be re-visited later on in case we use more than one argument.
///
vector<string> RootClassField::get_all_referenced_root_types(void) const
{
	auto allRootTypes = _trans->referenced_root_types();
	return allRootTypes;
}

///
/// Can we translate this guy?
///
bool RootClassField::can_be_translated() const
{
	if (_trans == 0) {
		return false;
	}
	vector<string> all_types (get_all_referenced_raw_types());

	for (int i = 0; i < all_types.size(); i++) {
		if (!CPPNetTypeMapper::instance()->has_mapping(all_types[i])) {
			ConverterErrorLog::log_type_error(all_types[i], "No type converter");
			return false;
		}
	}

	return true;
}
