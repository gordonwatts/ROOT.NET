///
/// Code that contains common configuration.
///

#include "WrapperConfigurationInfo.hpp"
#include "CPPNetTypeMapper.hpp"
#include "RootClassInfo.hpp"
#include "RootClassMethod.hpp"
#include "RootClassInfoCollection.hpp"
#include "RootClassMethodArg.hpp"
#include "ConverterErrorLog.hpp"
#include "ROOTHelpers.h"

#include "FeatureManager.hpp"
#include "FeatureTIterEnumerable.hpp"
#include "FeatureTCollectionEnumerable.hpp"
#include "FeatureTreeIterator.hpp"

#include "TTSimpleType.hpp"
#include "TPointerSimpleType.hpp"
#include "TTCPPString.hpp"
#include "TTSimpleReference.hpp"
#include "TArrayOfChar.hpp"
#include "TVoidPointer.hpp"
#include "TVectorArray.hpp"

#include "TROOT.h"
#include "TDataType.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include "shlobj.h"

using std::vector;
using std::string;
using std::set;
using std::map;
using std::find;
using std::cout;
using std::endl;
using std::ifstream;
using std::getline;

std::map<std::string, std::vector<std::string> > WrapperConfigurationInfo::_allowed_library_links;
std::map<std::string, std::vector<std::string> > WrapperConfigurationInfo::_disallowed_library_links;
bool WrapperConfigurationInfo::_allowed_library_links_ok = false;
string WrapperConfigurationInfo::_template_directory;

void InitTypeDefs (void);

/// Remove all classes that are in the bad class list file
vector<string> WrapperConfigurationInfo::RemoveBrokenClasses (const vector<string> &class_list)
{
	///
	/// Load in the bad headers list
	///

	set<string> bad_headers;
	{
		ifstream input ("bad_headers.txt");
		string line;
		while (!getline(input, line).fail()) {
			bad_headers.insert(line);
		}
	}

	///
	/// Now, just march through and find the bad ones!
	///

	vector<string> result;
	for (unsigned int i = 0; i < class_list.size(); i++) {
		///
		/// Some basic things we can't do yet, like a STL string and all iterators. We know how to translate
		/// the iterators as arguments, but not as an individual wrapper.
		///

		if (class_list[i] == "string" || class_list[i] == "std::string") {
			ConverterErrorLog::log_type_error(class_list[i], "Can't convert STL classes like string!");
			continue;
		}

		if (class_list[i].find("<") != string::npos) {
			continue;
		}

		///
		/// See if the list is on the bad-list error from the previous scan.
		///

		RootClassInfo &info (RootClassInfoCollection::GetRootClassInfo(class_list[i]));
		if (bad_headers.find(info.include_filename()) == bad_headers.end()) {
			result.push_back(class_list[i]);
		} else {
			ConverterErrorLog::log_type_error(class_list[i], "Class was on bad-class list (its header doesn't build stand alone)");
		}
	}

	return result;
}

///
/// Return a list of bad fields.
///
set<string> WrapperConfigurationInfo::GetListOfBadFields()
{
	set<string> bad_fields;

	// These are here because the CINT and C++ definitions are different.
	bad_fields.insert("TGenCollectionProxy::Value::fDtor");
	bad_fields.insert("TGenCollectionProxy::Value::fDelete");
	bad_fields.insert("TGenCollectionProxy::Value::fCtor");
	bad_fields.insert("TGenCollectionProxy::Method::call");
	bad_fields.insert("TGenCollectionProxy::Value::fDtor");
	bad_fields.insert("TGenCollectionProxy::Value::fDtor");
	bad_fields.insert("TStreamerInfoActions::TConfiguredAction::fAction");
	bad_fields.insert("TStreamerInfoActions::TConfiguredAction::fLoopAction");
	bad_fields.insert("TStreamerInfoActions::TConfiguredAction::fVecPtrLoopAction");

	return bad_fields;
}

