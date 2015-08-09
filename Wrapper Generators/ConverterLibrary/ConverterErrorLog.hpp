#pragma once
///
/// Keep track of errors during the conversion process. Attempts to collate them so they can be displayed
/// in some sort of nice way at the end.
///

#include <string>
#include <map>
#include <ostream>

class ConverterErrorLog
{
public:

	/// Main mehods to access
	static void log_type_error (const std::string &type_name, const std::string &message);

	/// Dump out all the errors
	static void dump (std::ostream &output);
	static void dump_by_error_order (std::ostream &output);

private:
	static ConverterErrorLog *_instance;
	static ConverterErrorLog *instance();

	std::map<std::string, std::map<std::string, int> > _type_errors;
};
