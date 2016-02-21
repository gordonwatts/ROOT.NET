#pragma once

#include "CPPNetTypeMapper.hpp"

///
/// Translate from a CPP string to a .NET string and back.
class TTCPPString : public CPPNetTypeMapper::TypeTranslator
{
public:
	// is_char - true of if it is a "char", and otherwise a std::string.
	// is_const - true if there is a "const" out in front.
	// is_reference - true if there is a reference
	TTCPPString(bool is_char, bool is_const, bool is_reference);

	~TTCPPString(void);

	inline bool requires_translation (void) const {
		return true;
	}

	void translate_to_cpp (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const;
	void translate_to_net (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter, bool use_interface, bool is_static) const;

private:
	const bool _is_char;
};
