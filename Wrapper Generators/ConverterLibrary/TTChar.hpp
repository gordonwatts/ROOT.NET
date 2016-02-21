#pragma once
#include "CPPNetTypeMapper.hpp"

// Translate from System::Char and the C++ char type.
class TTChar :
	public CPPNetTypeMapper::TypeTranslator
{
public:
	inline TTChar(const std::string &char_type = "const char")
		: TypeTranslator("::System::Char", char_type), _char_type(char_type)
	{
	}

	~TTChar(void);

	inline bool requires_translation(void) const {
		return true;
	}

	void translate_to_cpp(const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const;
	void translate_to_net(const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter, bool use_interface, bool is_static) const;

private:
	const std::string _char_type;
};
