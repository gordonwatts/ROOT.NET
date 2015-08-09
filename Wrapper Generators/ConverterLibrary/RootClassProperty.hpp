#pragma once
///
/// RootClassProperty.hpp
///
///  Contains basic info about properties that we can give to this class.
///

#include "CPPNetTypeMapper.hpp"
#include "RootClassMethod.hpp"
#include <string>

class RootClassMethod;

class RootClassProperty
{
public:
	inline RootClassProperty(void)
	{
	}

	inline RootClassProperty (const std::string &prop_name, const std::string &prop_type)
		: _property_type(prop_type), _setter(false), _getter(false), _tt(0)
	{
		ChangePropertyName(prop_name);
	}

	inline ~RootClassProperty(void)
	{
	}

	inline void SetAsGetter (const RootClassMethod m)
	{
		_getter = true;
		_getter_m = m;
	}
	inline void SetAsSetter (const RootClassMethod m)
	{
		_setter = true;
		_setter_m = m;
	}
	inline void ResetSetter ()
	{
		_setter = false;
	}

	inline bool isSetter (void) const
	{
		return _setter;
	}
	inline bool isGetter (void) const
	{
		return _getter;
	}

	/// If we were returning this guy from a "get" operation, this is the type we'd use.
	const std::string as_return_type(void) const;
	/// If we were passing this as an argumet (i.e. the set) then this is what we would use
	const std::string as_argument_type(void) const;
	/// If we wanted to use this as a property type, then this is what we would use
	const std::string property_type (void) const;
	/// The raw CPP type
	inline const std::string &raw_cpp_type (void) const {
		return _property_type;
	}

	const std::string &name (void) const
	{
		return _property_name;
	}
	void ChangePropertyName (const std::string &newpropname)
	{
		_property_name = newpropname;
	}

	std::string getter_name (void) const;
	std::string setter_name (void) const;

	const RootClassMethod *getter_method(void) const
	{
		if (_getter) {
			return &_getter_m;
		} else {
			return 0;
		}
	}
	const RootClassMethod *setter_method(void) const
	{
		if (_setter) {
			return &_setter_m;
		} else {
			return 0;
		}
	}

	const bool has_type_confict (void) const;

	const CPPNetTypeMapper::TypeTranslator *type_translator(void) const;

	// Return if static. We are static only if everyone is static!!
	bool isStatic (void) const
	{
		if (_setter & _getter)
			return _setter_m.IsStatic() && _getter_m.IsStatic();

		if (_getter)
			return _getter_m.IsStatic();

		return _setter_m.IsStatic();
	}

private:
	std::string _property_name;
	std::string _property_type;

	mutable const CPPNetTypeMapper::TypeTranslator *_tt;

	bool _setter, _getter;

	RootClassMethod _getter_m, _setter_m;
};

