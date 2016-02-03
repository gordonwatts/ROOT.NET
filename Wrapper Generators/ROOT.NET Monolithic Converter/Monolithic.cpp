///
/// Monolithic.cpp
///
///  This is meant mostly as a test driver program. It has two modes, which are only accessible by altering the source: :-)
///
///   - Build individual classes and no other references except for those
///   - Load every .dll that can be found in the library and create wrappers for every single class ROOT then knows about
///
///  Every thing is written into a single set of sub directories. No Solution, etc., is built, however! And source files
///  are not split by library, etc. Ugh!! :-)
///
///  See the first bit of the code to determine what is going on and what to set. This program was designed for use when
///  you want to debug the system. :-)
///
#include "ClassTraversal.h"
#include "ClassInterfaceRepositoryState.hpp"
#include "ClassTranslator.hpp"
#include "CPPNetTypeMapper.hpp"
#include "TTSimpleType.hpp"
#include "TTCPPString.hpp"
#include "WrapperConfigurationInfo.hpp"
#include "ConverterErrorLog.hpp"
#include "SourceEmitter.hpp"
#include "RootEnum.hpp"
#include "RootClassInfo.hpp"
#include "RootClassInfoCollection.hpp"

#include "ROOTHelpers.h"

#include "TSystem.h"
#include "TApplication.h"
#include "TDataType.h"

#include <vector>
#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <stdexcept>

using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::copy;
using std::back_inserter;
using std::set;
using std::ofstream;
using std::exception;

void InitTypeDefs(void);

