#pragma once
///
/// deal with a vector<TObject>'s.
///

#include "CPPNetTypeMapper.hpp"

#include <string>

class TTROOTClassVector : public CPPNetTypeMapper::TypeTranslator
{
public:
	TTROOTClassVector(const std::string &objectName);
	~TTROOTClassVector(void);

	/// We definately have to do the translation.
	virtual bool requires_translation (void) const {return true;}

	/// Emit the C++ code to do the translation between the two dudes
	void translate_to_cpp (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const;
	void translate_to_net(const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter, bool use_interface, bool is_static) const;

	/// Return the ROOT type that we contain...
	std::vector<std::string> referenced_root_types (void) const;
private:
	std::string _object_name;

};