///
/// Return a list of methods that we cna't translate for whatever reason. We
/// do this by svn revision number. So this requires some care and feeding.
///
set<string> WrapperConfigurationInfo::GetListOfBadMethods()
{
	//
	// Some of these methods refer to things that depend on the release.
	//

	auto svn_id = gROOT->GetSvnRevision();
	cout << "SVN Revision Number for ROOT is " << svn_id << endl;

	set<string> methods_to_skip;

	///
	/// ::Error turns out to be an ambiguous method - because we can't detect
	/// it properly (currently) we are going to just not translate anything
	/// having to do with it!
	///

	methods_to_skip.insert("TFitResult::Error");
	methods_to_skip.insert("FitResult::Error");
	methods_to_skip.insert("ROOT::Fit::FitResult::Error");

	///
	/// While .NET can deal with covariant returns, my code is having
	/// some trouble. So, till this becomes a higher priority, we are
	/// going to ignore them. :-)
	///

	methods_to_skip.insert("TGMainFrame::GetMainFrame");
	methods_to_skip.insert("TGFrame::GetMainFrame");
	methods_to_skip.insert("TGWindow::GetMainFrame");
	methods_to_skip.insert("TRootBrowser::GetMainFrame");
	methods_to_skip.insert("TRootBrowserLite::GetMainFrame");

	methods_to_skip.insert("TGFrame::GetDragType");
	methods_to_skip.insert("TVirtualDragManager::GetDragType");

	//
	// For C++ you can specify just a class reference in one object if you use it as a pointer
	// in another. For us to do that in ROOT.NET we'd have to build one library of interfaces and
	// one library of the objects, unfortunately. Since I've elected not to x2 the # of libraries,
	// there are a few places where we much watch out for this. My code can't resolve these at the
	// moment, so we just put them in here.
	//

	// TFileCacheRead - used in libRIO, but depends on libTreeWrapper, which depends on libNet, which depends on libRIO.
	methods_to_skip.insert("TFileCacheRead::AddBranch");

	// Marker_t is both an inner class and a typedef. As a result it currently gets converted to short*,
	// rather than its inner object (which is not something we support yet). This will go away when we
	// add the support for nested classes.
	methods_to_skip.insert("TEveScalableStraightLineSet::AddMarker");
	methods_to_skip.insert("TEveStraightLineSetProjected::AddMarker");
	methods_to_skip.insert("TEveStraightLineSet::AddMarker");

	///
	/// Next are version specific things that have to be not-translated.
	/// Most of the time these are due to bad linkages in ROOT - that Windows
	/// can't deal with. Note that the bulk of these were determined before
	/// we started doing this by version number. As a result, many of them
	/// are just lumped in.
	///

	if (svn_id < 31883)
	{
		///
		/// 5.26 or earlier - these may not apply, but that was the last
		/// time we were careful!
		///

		/// TStyleManager::fgStyleManager
		methods_to_skip.insert("TStyleManager::GetSM");

		/// Some bad globals in RooFitCore library
		// RooAbsPdf::_globalSelectComp
		methods_to_skip.insert("RooAbsPdf::isSelectedComp");
		// RooNumber::infinity (RooBinning) - used in the ctor of RooBinning
		methods_to_skip.insert("RooBinning::RooBinning");
		methods_to_skip.insert("RooNormListManager::setVerbose");
		methods_to_skip.insert("RooErrorVar::removeMin");
		methods_to_skip.insert("RooErrorVar::removeMax");
		methods_to_skip.insert("RooErrorVar::removeRange");
		methods_to_skip.insert("RooRealVar::removeMin");
		methods_to_skip.insert("RooRealVar::removeMax");
		methods_to_skip.insert("RooRealVar::removeRange");
		// RooAbsReal::_doLogEvalError
		methods_to_skip.insert("RooAbsReal::evalErrorLoggingEnabled");
		methods_to_skip.insert("RooAbsReal::enableEvalErrorLogging");
	
		// TEveLine::fgDefaultSmooth
		methods_to_skip.insert("TEveLine::GetDefaultSmooth");
		methods_to_skip.insert("TEveLine::SetDefaultSmooth");

		// TEveTrackProjected::fgBreakTracks
		methods_to_skip.insert("TEveTrackProjected::GetBreakTracks");
		methods_to_skip.insert("TEveTrackProjected::SetBreakTracks");

		/// Bad globals from libTable
		// TDataSet(fgMainSet)
		methods_to_skip.insert("TDataSet::GetMainSet");
		methods_to_skip.insert("TDataSet::AddMain");
		// TTableDescriptor(fgColDescriptors)
		methods_to_skip.insert("TTableDescriptor::GetDescriptorPointer");
		methods_to_skip.insert("TTableDescriptor::SetDescriptorPointer");
		methods_to_skip.insert("TChair::GetRowDescriptors");
		// TDataSetIter::fgNullDataSet
		methods_to_skip.insert("TDataSetIter::operator* ()");
		methods_to_skip.insert("TDataSetIter::GetNullSet");
		// TIndexTable::fgColDescriptors
		methods_to_skip.insert("TIndexTable::GetDescriptorPointer");
		methods_to_skip.insert("TIndexTable::SetDescriptorPointer");

		/// Bad globsl from libRGL
		// TGLSelectBuffer::fbMaxSize is another undefined symbol.
		methods_to_skip.insert("TGLSelectBuffer::CanGrow");
		// TGLUtil::fgDrawQuality is missing
		methods_to_skip.insert("TGLUtil::SetDrawQuality");
		methods_to_skip.insert("TGLUtil::ResetDrawQuality");
		// TGLUtil::fgDefaultDrawQuality is missing
		methods_to_skip.insert("TGLUtil::GetDefaultDrawQuality");
		methods_to_skip.insert("TGLUtil::SetDefaultDrawQuality");
		// TGLUtil::fgColorLockCount
		methods_to_skip.insert("TGLUtil::IsColorLocked");
	
		/// libVMC
		// Missing TVirtualMC::fgMC;
		methods_to_skip.insert("TVirtualMC::GetMC");

		/// libGeomBuilder
		// TGeoTreeDialog::fgSelectedObj
		methods_to_skip.insert("TGeoTreeDialog::GetSelected");

		/// libFitPanelWrapper
		// TFitEditor::fgFitDialog is missing
		methods_to_skip.insert("TFitEditor::GetFP");

	}
	if (svn_id < 37603)
	{
		/// 5.28 and friends
	} 
	if (svn_id < 40633)
	{
		/// 5.30 and friends
		//
		// Things that exist in the head. These are mostly global variables that aren't referenced in anything
		// but the header files.
		//

		// RooAbsReal::_evalErrorList
		methods_to_skip.insert("RooAbsReal::numEvalErrorItems");
		methods_to_skip.insert("RooAbsReal::evalErrorIter");

		// RooFactorWSTool::_of
		methods_to_skip.insert("RooFactoryWSTool::as_ARG");
		methods_to_skip.insert("RooFactoryWSTool::as_PDF");
		methods_to_skip.insert("RooFactoryWSTool::as_FUNC");
		methods_to_skip.insert("RooFactoryWSTool::as_VAR");
		methods_to_skip.insert("RooFactoryWSTool::as_VARLV");
		methods_to_skip.insert("RooFactoryWSTool::as_RMODEL");
		methods_to_skip.insert("RooFactoryWSTool::as_CAT");
		methods_to_skip.insert("RooFactoryWSTool::as_CATLV");
		methods_to_skip.insert("RooFactoryWSTool::as_CATFUNC");
		methods_to_skip.insert("RooFactoryWSTool::as_SET");
		methods_to_skip.insert("RooFactoryWSTool::as_LIST");
		methods_to_skip.insert("RooFactoryWSTool::as_DATA");
		methods_to_skip.insert("RooFactoryWSTool::as_DHIST");
		methods_to_skip.insert("RooFactoryWSTool::as_DSET");
		methods_to_skip.insert("RooFactoryWSTool::as_OBJ");
		methods_to_skip.insert("RooFactoryWSTool::as_STRING");
		methods_to_skip.insert("RooFactoryWSTool::as_INT");
		methods_to_skip.insert("RooFactoryWSTool::as_DOUBLE");

		// RooRealIntegral::_cacheAllNDim
		methods_to_skip.insert("RooRealIntegral::setCacheAllNumeric");
		methods_to_skip.insert("RooRealIntegral::getCacheAllNumeric");
	}

	/// 5.30/1 and friends
	methods_to_skip.insert("RooObjCacheManager::doClearObsList");
	methods_to_skip.insert("RooObjCacheManager::clearObsList");

	// 5.32.03 and friends
	if (svn_id <= 44194)
	{
		methods_to_skip.insert("TGeoManager::SetNavigatorsLock");
		methods_to_skip.insert("TGeoManager::GetNumThreads");
		methods_to_skip.insert("TGeoGlobalMagField::GetInstance");
	}

	// This has been there a long time...

	methods_to_skip.insert("RooRealSumPdf::setFloorGlobal");
	methods_to_skip.insert("RooRealSumPdf::getFloorGlobal");

	// Some items in the gui code

	methods_to_skip.insert("TBrowserImp::GetMainFrame");

	// Issues that are differences between CINT and C++
	// If needed, many of these could be fixed by the fix up bad args call elsewhere in this
	// code.
	methods_to_skip.insert("ROOT::Math::Functor::Functor");
	methods_to_skip.insert("ROOT::Math::Functor1D::Functor1D");
	methods_to_skip.insert("ROOT::Math::GradFunctor::GradFunctor");
	methods_to_skip.insert("ROOT::Math::GradFunctor1D::GradFunctor1D");
	methods_to_skip.insert("ROOT::Math::ParamFunctor::ParamFunctor");
	methods_to_skip.insert("TObject::Error");
	methods_to_skip.insert("ROOT::Fit::Fitter::FitFCN");
	methods_to_skip.insert("ROOT::Fit::Fitter::SetFCN");
	methods_to_skip.insert("TGenCollectionProxy::Method::Method");
	methods_to_skip.insert("ROOT::Math::ChebyshevApprox::ChebyshevApprox");
	methods_to_skip.insert("ROOT::Math::GSLIntegrator::Integral");
	methods_to_skip.insert("ROOT::Math::GSLIntegrator::IntegralLow");
	methods_to_skip.insert("ROOT::Math::GSLIntegrator::IntegralUp");
	methods_to_skip.insert("ROOT::Math::GSLIntegrator::SetFunction");
	methods_to_skip.insert("ROOT::Math::GSLMinimizer1D::SetFunction");
	methods_to_skip.insert("ROOT::Math::GSLRootFinder::SetFunction");

	// Bizare duplication of const and non-const functions that have only arguments by value.
	methods_to_skip.insert("TMVA::kNN::Event::GetDist");
	methods_to_skip.insert("TMVA::kNN::Event::GetTgt");
	methods_to_skip.insert("TMVA::kNN::Event::GetVar");

	// Postfix ++ and -- are not valid CLR... perhaps need something more general, but for now...
	methods_to_skip.insert("TGenericTable::iterator::operator++");
	methods_to_skip.insert("TGenericTable::iterator::operator--");
	methods_to_skip.insert("TIndexTable::iterator::operator++");
	methods_to_skip.insert("TIndexTable::iterator::operator--");
	methods_to_skip.insert("TTable::iterator::operator++");
	methods_to_skip.insert("TTable::iterator::operator--");

	methods_to_skip.insert("TIndexTable::iterator::operator-");

	// Here are a few undefined static variable references that aren't done right in TMVA:
	methods_to_skip.insert("TMVA::BDTEventWrapper::SetVarIndex");
	methods_to_skip.insert("TMVA::BDTEventWrapper::GetVal");
	methods_to_skip.insert("TMVA::BDTEventWrapper::operator<");
	methods_to_skip.insert("TMVA::DecisionTree::DecisionTree");
	methods_to_skip.insert("TMVA::Factory::RootBaseDir");
	methods_to_skip.insert("TMVA::LogInterval::Log");
	methods_to_skip.insert("TMVA::MsgLogger::GetMaxSourceSize");

	//
	// Return the list
	//

	return methods_to_skip;
}

