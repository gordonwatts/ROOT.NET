///
/// Implement hte feature manager. We hold onto all the features,
/// and when asked return features that are needed, and coordinate
/// composing several of the features together.
///
#include "FeatureManager.hpp"

#include <vector>
#include <string>
#include <algorithm>
#include <iterator>

using std::vector;
using std::string;
using std::for_each;
using std::copy_if;
using std::copy;
using std::back_inserter;

FeatureManager *FeatureManager::_instance = 0;

FeatureManager::FeatureManager(void)
{
}


FeatureManager::~FeatureManager(void)
{
}

///
/// Return an instance, or new it if now already existing
///
FeatureManager *FeatureManager::instance()
{
	if (_instance == 0)
		_instance = new FeatureManager();
	return _instance;
}

///
/// Add a feature to the list
///
void FeatureManager::AddFeature(FeatureBase *b)
{
	instance()->_all_features.push_back(b);
}

///
/// Hold onto a colleciton of features and
/// make it easy for the rest of the code to use it.
///
FeatureManager::FeatureCollection::FeatureCollection (vector<FeatureBase*> features)
	: _features(features)
{
}
///
/// Return all the interfaces that these features might want.
///
vector<string> FeatureManager::FeatureCollection::get_additional_interfaces(const RootClassInfo &info)
{
	vector<string> result;
	for_each (_features.begin(), _features.end(),
		[&result, &info] (FeatureBase *b)
	{
		vector<string> t (b->get_additional_interfaces(info));
		copy (t.begin(), t.end(), back_inserter(result));
	});
	return result;
}

///
/// Emit code for the header
///
void FeatureManager::FeatureCollection::emit_header_method_definitions (const RootClassInfo &info, SourceEmitter &emitter)
{
	for_each(_features.begin(), _features.end(),
		[&emitter, &info] (FeatureBase *b) { b->emit_header_method_definitions(info, emitter); });
}

///
/// Emit any extra class methods.
///
void FeatureManager::FeatureCollection::emit_class_methods (const RootClassInfo &info, SourceEmitter &emitter)
{
	for_each(_features.begin(), _features.end(),
		[&emitter, &info] (FeatureBase *b) { b->emit_class_methods(info, emitter); });
}

///
/// Get all the various extra referenced classes...
///
std::vector<std::string> FeatureManager::FeatureCollection::get_additional_root_class_references (const RootClassInfo &info)
{
	vector<string> result;
	for_each(_features.begin(), _features.end(),
		[&result, &info] (FeatureBase *b) {
			auto t = b->get_additional_root_class_references(info);
			copy (t.begin(), t.end(), back_inserter(result));
	});
	return result;
}

///
/// Return all the features that this class might have to deal with
///
FeatureManager::FeatureCollection FeatureManager::GetFeaturesFor(const RootClassInfo &info)
{
	vector<FeatureBase*> applicable;
	auto &allf = instance()->_all_features;
	copy_if(allf.begin(), allf.end(), back_inserter(applicable),
		[&info] (FeatureBase *b) { return b->is_applicable(info); });

	return FeatureCollection(applicable);
}
