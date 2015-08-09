#pragma once

#include <string>

class TMethodArg;

class RootClassMethodArg
{
public:
	RootClassMethodArg(void);
	RootClassMethodArg (TMethodArg *arg);

	/// Returns the CPP type name
	std::string CPPTypeName (void) const;

	/// Returns the NET type name
	std::string NETTypeName (void) const;

	/// Returns the NET interface name
	std::string NETInterfaceTypeName (void) const;

	/// Returns the default name of the argument
	std::string get_argname (void) const;

	/// Reset arg counter - for when args have no name (!!)
	static void reset_arg_counter (void);

	  /// Return the raw type
	std::string RawCPPTypeName (void) const;

	/// Return true if we can translate this guy
	bool can_be_translated (void) const;

	/// If we need to fake out a different type
	void ResetType (const std::string &full_type_name, const std::string &raw_type_name);

private:
	int _index;

	std::string _arg_name;
	std::string _arg_type;
	std::string _arg_raw_type;

	static int _counter;
};
