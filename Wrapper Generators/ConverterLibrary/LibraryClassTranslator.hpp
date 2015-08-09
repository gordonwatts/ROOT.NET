#pragma once
///
/// Special version of the class translator that writes out
/// the header files only when needed.

#include "ClassTranslator.hpp"
#include "RootClassInfo.hpp"
#include "SourceEmitter.hpp"

class LibraryClassTranslator: public ClassTranslator
{
public:
	inline LibraryClassTranslator (const std::string &dirname)
		: ClassTranslator(dirname)
	{}

protected:
	/// Emit the .hpp file only if the two libraries are the same
	virtual bool emit_this_header (const RootClassInfo &class_being_wrapped, const RootClassInfo &dependent_class)
	{
		return class_being_wrapped.LibraryName() == dependent_class.LibraryName();
	}
	virtual bool emit_this_enum (const RootClassInfo &class_being_wrapped, const RootEnum &dependent_enum)
	{
		return class_being_wrapped.LibraryName() == dependent_enum.LibraryName();
	}

	/// Dump out the headers if we are orking on the core library (only)
	void emit_hpp_headers (SourceEmitter &emitter)
	{
		ClassTranslator::emit_hpp_headers(emitter);
		if (OutputDir().find("libCore") != OutputDir().npos) {
		emitter.include_file("root_type_helpers.hpp");
		}
	}
};

