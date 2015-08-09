///
/// Add the IEnumerable<NTObject> pattern to a TCollection or any of its decendents
#pragma once

#include "FeatureBase.hpp"

class FeatureTCollectionEnumerable : public FeatureBase
{
public:
	FeatureTCollectionEnumerable(void);
	~FeatureTCollectionEnumerable(void);

	bool is_applicable (const RootClassInfo &info);
	std::vector<std::string> get_additional_interfaces (const RootClassInfo &info);

	void emit_header_method_definitions (const RootClassInfo &info, SourceEmitter &emitter);

	void emit_class_methods (const RootClassInfo &info, SourceEmitter &emitter);

	std::vector<std::string> get_additional_root_class_references (const RootClassInfo &info);
};

