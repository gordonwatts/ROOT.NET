#pragma once

#include <string>
#include <map>
#include <ostream>
#include <vector>

class SourceEmitter;

///
/// Deals with mapping between the two type systems. For example, if you have a char * and you need
/// to move to the System::String world, this guy will contain the info to do it.
///
class CPPNetTypeMapper
{
public:
		/// Get the instance for teh "global" singleton.
	static CPPNetTypeMapper *instance(void);
	/// Reset to null the instance -- delete it!
	static void Reset (void);

	/// If you have a cpp type name, this guy will return the typename in the NET world
	std::string GetNetTypename (const std::string cpp_typename);

	/// If you have a cpp type name, this will return the net interface name
	std::string GetNetInterfaceTypename (const std::string cpp_typename);

	class TypeTranslator {
	public:
		inline TypeTranslator (const std::string &dotnetname, const std::string &cppname)
			: _dotnet_name(dotnetname), _cpp_name(cppname)
		{
		}
		inline TypeTranslator (void)
		{
		}

		inline virtual ~TypeTranslator(void)
		{
		}

		virtual std::string net_typename(void) const {
			return _dotnet_name;
		}
		virtual std::string cpp_typename(void) const {
			return _cpp_name;
		}

		/// Returns the core typename -- without "const" or "&" or "*" or anything
		virtual std::string cpp_core_typename(void) const {
			return cpp_typename();
		}

		/// Use if the actual cpp name that we use everywhere isn't what gets used
		/// when we write out the C++ code...
		virtual std::string cpp_code_typename(void) const {
			return cpp_typename();
		}

		/// Returns the .NET name when you want to return when used in code. This help when you want
		/// your items in the header, arguments, return type, and interface to be different from the code.
		virtual std::string net_interface_name (void) const {
			return net_typename();
		}

		/// Use this if you want to alter just the return type. This is good when you have to accept
		/// things as arguments in one form, and return them in another (C++ array translation on return
		/// types generated the need for this method).
		///
		virtual std::string net_return_type_name (void) const {
		  return net_interface_name();
		}

		/// True if this references the object rather than is a value type. So a "&" should do this,
		/// but a * is a pointer, and not a reference.
		virtual bool is_reference_to_object (void) const {
		  return false;
		}

		/// True if some code has to be generated to translate between the two worlds
		virtual bool requires_translation (void) const = 0;

		/// True if translation is required to convert from .NET to CPP
		virtual bool requires_translation_to_cpp (void) const {
			return requires_translation();
		}
		virtual bool requires_translation_to_net (void) const {
			return requires_translation();
		}

		virtual bool requires_cleanup_code (void) const {
			return requires_translation();
		}

		/// Emit the translation code from .NET to CPP
		virtual void translate_to_cpp (const std::string &name_net, const std::string &name_cpp, SourceEmitter &emitter) const
		{
			throw std::runtime_error ("translate_to_cpp should never be called in base class!");
		}

		/// Emit any required cleanup code for .NET to CPP. Made after the method call, always matched to above.
		virtual void translate_to_cpp_cleanup (const std::string &name_net, const std::string &name_cp, SourceEmitter &emitter) const
		{
		}

		/// Return false if no clean up code needs to be emitted even if return value doesn't matter.
		virtual bool clean_up_matters_for_return_value_only (void) const
		{
			return false;
		}

		/// Emit the translation code from CPP to .NET
		// TODO: remove the use_interface hack. Not sure how to clean that up. We need to pass deep into the code, and only for
		// a few of the objects, the fact that we want to return ROOTNET::Object, not ROOTNET::Interface::Object. Not clear how
		// to do that in a "proper" manner.
		virtual void translate_to_net (const std::string &name_net, const std::string &name_cpp, SourceEmitter &emitter, bool use_interface = true, bool is_static = false) const
		{
			throw std::runtime_error ("translate_to_net should never be called in base class!");
		}

		/// If any supporting CLR types have to be written out, here is the spot.
		virtual void write_out_clr_types (SourceEmitter &output)
		{
		}

		virtual std::vector<std::string> referenced_root_types (void) const
		{
			return std::vector<std::string>();
		}

	private:
		const std::string _dotnet_name;
		const std::string _cpp_name;
	};

	/// Returns the translator for a particular type for quick access to various functions.
	const TypeTranslator *get_translator_from_cpp (const std::string &cpp_typename);

	/// Add a new mapper to the library.
	void AddTypeMapper (TypeTranslator *trans);

	/// Add a new type-def -- For example, if Int_t is really int.
	void AddTypedefMapping (const std::string &typedef_name, const std::string &type_name);

	/// Return true if we know about the mapping already
	bool has_mapping (const std::string &class_name);

	/// Write out any extra files that are CLR files (so only need to be written once).
	void write_out_clr_type_support_files (SourceEmitter &output) const;

	/// Simple thing to tell if two types are the same
	bool AreSameType (const std::string &t1, const std::string &t2);

private:
	static CPPNetTypeMapper *_mapper;

	/// ctor is private -- this is a singleton.
	CPPNetTypeMapper(void);

	/// Map of all the translators. This map goes from cpp class name
	std::map<std::string, TypeTranslator*> _cpp_translator_map;

	/// Map of all typedefs. Indexed by typedef.
	std::map<std::string, std::string> _typedef_map;

	/// Resolve the typename
	std::string resolve_typedefs (const std::string &type_name) const;

	/// Resolve a type reference and normalize it (i.e. clean it up!!)
	std::string normalize_type_reference (const std::string &type_name);

	/// Normalize a template reference
	std::string normalize_template_referece (const std::string &type_name);
};