///
/// Init the type translation system
///
void WrapperConfigurationInfo::InitTypeTranslators()
{
	///
	/// Initialize the ROOT typedef mappers...
	///

	InitTypeDefs();

	///
	/// Now, the simple type mappers for things that are so basic we can't translate them.
	///

	char *simple_types[] = {"int", "long", "double", "float", "bool", "short", "long long"};
	bool canbe_unsigned[] = {true, true, false, false, false, true, true};
	int n_simple_types = sizeof(simple_types)/sizeof(char*);

	for (int i = 0; i < n_simple_types; i++) {
	  string simple_type (simple_types[i]);

	  CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleType (simple_type, simple_type));
	  CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleType ("const " + simple_type, "const " + simple_type));
	  CPPNetTypeMapper::instance()->AddTypeMapper (new TPointerSimpleType (simple_type));
	  CPPNetTypeMapper::instance()->AddTypeMapper (new TPointerSimpleType ("const " + simple_type, true));
	  CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleReference (simple_type));
	  CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleReference ("const " + simple_type, true));
	  CPPNetTypeMapper::instance()->AddTypeMapper (new TVectorArray(simple_type));

	  if (canbe_unsigned[i]) {
		CPPNetTypeMapper::instance()->AddTypeMapper (new TVectorArray("unsigned " + simple_type));
	    CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleType ("unsigned " + simple_type, "unsigned " + simple_type));
	    CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleType ("const unsigned " + simple_type, "const unsigned " + simple_type));
	    CPPNetTypeMapper::instance()->AddTypeMapper (new TPointerSimpleType ("unsigned " + simple_type));
	    CPPNetTypeMapper::instance()->AddTypeMapper (new TPointerSimpleType ("const unsigned " + simple_type, true));
	    CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleReference ("unsigned " + simple_type));
	    CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleReference ("const unsigned " + simple_type, true));
	  }
	}

	///
	/// void* - which we deal with as just a TObject of sorts...
	///

	CPPNetTypeMapper::instance()->AddTypeMapper(new TVoidPointer());

	///
	/// Char we have to handle specially since it can become a string -- and we don't want to be passing
	/// arrays back and forth. ;-) [Good way to piss people off!]
	///

	CPPNetTypeMapper::instance()->AddTypeMapper (new TTCPPString());
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTCPPString("char*"));

	CPPNetTypeMapper::instance()->AddTypeMapper (new TArrayOfChar ("char**", false));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleType ("unsigned char", "unsigned char"));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleType ("char", "char"));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleType ("const char", "const char"));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TPointerSimpleType ("unsigned char"));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TPointerSimpleType ("const unsigned char", true));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleReference ("char"));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleReference ("unsigned char", false));

	CPPNetTypeMapper::instance()->AddTypeMapper (new TVectorArray("char"));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TPointerSimpleType ("char", false, true));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TPointerSimpleType ("const char", true, true));

	///
	/// Finally, setup all the features for the classes we are translating
	///

	FeatureManager::AddFeature(new FeatureTIterEnumerable());
	FeatureManager::AddFeature(new FeatureTCollectionEnumerable());
	FeatureManager::AddFeature(new FeatureTreeIterator());
}