int main()
{
	///
	/// These control the behavior of the program.
	///

	vector<string> asked_for_class_list;	/// What classes should we translate?
	vector<string> asked_for_enum_list;		/// What enums should we translate?
	vector<string> libraries_to_load;		/// What libraries (dll's) should be loaded before we start translation?
	string output_dir;						/// Where the source code should be written. This directory should already exist!!!
	bool scan_libraries_for_classes = false;/// If true, then every single class that is a member of a loaded library is wrapped,
	/// in addition to the asked_for_class_list.

	///
	/// Setup the above variables and figure out what we are going to do.
	///

	/// The below lines are used during debugging in order to build a single (problem) class.

//#define UNIT_TEST
#ifdef UNIT_TEST
	// These are the classes needed for all the Wrapper unit tests to succeed.
	asked_for_class_list.push_back ("TLorentzVector");
	asked_for_class_list.push_back ("TAttFill");

	asked_for_class_list.push_back ("TTree");
	asked_for_class_list.push_back ("TFile");
#else
	//asked_for_class_list.push_back("TList");
	//asked_for_class_list.push_back("TRootSnifferScanRec");
	//asked_for_class_list.push_back("TIter");
	asked_for_class_list.push_back("TMVA::Factory");
	libraries_to_load.push_back("libTMVA");
	libraries_to_load.push_back("libRHTTP");
#endif

	/// Make sure the libraries that are going to be needed are loaded!
	libraries_to_load.push_back ("libCore");
	libraries_to_load.push_back ("libPhysics");
	libraries_to_load.push_back ("libThread");

	output_dir = "..\\..\\Wrappers\\MonolithicROOTWrapper\\ROOTSource";

	///
	/// -------------------------------------------------------------
	/// Setup is done. Now it is time to actually run the full thing!
	/// Start up our ROOT dude. Make sure it starts in batch.
	///

	int nargs = 2;
	char *argv[2];
	argv[0] = "ROOTWrapperGenerator.exe";
	argv[1] = "-b";
	TApplication *app = new TApplication ("ROOTWrapperGenerator", &nargs, argv);

	///
	/// Get the type system up and running
	///

	WrapperConfigurationInfo::InitTypeTranslators();
	RootClassInfoCollection::SetBadMethods(WrapperConfigurationInfo::GetListOfBadMethods());

	///
	/// If so asked, grab all the classes that are in our libraries. We do the scan no matter what,
	/// mainly because that forces the loading of the libraries. Sure, expensive, but this is a test
	/// harness...
	///

	vector<string> scanned_classes = ROOTHelpers::GetAllClassesInLibraries (libraries_to_load);
	if (scan_libraries_for_classes) {
		copy (scanned_classes.begin(), scanned_classes.end(), back_inserter (asked_for_class_list));
	}

	///
	/// Make sure to re-load the typedefs if not already done. This is because loading up the
	/// libraries above will have altered the typedef list!
	///

	WrapperConfigurationInfo::InitTypeDefs();

	///
	/// Now, in order to convert these reqeusted classes, we need to scan back up the class inheritance list
	/// and add in any missing classes.
	///

	vector<string> all_classes = ClassTraversal::FindAllRelatedClasses (asked_for_class_list);

	///
	/// Remove any classes we should not be doing! Later code will automatically "die" if it can't
	/// build a clas because an inherited class is "missing".
	///

	all_classes = WrapperConfigurationInfo::RemoveBrokenClasses(all_classes);

	///
	/// Inform framework of the list of classes that should be translated. And then do the translation!
	///

	ClassInterfaceRepositoryState rep_state;

	for (unsigned int i = 0; i < all_classes.size(); i++) {
		rep_state.request_class_translation (all_classes[i]);
	}
	for (unsigned int i = 0; i < asked_for_enum_list.size(); i++) {
		rep_state.request_enum_translation (asked_for_enum_list[i]);
	}

	///
	/// Next, for classes with enums in the classes that are public - those will be translated, but
	/// we need to inform the type system of them.
	///

	for (unsigned int i = 0; i < all_classes.size(); i++) {
		auto &info = RootClassInfoCollection::GetRootClassInfo(all_classes[i]);
		for (unsigned ienum = 0; ienum < info.GetClassEnums().size(); ienum++) {
			auto &enumInfo = info.GetClassEnums()[ienum];
			if (enumInfo.NameUnqualified().size() > 0) {
				rep_state.register_enum_translation(enumInfo.NameQualified());
			}
		}
	}

	///
	/// Next, do the translation
	///

	ClassTranslator translator (output_dir);
	translator.include_in_header("root_type_helpers.hpp");

	while (rep_state.enums_to_translate()) {
		string enum_name = rep_state.next_enum();
		cout << "Translating enum " << enum_name << endl;
		translator.translate(RootEnum(enum_name));
	}

	string class_name;
	try {
		while (rep_state.classes_to_translate()) {
			class_name = rep_state.next_class();
			cout << "Translating " << class_name << endl;
			translator.translate (RootClassInfoCollection::GetRootClassInfo(class_name));
		}
		translator.finalize_make_publics();
	} catch (exception &e) {
		cout << "Error processing class '" << class_name << "' - message: '" << e.what() << "'." << endl;
		throw;
	}

	///
	/// Now transltae the global variables that we know about
	///

	cout << "Processing Global Variables" << endl;
	translator.translate_global_variables();

	///
	/// Write out any .hpp files that are going to get required...
	///

	cout << "Writing out final files" << endl;
	SourceEmitter helper_output_hpp (output_dir + "/root_type_helpers.hpp");
	SourceEmitter helper_output_cpp (output_dir + "/root_type_helpers.cpp");

	helper_output_cpp.include_file("root_type_helpers.hpp");

	helper_output_hpp.start_line() << "#ifndef __root_type_helpers__" << endl;
	helper_output_hpp.start_line() << "#define __root_type_helpers__" << endl;
	CPPNetTypeMapper::instance()->write_out_clr_type_support_files(helper_output_hpp);
	helper_output_hpp.start_line() << "#endif" << endl;

	helper_output_cpp.close();
	helper_output_hpp.close();

	///
	/// Finally, dump out all errors!
	///

	ConverterErrorLog::dump(cout);
	cout << endl;
	cout << endl;
	ConverterErrorLog::dump_by_error_order(cout);

	return 0;
}
