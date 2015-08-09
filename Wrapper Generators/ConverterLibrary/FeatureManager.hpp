#pragma once
///
/// Manage the features collection.
///

#include "FeatureBase.hpp"

#include <vector>

class FeatureManager
{
public:
	class FeatureCollection : FeatureBase
	{
	public:
		FeatureCollection (std::vector<FeatureBase*> features);

		/// We already went through the selection. All good!
		bool is_applicable(const RootClassInfo &info)
		{
			return true;
		}

		/// Return all the interfaces that these features might want.
		std::vector<std::string> get_additional_interfaces(const RootClassInfo &info);
		/// Define the IEnumerable guys
		void emit_header_method_definitions (const RootClassInfo &info, SourceEmitter &emitter);
		/// Write out the Enumerable methods as well as the class definition to run the eumerable!
		void emit_class_methods (const RootClassInfo &info, SourceEmitter &emitter);
		/// Get a list of classes that are referenced
		std::vector<std::string> get_additional_root_class_references (const RootClassInfo &info);

	private:
		std::vector<FeatureBase*> _features;
	};

	/// Add a new feature
	static void AddFeature (FeatureBase *b);

	/// Return all the feature that might be activated for a particular feature.
	/// Often this will be a collection of features hidden behind a single wrapper.
	static FeatureCollection GetFeaturesFor (const RootClassInfo &info);
private:
	FeatureManager(void);
	~FeatureManager(void);

	static FeatureManager *instance();
	static FeatureManager *_instance;

	/// List of all features
	std::vector<FeatureBase*> _all_features;
};

