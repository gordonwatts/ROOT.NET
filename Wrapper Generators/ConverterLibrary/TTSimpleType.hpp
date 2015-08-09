#pragma once

#include "CPPNetTypeMapper.hpp"

///
/// Translate a type that has a 1:1 mappign with .NET and needs no real effort to do the translation.
class TTSimpleType : public CPPNetTypeMapper::TypeTranslator
{
public:
	inline TTSimpleType(void)
	{
	}

	inline TTSimpleType(const std::string &dotnet_type, const std::string &cpp_type)
		: TypeTranslator (dotnet_type, cpp_type)
	{
	}

	inline bool requires_translation (void) const {
		return false;
	}
};
