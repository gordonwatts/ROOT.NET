///
/// Add a feature that will make a TTree iterable, and allow access to its leaves via
/// the dynamic interface.
///
#pragma once
#include "FeatureBase.hpp"

class FeatureTreeIterator :
	public FeatureBase
{
public:
	inline FeatureTreeIterator(void) {};
	inline ~FeatureTreeIterator(void) {};

	// We only work on the TTree class.
	bool is_applicable (const RootClassInfo &info);

	// Add the enumerator interface dependency
	std::vector<std::string> get_additional_interfaces (const RootClassInfo &info);

	// Add the get enumerator calls here.
	void emit_header_method_definitions (const RootClassInfo &info, SourceEmitter &emitter);

	// And the methods that do the work here.
	void emit_class_methods (const RootClassInfo &info, SourceEmitter &emitter);
};
