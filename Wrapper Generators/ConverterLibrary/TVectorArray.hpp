#pragma once
///
/// Implement a type transformation (to an array<>) from a vector of some simple type (like int, double, etc.).
///

#include "CPPNetTypeMapper.hpp"

class TVectorArray : public CPPNetTypeMapper::TypeTranslator
{
public:
	inline TVectorArray(void)
	{
	}

	TVectorArray(const std::string &simpleTypeName);
	~TVectorArray(void);

	/// Make sure we do the translation correctly
	bool requires_translation(void) const {return true; }

	/// Do the mapping between the two
	virtual void translate_to_cpp (const std::string &name_net, const std::string &name_cpp, SourceEmitter &emitter) const;
	virtual void translate_to_net(const std::string &name_net, const std::string &name_cpp, SourceEmitter &emitter, bool use_interface, bool is_static) const;

private:
	const std::string _simple_name;
};

