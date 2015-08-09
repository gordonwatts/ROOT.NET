#pragma once
///
/// Add Enumerable code to any TIter... The TIter class itself will get the specific code that does the work. Anything
/// below will just get the implementation.
///

#include "FeatureBase.hpp"

class FeatureTIterEnumerable : public FeatureBase
{
public:
	FeatureTIterEnumerable(void);
	~FeatureTIterEnumerable(void);

	bool is_applicable (const RootClassInfo &info);
	std::vector<std::string> get_additional_interfaces (const RootClassInfo &info);

	void emit_header_method_definitions (const RootClassInfo &info, SourceEmitter &emitter);

	void emit_class_methods (const RootClassInfo &info, SourceEmitter &emitter);
};