void DefineTypeDef (const string &typedef_name, const string &base_name)
{
  CPPNetTypeMapper::instance()->AddTypedefMapping (typedef_name, base_name);
  CPPNetTypeMapper::instance()->AddTypedefMapping (typedef_name + "*", base_name + "*");
  CPPNetTypeMapper::instance()->AddTypedefMapping (typedef_name + "&", base_name + "&");

  CPPNetTypeMapper::instance()->AddTypedefMapping ("const " + typedef_name, "const " + base_name);
  CPPNetTypeMapper::instance()->AddTypedefMapping ("const " + typedef_name + "*", "const " + base_name + "*");
  CPPNetTypeMapper::instance()->AddTypedefMapping ("const " + typedef_name + "&", "const " + base_name + "&");
}

///
/// Look through ROOT's typedefs and put translations into our library
///
void WrapperConfigurationInfo::InitTypeDefs (void)
{
	// When we get the list of types in ROOT to look for typedefs, make sure to tell it to go back to CINT
	// and RE-init the list (the "true").
	TIter i_typedef (gROOT->GetListOfTypes(true));
	int junk = gROOT->GetListOfTypes()->GetEntries();
	TDataType *typedef_spec;
	while ((typedef_spec = static_cast<TDataType*>(i_typedef.Next())) != 0)
	{
		string typedef_name = typedef_spec->GetName();
		string base_name = typedef_spec->GetFullTypeName();

		///
		/// typedefs for function definitions we don't allow. This is a bit of
		/// a kludge work-around, but not sure exactly how to check...
		///

		//G__TypedefInfo g (typedef_name.c_str());
		//string t1 = g.Name();
		//string t2 = g.TrueName();
		//auto t5 = g.Value();
		if (string(typedef_spec->GetTitle()).find("(*") != string::npos) {
			continue;
		}

		///
		/// Special cases. These are required because in many places ROOT lies to TCint and defines the type as void* when in C++
		/// it is #ifdef'd to be a real function definition.
		///

		if (typedef_name.find("Func_t") != string::npos
			|| typedef_name.find("Fun_t") != string::npos) {
			continue;
		}

		///
		/// Check to see if it is a pointer or is a const and modify accordingly.
		/// TODO: This code doesn't work, but that, according to Axel, is because it is broken. Wait for REFLEX I guess.
		///

		if (typedef_name == "Option_t") {
			base_name = "const char";
		}

#ifdef notyet
		cout << typedef_name << " -> " << base_name << " (flags: " << std::hex << typedef_spec->Property() << ")" << endl;

		if (typedef_spec->Property() & G__BIT_ISCONSTANT) {
			base_name = "const " + base_name;
		}
		if (typedef_spec->Property() & kIsPointer) {
			base_name += "*";
		}
#endif

		//
		// Ironically, we just aren't ready to handle wchar_t. This is because char * and wchar_t* are both
		// gonig to want to translate to System::String. So if you have both methods, then you'll end up with...
		// problems because you'll have the same method defined twice.
		//

		if (typedef_name.find("wchar_t") != string::npos)
			continue;

		///
		/// Now, add several varriations of the typedef to the translator - including the const because
		/// our typdef translator isn't very smart!!! :-)
		///

		DefineTypeDef (typedef_name, base_name);
	}

	///
	/// There are some that seem to be left out as well...
	///

	DefineTypeDef ("Text_t", "char");
}

