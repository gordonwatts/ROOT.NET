#pragma once

#include "RootClassMethodArg.hpp"
#include "CPPNetTypeMapper.hpp"

#include <string>
#include <vector>

class RootClassInfo;

class TMethod;

/// Info about a method associated with a class.
class RootClassMethod
{
public:
	RootClassMethod(void);
	RootClassMethod(TMethod *method, const RootClassInfo *parent_class, int max_args = -1);

	std::string generate_method_header (bool add_object_qualifier = false, bool use_argument_names = true) const;

	/// Generates the header, but leaves argument names off. This can be used to
	/// compare other methods to see if they are the same, for example.
	std::string generate_normalized_method_header (void) const;

	/// Generate just the argument list, Drop the arg names by default from the header.
	std::string generate_normalized_method_arguments (bool use_argument_names = false) const;

	/// Returns true if this method can be fully translated (i.e. the type system knows about everything).
	/// This guy does not use thrown exceptions to see if something is ok, though it will log the fact that 
	/// certian types are not known.
	bool can_be_translated (void) const;

	/// True if this guy is a constructor.
	bool IsCtor (void) const;

	/// True if this is an inherited ctor (yes, root is weird)
	bool IsOtherObjectCtor (void) const;

	/// True if this guy is a destructor
	bool IsDtor (void) const;

	/// True if it is a virtual method
	bool IsVirtual (void) const;

	/// True if this method over-rides one in the default super-class
	bool IsDefaultOverride (void) const;

	/// True if this is an operator
	bool IsOperator (void) const;

	/// True if this is a math operator (operator+)
	bool IsMathOperator (void) const;

	/// True if it is an index lookup (i.e. operator[]).
	bool IsIndexer(void) const;

	/// True if this is a user defined conversion operator
	bool IsUserConversion (void) const;

	/// True if this is a static user method.
	bool IsStatic (void) const;

	/// Return the name
	std::string CPPName (void) const;
	const std::string &NETName (void) const;

	/// Return full name, qualified with a class if it is inherited
	std::string FullName (void) const;

	/// Return a list of arguments to this method.
	const std::vector<RootClassMethodArg> &arguments (void) const;

	/// Does this guy have a return value?
	bool has_return_value (void) const;

	/// What is the CPP return type?
	std::string return_type (void) const;

	/// Reste the return type -- sometimes used to get around covar stuff
	void SetReturnType (const std::string &return_type);

	/// And the raw one without "*", etc.
	std::string raw_return_type (void) const;

	/// Get the type translator for the return type
	const CPPNetTypeMapper::TypeTranslator *get_return_type_translator(void) const;

	/// List of all the raw (or not) types that we reference -- makes it easy to get everything at once.
	std::vector<std::string> get_all_referenced_types (void) const;
	std::vector<std::string> get_all_referenced_raw_types (void) const;
	std::vector<std::string> get_all_referenced_raw_root_types (void) const;
	std::vector<std::string> get_all_referenced_enums (void) const;

	/// The class name where this method is defined (so the inherited class, perhaps...)
	std::string ClassOfMethodDefinition (void) const;

	/// Is this method ambiguous?
	inline bool IsAmbiguous (void) const {return _is_ambiguous;}
	void SetAmbiguous (bool is_ambiguous) const {
		_is_ambiguous = is_ambiguous;
	}

	/// Is this method a hidden one -- and hiding others in the interface chain?
	inline bool IsHidden (void) const {return _is_hidden;}
	void SetHidden (bool is_hidden) const {
		_is_hidden = is_hidden;
	}

	/// Should we skip emitting any code at all for this method?
	inline bool IsToBeSkipped (void) const {return _skip_method;}
	inline void SetSkip (bool do_skip) {
		_skip_method = do_skip;
	}

	/// Is this method a covar return violator?
	inline bool IsCovarReturn (void) const {return _covar_method;}
	inline void SetCovarReturn (bool is_covar) {
		_covar_method = is_covar;
	}

	/// True if this meets conditions to be emitted as an interface
	bool IsGoodForInterface (void) const;

	/// True if this meets conditions to be emitted as C++ .NET method
	bool IsGoodForClass (void) const;

	/// True if this method is const -- it leaves the class as a "const"
	bool IsConst (void) const;

	/// Returns true if the signature is identical (e.g. over-riding style equal).
	bool is_equal (const RootClassMethod &other, bool consider_return_type = false) const;

	/// Returns true if the other class is less than this one -- allows for ordering!
	bool is_less_than (const RootClassMethod &other, bool consider_return_type = false) const;

	/// Return the parent class - the class that holds onto this object.
	const RootClassInfo &OwnerClass() const {return *_parent;}

	void ResetParent (const RootClassInfo *parent) {_parent = parent;}

private:
	TMethod *_root_method_info;
	const RootClassInfo *_parent;
	mutable bool _is_ambiguous;
	mutable bool _is_hidden;
	bool _skip_method;
	bool _covar_method;
	int _number_args;
	mutable std::string _return_type;
	mutable std::string _netname;

	mutable bool _is_user_conversion;
	mutable bool _is_user_conversion_good;

	mutable bool _args_good;
	mutable std::vector<RootClassMethodArg> _args;

	mutable bool _superclass_ptr_good;
	mutable RootClassInfo *_superclass_ptr;

	// Some fairly expensive, but often called, items that we calc ahead of time.
	bool _is_operator;
	bool _is_math_operator;
	bool _is_dtor;
	void ParseMethodInfo (void);

	/// Returns true if this looks like a root class...
	bool is_root_class (const std::string &class_name) const;


};
