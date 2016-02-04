#pragma once
///
/// Utilities to help with processing fields (instance variables).
///

#include "RootClassField.hpp"
#include "RootClassInfo.hpp"
#include "RootClassInfoCollection.hpp"
#include "WrapperConfigurationInfo.hpp"
#include "ConverterErrorLog.hpp"

#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

/// Less operator for ordering class methods in a container like a set.
/// This ordering does not ignores the return value of a method - so covar returns
/// will look different.
struct FieldOrdering : public std::less<RootClassField>
{
	bool operator() (const RootClassField &left, const RootClassField &right) const
	{
		return left.is_less_than(right);
	}
};
typedef std::set<const RootClassField, FieldOrdering> field_set;

///
/// Iterator operator that will convert a RootClassInfo pointer into
/// a set of fields
///
class convert_fields_to_set {
public:
	inline convert_fields_to_set(void) {}

	field_set operator() (const RootClassInfo *info) const {
		const std::vector<RootClassField> &protos(info->GetAllDataFields(false));
		return field_set(protos.begin(), protos.end());
	}
	field_set operator() (const std::string &class_name) const {
		RootClassInfo &cinfo (RootClassInfoCollection::GetRootClassInfo(class_name));
		return this->operator ()(&cinfo);
	}
};

///
/// 
///
class change_field_parent
{
public:
	inline change_field_parent (const RootClassInfo *info)
		: _info (info)
	{
	}
	inline field_set operator() (const field_set &item)
	{
		field_set result;
		std::transform (item.begin(), item.end(), std::inserter (result, result.begin()), change_field_parent(_info));
		return result;
	}
	inline RootClassField operator() (const RootClassField &field)
	{
		RootClassField mnew (field);
		mnew.ResetParent(_info);
		return mnew;
	}
private:
	const RootClassInfo *_info;
};

///
/// Collapse a set of methods
///
template<class C>
void flatten_field_set_list (const std::vector<field_set> &set_of_fields, C &fields)
{
	for(std::vector<field_set>::const_iterator itr = set_of_fields.begin(); itr != set_of_fields.end(); itr++) {
		copy(itr->begin(), itr->end(), std::inserter(fields, fields.end()));
	}
}

///
/// Given a list of fields, move through them and suck up everything that shouldn't be there
/// b/c we don't know how to translate it!
///
inline std::vector<RootClassField> make_field_list_clean (const RootClassInfo &thisclass, const std::vector<RootClassField> &fields)
{
	std::vector<RootClassField> result;
	for (int i = 0; i < fields.size(); i++) {
		const RootClassField &field (fields[i]);
		if (!field.can_be_translated()) {
			continue;
		}

		std::vector<std::string> bad_classes (WrapperConfigurationInfo::BadClassLibraryCrossReference(thisclass.LibraryName(), field.get_all_referenced_raw_types()));
		if (bad_classes.size() > 0) {
			for (int i = 0; i < bad_classes.size(); i++) {
				ConverterErrorLog::log_type_error(bad_classes[i], "Field " + thisclass.CPPQualifiedName() + "::" + field.CPPName() + " can't be translated because it would cause an incorrect reference from this library (" + thisclass.LibraryName() + ")");
			}
			continue;
		}
		result.push_back(field);
	}
	return result;
}

class FieldHandlingUtils
{
public:
	FieldHandlingUtils(void);
	~FieldHandlingUtils(void);
};