/// Do we know about allowed libraries for this guy?
bool WrapperConfigurationInfo::CheckAllowedLibraries(const std::string &library_name)
{
	init_allowed_library_links();
	return _allowed_library_links.find(library_name) != _allowed_library_links.end();
}
/// Do we know about disallowed libraries?
bool WrapperConfigurationInfo::CheckDisallowedLibraries(const std::string &library_name)
{
	init_allowed_library_links();
	return _disallowed_library_links.find(library_name) != _disallowed_library_links.end();
}

/// Certian (and not) libraries are built to have very restricted linkages. This is mostly to cut
/// circular dependences.
bool WrapperConfigurationInfo::IsLibraryLinkRestricted(const std::string &library_name)
{
	return CheckAllowedLibraries(library_name) || CheckDisallowedLibraries(library_name);
}

///
/// Return the list of libraries that this guy is allowed to link to!
///
vector<string> WrapperConfigurationInfo::AllowedLibraryLinks(const std::string &library_name)
{
	init_allowed_library_links();

	map<string, vector<string> >::const_iterator items = _allowed_library_links.find(library_name);
	if (items == _allowed_library_links.end()) {
		return vector<string> (); // SHould never happen! Call IsResitrctedFirst!!!
	}
	return items->second;
}

///
/// Return the list of libraries that this guy is not allowed to link to!
///
vector<string> WrapperConfigurationInfo::DisallowedLibraryLinks(const std::string &library_name)
{
	init_allowed_library_links();

	map<string, vector<string> >::const_iterator items = _disallowed_library_links.find(library_name);
	if (items == _disallowed_library_links.end()) {
		return vector<string> (); // SHould never happen! Call IsResitrctedFirst!!!
	}
	return items->second;
}

