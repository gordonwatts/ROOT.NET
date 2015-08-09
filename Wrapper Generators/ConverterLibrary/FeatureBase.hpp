#pragma once
///
/// A feature is something we add to a class as it is being generated. For example, an enumerator
/// can be added to TIter or TList, etc., to make things more ".NET" like.
///

#include <vector>
#include <string>

class RootClassInfo;
class SourceEmitter;

class FeatureBase
{
public:
	FeatureBase(void);
	virtual ~FeatureBase(void);

	/// Return true if this feature applies to the given class.
	virtual bool is_applicable (const RootClassInfo &info) { return false; }

	/// Get extra interfaces that the interface (and object) should inherrit from
	virtual std::vector<std::string> get_additional_interfaces (const RootClassInfo &info) { return std::vector<std::string> (); }

	/// Add anything to the class declaration
	virtual void emit_header_method_definitions (const RootClassInfo &info, SourceEmitter &emitter) {}

	/// Add methods to the C++ source file
	virtual void emit_class_methods (const RootClassInfo &info, SourceEmitter &emitter) {}

	/// List of additonal ROOT classes that will be referenced and should have #include done
	virtual std::vector<std::string> get_additional_root_class_references (const RootClassInfo &info) { return std::vector<std::string> (); }

};

