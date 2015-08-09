#pragma once
///
/// Translator to translate ROOT enum's in code.
///

#include "CPPNetTypeMapper.hpp"

#include <string>

class TTROOTenum : public CPPNetTypeMapper::TypeTranslator
{
public:
	TTROOTenum(const std::string &enum_name);
	~TTROOTenum(void);

	/// We definately require some casting. Bummer.
	virtual bool requires_translation (void) const {
		return true;
	}

	/// We have to adorn all C++ typenames with the global namespace indicator.
	std::string cpp_code_typename (void) const;

	/// But we don't require any cleanup code!
	virtual bool requires_cleanup_code (void) const {
		return false;
	}

	/// Make sure we know where we are getting this from!
	std::string net_interface_name (void) const;

	virtual void translate_to_cpp (const std::string &name_net, const std::string &name_cpp, SourceEmitter &emitter) const;
	virtual void translate_to_net(const std::string &name_net, const std::string &name_cpp, SourceEmitter &emitter, bool use_interface, bool is_static) const;
};