///
/// Setup the list of allowed links.
///
void WrapperConfigurationInfo::init_allowed_library_links()
{
	if (_allowed_library_links_ok) {
		return;
	}
	_allowed_library_links_ok = true;

	/// libCore can't link to anything. We have to start somewhere! :-)
	_allowed_library_links["libCore"] = vector<string> ();

	/// libTreePlayer links to libTree. Don't allow the reverse!
	_disallowed_library_links["libTree"].push_back("libTreePlayer");

	/// libGpad can't depend on the open GL stuff
	_disallowed_library_links["libGpad"].push_back("libRGL");

	/// libNet can't depend on libProof!
	_disallowed_library_links["libNet"].push_back("libProof");
	_disallowed_library_links["libNet"].push_back("libTree");

	/// Proof can't depend on the proof player!
	_disallowed_library_links["libProof"].push_back("libProofPlayer");
}

//
// List of libraries we shouldn't try to translate!
//
vector<string> WrapperConfigurationInfo::GetListOfBadLibraries(void)
{
	vector<string> bad_libs;
	bad_libs.push_back("gdk-1.3");
	bad_libs.push_back("iconv-1.3");
	bad_libs.push_back("libEGPythia6");
	bad_libs.push_back("libFFTW");
	bad_libs.push_back("libHbook");
	bad_libs.push_back("libOracle");
	bad_libs.push_back("libPyROOT");
	bad_libs.push_back("libQtGSI");
	bad_libs.push_back("libRMySQL");
	bad_libs.push_back("libNew");
	bad_libs.push_back("libCintex");
	bad_libs.push_back("libReflex");
	bad_libs.push_back("libGviz");
	bad_libs.push_back("libRCastor");
	bad_libs.push_back("libRFIO");
	bad_libs.push_back("libFITSIO"); // Fit library is not distributed with ROOT, so this just fails.
	return bad_libs;
}

