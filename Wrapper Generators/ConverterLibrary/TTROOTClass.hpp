#pragma once

#include "CPPNetTypeMapper.hpp"

///
/// A ROOT class translator -- so we can pass in and out other root classes!
/// :-)
///
class TTROOTClass : public CPPNetTypeMapper::TypeTranslator
{
public:
	TTROOTClass(const std::string &root_class, const bool is_from_tobject, const std::string modifier = "", bool _is_const = false);
	~TTROOTClass(void);

	inline bool requires_translation (void) const {
		return true;
	}

	/// We have to adorn all C++ typenames with the global namespace indicator.
	std::string cpp_code_typename (void) const;

	/// Strip off any of the extra stuff
	std::string cpp_core_typename (void) const;

	/// The .NET interface name
	inline std::string net_interface_name (void) const {
		return "ROOTNET::Interface::" + net_typename();
	}

	void translate_to_cpp (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const;
	void translate_to_net (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter, bool use_interface, bool is_static) const;

	/// Return the ROOT type that we contain...
	std::vector<std::string> referenced_root_types (void) const;

private:
	const std::string _class_name;

	/// Can be a space, a * or a &...
	const std::string _modifiers;

	/// True if the thing is const
	const bool _is_const;

	/// True if this class comes from TObject
	const bool _inherits_from_TObject;
};
