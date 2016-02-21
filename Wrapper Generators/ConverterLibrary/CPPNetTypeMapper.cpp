#include "CPPNetTypeMapper.hpp"
#include "ConverterErrorLog.hpp"
#include "TTROOTClass.hpp"
#include "TTROOTClassVector.hpp"
#include "RootClassInfoCollection.hpp"

#include <stdexcept>
#include <algorithm>

using std::string;
using std::map;
using std::runtime_error;
using std::for_each;
using std::ostream;
using std::pair;

CPPNetTypeMapper *CPPNetTypeMapper::_mapper = 0;

CPPNetTypeMapper::CPPNetTypeMapper(void)
{
}

///
/// Return the singleton. Create if we have to.
///
CPPNetTypeMapper* CPPNetTypeMapper::instance()
{
	if (_mapper == 0) {
		_mapper = new CPPNetTypeMapper();
	}
	return _mapper;
}

///
/// Reset to zero -- delete the thing!
/// Used mostly in tests...
///
void CPPNetTypeMapper::Reset (void)
{
  delete _mapper;
  _mapper = 0;
}

///
/// Returns a .net type name. Exception if it can't find it!
///
string CPPNetTypeMapper::GetNetTypename(const std::string cpp_typename)
{
  const TypeTranslator *t (get_translator_from_cpp(cpp_typename));
  return t->net_typename();
}

///
/// Returns a .net type name. Exception if it can't find it!
///
string CPPNetTypeMapper::GetNetInterfaceTypename(const std::string cpp_typename)
{
	return get_translator_from_cpp(cpp_typename)->net_interface_name();
}

///
/// Add a type translator to our list of type translators...
///
void CPPNetTypeMapper::AddTypeMapper(CPPNetTypeMapper::TypeTranslator *trans)
{
	_cpp_translator_map[trans->cpp_typename()] = trans;
}

///
/// Add a typedef mapping.
///
void CPPNetTypeMapper::AddTypedefMapping(const std::string &typedef_name, const std::string &type_name)
{
	if (typedef_name != type_name) {
		_typedef_map[typedef_name] = type_name;
	}
}

///
/// Return the type transltor for a particular type
///
const CPPNetTypeMapper::TypeTranslator *CPPNetTypeMapper::get_translator_from_cpp(const std::string &cpp_typename)
{
	map<string, TypeTranslator*>::const_iterator itr = _cpp_translator_map.find(normalize_type_reference(cpp_typename));
	if (itr == _cpp_translator_map.end()) {
		ConverterErrorLog::log_type_error (normalize_type_reference(cpp_typename), "No .NET translator available");
		throw runtime_error ("Unable to translate cpp type " + cpp_typename + " (lookup as " + normalize_type_reference(cpp_typename) + ")");
	}
	return itr->second;
}

///
/// We normalize all type references. So, for example, template references we know how to
/// deal with, typedefs are resolved, etc.
///
string CPPNetTypeMapper::normalize_type_reference (const string &in_name)
{
	///
	/// If this is a template, then process it
	///

	if (in_name.find("<") != in_name.npos) {
		return normalize_template_referece(in_name);
	}

	///
	/// Ok - very simple guy - just ship it out.
	///

	return resolve_typedefs(in_name);
}

///
/// Pick a part a type reference for a template
///
string CPPNetTypeMapper::normalize_template_referece(const string &in_name)
{
	/// If this is a vector guy, then perhaps we can deal with it.
	auto loc_start = in_name.find("<");
	auto loc_end = in_name.rfind(">");
	auto template_definition = in_name.substr(0, loc_start);
	if (template_definition.find("vector") != template_definition.npos) {
		auto arg = in_name.substr(loc_start + 1, loc_end - loc_start - 1);

		///
		/// We can't deal with nestaed arguments or with an allocator.
		///

		if (arg.find(",") != arg.npos) {
			arg = arg.substr(0, arg.find(","));
		}

		if (arg.find("<") != arg.npos) {
			return in_name;
		}

		///
		/// If this something that is referencing a TObject (i.e. vector<TObject>) or something
		/// that comes from TObject, then we want to make sure that we can deal with this vector!
		///

		if (!instance()->has_mapping(arg)) {
			return in_name;
		}
		auto trans = instance()->get_translator_from_cpp(arg);
		if (trans == 0) {
			return in_name;
		}

		///
		/// If it isn't a root class, we won't dynamically create
		/// a new guy - but there may be something already up and going for it
		///

		auto template_postfix = in_name.substr(loc_end + 1);
		if (dynamic_cast<const TTROOTClass*>(trans) == 0) {
			return template_definition + "<" + trans->cpp_code_typename() + ">" + template_postfix;
		}

		// The Class Vector holder only works with TObject derived things,
		// so we will have to watch for that.

		auto info = RootClassInfoCollection::GetRootClassInfoPtr(trans->cpp_core_typename());
		if (info != 0 && !info->InheritsFromTObject()) {
			return template_definition + "<" + trans->cpp_code_typename() + ">" + template_postfix;
		}

		///
		/// This is a TObject inherrited object. We create a vector class translator on the fly
		/// for that.
		///

		string normalized_name = template_definition + "<" + trans->cpp_code_typename() + ">" + template_postfix;

		if (_cpp_translator_map.find(normalized_name) == _cpp_translator_map.end())
		{
			CPPNetTypeMapper::AddTypeMapper(new TTROOTClassVector(arg));
		}

		return normalized_name;


	} else {
		return in_name;
	}
}

///
/// Resolve any typedef references
///
string CPPNetTypeMapper::resolve_typedefs (const string &in_name) const
{
  string name (in_name);
  bool is_const = false;
  if (name.find("const ") == 0) {
	is_const = true;
	name = name.substr(6);
  }
  map<string, string>::const_iterator itr;
  string found_type (name);
  int count = 0;
  while ((itr = _typedef_map.find(found_type)) != _typedef_map.end()) {
	found_type = itr->second;
	count++;
	if (count > 100) {
	  ConverterErrorLog::log_type_error(name, "Circular/recursive type definition detected.");
	  throw runtime_error ("Typedef " + name + " has a circular typedef definition -- giving up!");
	}
  }
  if (is_const) {
	found_type = "const " + found_type;
  }

  return found_type;
}

///
/// has_mapping
///
///  Do we know about a particular mapping?
///
bool CPPNetTypeMapper::has_mapping (const string &class_name)
{
	return _cpp_translator_map.find(normalize_type_reference(class_name)) != _cpp_translator_map.end();
}

namespace {
  class write_out_types {
  public:
	inline write_out_types (SourceEmitter &output)
	  : _output (output)
	{}
	inline void operator() (const pair<string, CPPNetTypeMapper::TypeTranslator*> &item)
	{
	  item.second->write_out_clr_types(_output);
	}
  private:
	SourceEmitter &_output;
  };
}

///
/// write out type support files. Sometimes the type translators need to author there
/// own support types. They can't do this inline, so we give them a chance to write
/// everything to a seperate file.
///
void CPPNetTypeMapper::write_out_clr_type_support_files(SourceEmitter &output) const
{
  for_each (_cpp_translator_map.begin(), _cpp_translator_map.end(),
	write_out_types (output));
}

///
/// Are the two given types actually the same?
///
bool CPPNetTypeMapper::AreSameType (const std::string &t1, const std::string &t2)
{
  return normalize_type_reference(t1) == normalize_type_reference(t2);
}