///
/// Remove libraries we know we can't translate from the list!
///
vector<string> WrapperConfigurationInfo::RemoveBadLibraries(const std::vector<std::string> &library_list)
{
	auto bad_libs (GetListOfBadLibraries());
	vector<string> libraries(library_list);
	for (unsigned int i = 0; i < bad_libs.size(); i++) {
		vector<string>::iterator itr;
		while ((itr = find(libraries.begin(), libraries.end(), bad_libs[i])) != libraries.end()) {
			libraries.erase(itr);
		}
	}

	return libraries;
}

///
/// Return the list of classes that, if built in the particular given library, will cause bad things to be linked in!
///
vector<string> WrapperConfigurationInfo::BadClassLibraryCrossReference(const std::string &library_name, const std::vector<std::string> &class_list)
{
	///
	/// If there is nothign we know, then there are no classes to remove!
	///

	if (!IsLibraryLinkRestricted(library_name)) {
		return vector<string>();
	}

	///
	/// Ok -- do it carefully!
	///

	vector<string> allowed_libraries (AllowedLibraryLinks(library_name));
	vector<string> disallowed_libraries (DisallowedLibraryLinks(library_name));
	vector<string> result;
	for (unsigned int i = 0; i < class_list.size(); i++) {
	  if (!ROOTHelpers::IsClass(class_list[i])) {
		continue;
	  }
		RootClassInfo &class_info (RootClassInfoCollection::GetRootClassInfo(class_list[i]));
		if (class_info.LibraryName() != library_name) {
			bool bad_library = false;
			if (CheckAllowedLibraries(library_name)) {
				if (find(allowed_libraries.begin(), allowed_libraries.end(), class_info.LibraryName()) == allowed_libraries.end()) {
					bad_library = true;
				}
			}
			if (CheckDisallowedLibraries(library_name)) {
				if (find(disallowed_libraries.begin(), disallowed_libraries.end(), class_info.LibraryName()) != disallowed_libraries.end()) {
					bad_library = true;
				}
			}
			if (bad_library) {
				result.push_back(class_list[i]);
			}
		}
	}
	return result;
}

namespace {
	bool fileExists(const string &fname)
	{
		WIN32_FIND_DATAA FindFileData;
		HANDLE handle = FindFirstFileA(fname.c_str(), &FindFileData);
		bool found = handle != INVALID_HANDLE_VALUE;
		if (found)
		{
			//FindClose(&handle); this will crash
			FindClose(handle);
		}
		return found;
	}
}

///
/// Find and get all the .libs we should be loading!
///
vector<string> WrapperConfigurationInfo::GetAllRootDLLS()
{
	///
	/// The directory we will scan. Somehow this should be passed in or looked up with ROOTSYS. Later.
	///

	const int root_sys_buffer_size = 1024;
	char root_sys_buffer[root_sys_buffer_size];
	::GetEnvironmentVariableA ("ROOTSYS", root_sys_buffer, root_sys_buffer_size);
	string dir_to_scan (root_sys_buffer);
	dir_to_scan += "\\bin";
	string lib_dir = string(root_sys_buffer) + "\\lib";

	///
	/// Use the standard WIN32 find file code
	///

	WIN32_FIND_DATAA data_finder;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	
	string scan_wildcard = dir_to_scan + "\\*.dll";
	hFind = FindFirstFileA (scan_wildcard.c_str(), &data_finder);
	if (hFind == INVALID_HANDLE_VALUE) {
		cout << "Unable to search for librareis using " << scan_wildcard << "!" << endl;
		return vector<string> ();
	}

	// Strip out the dll, make sure a .lib exists (otherwise we can never link against the classes
	// in them), and add them to the list to return.

	vector<string> result;
	do {
		string fname = data_finder.cFileName;
		int end_of_lib = fname.find(".dll");
		auto libName(fname.substr(0, end_of_lib));

		if (fileExists(lib_dir + "\\" + libName + ".lib")) {
			result.push_back(libName);
		}
		else {
			cout << "Ignoring library " << libName << " because we can't find a .lib name" << endl;
		}
	} while (FindNextFileA(hFind, &data_finder) != 0);

	FindClose(hFind);

	return result;
}

