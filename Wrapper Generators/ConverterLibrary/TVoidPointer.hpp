#pragma once
///
/// Implement the void* pointer type. For us this assumes it is a TObject style-pointer. For now, this
/// means we can't deal with any other type of class, but all of ROOT.NET is based on that, so...
///

#include "CPPNetTypeMapper.hpp"

class TVoidPointer : public CPPNetTypeMapper::TypeTranslator
{
public:
	TVoidPointer(void);
	~TVoidPointer(void);

	/// The work we have to do is pretty minimal, but it is there!
	bool requires_translation(void) const { return true; }

	virtual void translate_to_cpp (const std::string &name_net, const std::string &name_cpp, SourceEmitter &emitter) const;
	virtual void translate_to_net(const std::string &name_net, const std::string &name_cpp, SourceEmitter &emitter, bool use_interface, bool is_static) const;

	std::vector<std::string> referenced_root_types (void) const;
};

