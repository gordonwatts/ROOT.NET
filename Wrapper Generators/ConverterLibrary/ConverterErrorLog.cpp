#include "ConverterErrorLog.hpp"
///
/// Keep track of errors for a conversion.
///

#include <algorithm>

using std::string;
using std::map;
using std::ostream;
using std::for_each;
using std::pair;
using std::endl;
using std::multimap;

/// Instance access
ConverterErrorLog *ConverterErrorLog::_instance = 0;
ConverterErrorLog *ConverterErrorLog::instance()
{
	if (_instance == 0) {
		_instance = new ConverterErrorLog();
	}
	return _instance;
}

///
/// A new type error. Keep track of it!
///
void ConverterErrorLog::log_type_error(const std::string &type_name, const std::string &message)
{
	ConverterErrorLog *err = ConverterErrorLog::instance();
	err->_type_errors[type_name][message]++;
}

class dump_type_error
{
public:
	inline dump_type_error (ostream &output)
		: _output (output)
	{}
	void operator() (const pair<string, map<string, int> > &item);
private:
	ostream &_output;
};

///
/// Dump all the errors out!
///
void ConverterErrorLog::dump(ostream &output)
{
	output << "Conversion Error Report by Type" << endl;
	ConverterErrorLog *err = ConverterErrorLog::instance();
	for_each(err->_type_errors.begin(), err->_type_errors.end(), dump_type_error (output));
}

class dump_counted_error_message
{
public:
	inline dump_counted_error_message (ostream &output)
		: _output (output)
	{}
	void operator() (const pair<string, int> &item);
private:
	ostream &_output;
};

/// Dump out info for one type of errors
void dump_type_error::operator ()(const std::pair<string,map<string,int> > &item)
{
	_output << item.first << ":" << endl;
	for_each (item.second.begin(), item.second.end(), dump_counted_error_message(_output));
}

void dump_counted_error_message::operator() (const pair<string, int> &item)
{
	_output << "  " << item.second << " - " << item.first << endl;
}

class dump_ordered_error
{
public:
	inline dump_ordered_error (ostream &output)
		: _output(output)
	{}
	inline void operator() (const pair<int, string> &item)
	{
		_output << item.first << " - " << item.second << endl;
	}
private:
	ostream &_output;
};

class extract_error_info
{
public:
	inline extract_error_info (multimap<int, string> &error_table)
		: _error_table(error_table)
	{}
	void operator() (const pair<string, map<string, int> > &item);
private:
	multimap<int, string> &_error_table;
};

/// Dump by in order of the # of errors
void ConverterErrorLog::dump_by_error_order(std::ostream &output)
{
	/// Indexed by # of times it happend, and the string that sumarizes the errro as the target.
	multimap<int, string> error_table;

	ConverterErrorLog *err = ConverterErrorLog::instance();
	for_each (err->_type_errors.begin(), err->_type_errors.end(), extract_error_info (error_table));
	
	for_each (error_table.begin(), error_table.end(), dump_ordered_error (output));
}

class extract_message_info
{
public:
	inline extract_message_info (multimap<int, string> &error_table, const string &class_name)
		: _error_table(error_table), _class_name (class_name)
	{}
	void operator() (const pair<string, int> &item)
	{
		_error_table.insert(make_pair(item.second, _class_name + ": " + item.first));
	}
private:
	multimap<int, string> &_error_table;
	const string _class_name;
};

void extract_error_info::operator()(const pair<string, map<string, int> > &item)
{
	for_each(item.second.begin(), item.second.end(), extract_message_info (_error_table, item.first));
}