///
/// Some methods have different definitions in CINT and ROOT. We have to kludge our way around this
/// as there is no way for us to really know what is going on here.
///
void WrapperConfigurationInfo::FixUpMethodArguments (const RootClassInfo *class_info, const string &method_name, vector<RootClassMethodArg> &args)
{
	///
	/// TTree::Process and anything below it use Process(void *selector...) when they really mean "Process(TSelector *...).
	///

	auto &inher = class_info->GetInheritedClassesDeep();
	if (method_name == "Process"
		&& (class_info->CPPQualifiedName() == "TTree"
		|| find(inher.begin(), inher.end(), "TTree") != inher.end())) {
			if (args[0].CPPTypeName() == "void*") {
				args[0].ResetType("TSelector*", "TSelector");
			}
	}
}

///
/// Fix up a return type for certian methods.
///
string WrapperConfigurationInfo::FixupMethodReturnType(const RootClassInfo *class_info, const RootClassMethod *method_info, const string &return_typename)
{
	if ((method_info->ClassOfMethodDefinition() == "TBuffer")
		&& (method_info->NETName() == "Buffer")
		&& return_typename == "char*")
	{
		return "char[]";
	}

	return return_typename;
}

///
/// Should we hid this method? Mostly we want to return yes to this, but once and a while there
/// is a bad method, and we need to reset it.
///
bool WrapperConfigurationInfo::MakeMethodHidden (const RootClassMethod &method)
{
	///
	/// TTree::Process with the void * should show through to down-level guys. For whatever
	/// reason they decided to omit it in the root dict file, so we need to put it
	/// back. This is a little artificial - anyone who inherrits from TTree is
	/// going to get this treatment.
	///

	if (method.CPPName() == "Process"
		&& method.ClassOfMethodDefinition() == "TTree"
		&& method.arguments()[0].CPPTypeName() == "TSelector*"
		) {
			string temp = method.arguments()[0].CPPTypeName();
		return false;
	}

	return true;
}

///
/// Sometimes there are property names that are bad in the sense that if we use them
/// in one class, the infect the interface, and then they will force us to use them down-stream.
/// this is killer b/c that also means that if a method has the prop name later on that will cause
/// a compiler explosion! We check for and crash during translation... but still...
///
/// This is an artifiact, btw, of the way that we do the translation: the base class defines everything it
/// does in an interface. After that you always have to implement that interface. So, something that works find in
/// A can cause a compiler error in B if B : public A!!
///
bool WrapperConfigurationInfo::CheckPropertyNameBad (const RootClassInfo *class_info, const std::string &property_name)
{
	if (property_name == "Selected") {
		vector<string> allcls = class_info->GetInheritedClassesDeep();
		allcls.push_back(class_info->CPPQualifiedName());
		if (find(allcls.begin(), allcls.end(), "TVirtualPad") != allcls.end()) {
			return true;
		}
	}

	if (property_name == "Cleanup") {
		vector<string> allcls = class_info->GetInheritedClassesDeep();
		allcls.push_back(class_info->CPPQualifiedName());
		if (find(allcls.begin(), allcls.end(), "TGFrame") != allcls.end()) {
			return true;
		}
	}

	if (property_name == "Select") {
		vector<string> allcls = class_info->GetInheritedClassesDeep();
		allcls.push_back(class_info->CPPQualifiedName());
		if (find(allcls.begin(), allcls.end(), "TSelectorDraw") != allcls.end()) {
			return true;
		}
	}

	if (property_name == "ColumnSize"
		|| property_name == "ColumnType"
		|| property_name == "Dimensions"
		|| property_name == "NumberOfColumns"
		|| property_name == "Offset"
		|| property_name == "RowClass"
		|| property_name == "TypeSize"
		) {
		vector<string> allcls = class_info->GetInheritedClassesDeep();
		allcls.push_back(class_info->CPPQualifiedName());
		if (find(allcls.begin(), allcls.end(), "TTable") != allcls.end()) {
			return true;
		}
	}

	return false;
}

///
/// Change where we locate the wrapper templates
///
void WrapperConfigurationInfo::SetTemplateLocation (const string &dir)
{
	_template_directory = dir;
}

///
/// Do a template lookup - return the complete path.
///
string WrapperConfigurationInfo::TemplatePath (const string &name)
{
	if (_template_directory.size() > 0) {
		return _template_directory + "\\" + name;
	}
	return name;
}

///
/// Return a list of global variables that should not be
/// sent out for the .NET world.
///
vector<string> WrapperConfigurationInfo::GetListOfBadGlobalVariables()
{
	vector<string> result;

	// There is no include file that will allow of this guy!
	result.push_back("G__cintv6");

	return result;
}
