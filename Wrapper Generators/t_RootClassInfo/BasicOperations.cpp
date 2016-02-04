#include "stdafx.h"

#pragma unmanaged
#include "RootClassInfo.hpp"
#include "RootClassMethod.hpp"
#include "RootClassInfoCollection.hpp"
#include "WrapperConfigurationInfo.hpp"

#include "CPPNetTypeMapper.hpp"
#include "TTSimpleType.hpp"
#include "TTROOTClass.hpp"
#include "TTROOTenum.hpp"

#include "TApplication.h"
#include "TSystem.h"
#include "TROOT.h"

#include <vector>
#include <string>
#include <algorithm>
#include <set>

using std::string;
using std::vector;
using std::find;
using std::set;
using std::find_if;
#pragma managed

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace	Microsoft::VisualStudio::TestTools::UnitTesting;

namespace {
	class findmethodname {
	public:
		findmethodname(const string n)
			: _name(n)
		{}
		bool operator() (const RootClassMethod &info)
		{
			return info.CPPName() == _name;
		}
	private:
		string _name;
	};
}

namespace t_RootClassInfo
{

	[TestClass]
	public ref class BasicOperations
	{
	private:
		TestContext^ testContextInstance;

	public: 
		/// <summary>
		///Gets or sets the test context which provides
		///information about and functionality for the current test run.
		///</summary>
		property Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ TestContext
		{
			Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ get()
			{
				return testContextInstance;
			}
			System::Void set(Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ value)
			{
				testContextInstance = value;
			}
		};

		[ClassInitialize]
		static void SetupClassTest(Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ testContext)
		{
			int nargs = 2;
			char *argv[2];
			argv[0] = "ROOTWrapperGenerator.exe";
			argv[1] = "-b";
			TApplication *app = new TApplication ("ROOTWrapperGenerator", &nargs, argv);
			gSystem->Load("libHist");
			gSystem->Load("libHist");
			gSystem->Load("libRooFit");
		}

		/// Make sure the type translators are empty!
		[TestCleanup]
		[TestInitialize]
		void CleanOutTypeSystem()
		{
			CPPNetTypeMapper::Reset();
			RootClassInfoCollection::Reset();
		}

		[TestMethod]
		void CheckBasicAccessors()
		{
			RootClassInfo *cinfo = new RootClassInfo ("TH1F");
			Assert::IsTrue (cinfo->CPPQualifiedName() == "TH1F", "CPP Name is incorrect");
			Assert::IsTrue (cinfo->NETQualifiedName() == "NTH1F", "NET Name is incorrect");
			delete cinfo;
		}

		[TestMethod]
		void CheckBasicInherritance()
		{
			RootClassInfo *cinfo = new RootClassInfo ("TH1F");
			vector<string> inherrited_classes (cinfo->GetDirectInheritedClasses());
			Assert::IsFalse (find(inherrited_classes.begin(), inherrited_classes.end(), "TNamed") != inherrited_classes.end(), "TNamed listed as direct inherrited");
			Assert::IsTrue (find(inherrited_classes.begin(), inherrited_classes.end(), "TH1") != inherrited_classes.end(), "TNamed is not listed as direct inherrited");
			delete cinfo;
		}

		[TestMethod]
		void GetInherritedPrototypes()
		{
			RootClassInfo *cinfo = new RootClassInfo ("TH1F");

			/// Check getting lots of protoypes.
			vector<RootClassMethod> protos (cinfo->GetPrototypesImplementedByThisClass());
			Assert::IsTrue (protos.size() > 0, "No inherrited prototypes were returned");
			Assert::IsFalse(find_if(protos.begin(),protos.end(),findmethodname("Add")) != protos.end(), "The inherrited method Add is present in the prototype list");
			Assert::IsTrue(find_if(protos.begin(),protos.end(),findmethodname("AddBinContent")) != protos.end(), "The inherrited method AddBinContent is not present in the prototype list");
			Assert::IsTrue(find_if(protos.begin(),protos.end(),findmethodname("Copy")) != protos.end(), "The inherrited method Copy is not present in the prototype list");

			/// Check getting the "cleaned" prototypes. Assume our type manager knows nothing at this point! ;-)
			vector<RootClassMethod> cprotos (cinfo->GetPrototypesImplementedByThisClass(true));
			Assert::IsTrue (cprotos.size() > 0, "No cleaned inherrited prototypes were returned");
			Assert::IsTrue(find_if(cprotos.begin(),cprotos.end(),findmethodname("AddBinContent")) == cprotos.end(), "The clean inherrited method AddBinContent should not be present in the prototype list");
			Assert::IsTrue(find_if(cprotos.begin(),cprotos.end(),findmethodname("Copy")) == cprotos.end(), "The clean inherrited method Copy is present in the prototype list - it isn't clean!");
			Assert::IsFalse(find_if(cprotos.begin(),cprotos.end(),findmethodname("Reset")) == cprotos.end(), "The clean inherrited method Reset should be present list - it isn't!");
			Assert::IsTrue(find_if(cprotos.begin(),cprotos.end(),findmethodname("IsA")) == cprotos.end(), "The clean inherrited method IsA should not be present list - it is!");

			delete cinfo;
		}

		[TestMethod]
		void GetBestInherritedForBaseClass()
		{
			RootClassInfo *cinfo = new RootClassInfo("TObject");
			auto iclass = cinfo->GetBestClassToInherrit();
			Assert::IsTrue(iclass.size() == 0, "TObject has no inherrited classes");
			delete cinfo;
		}

		[TestMethod]
		void GetBestInherritedForSingleInheritance()
		{
			RootClassInfo *cinfo = new RootClassInfo("TArrayF");
			auto iclass = cinfo->GetBestClassToInherrit();
			Assert::IsTrue(iclass == "TArray", "TArray should be what we come from!");
			delete cinfo;
		}

		[TestMethod]
		void GetBestInherritedForMultiInheritance1()
		{
			RootClassInfo *cinfo = new RootClassInfo("TH1F");
			auto iclass = cinfo->GetBestClassToInherrit();
			Assert::IsTrue(iclass == "TH1", "TH1 should be what we come from!");
			delete cinfo;
		}

		[TestMethod]
		void GetBestInherritedForMultiInheritance2()
		{
			RootClassInfo *cinfo = new RootClassInfo("TH1");
			auto iclass = cinfo->GetBestClassToInherrit();
			Assert::IsTrue(iclass == "TNamed", "TNamed should be what we come from!");
			delete cinfo;
		}

#ifdef notanymore
		// This GetDragType used to be implemented by both sub-classes, but ROOT has cleaned up their code. Nice! The result is this
		// test is no longer meanful. We'll have to do something else when a bug creeps in later.

		///
		/// GetDragType comes from two sub-classes. How do we pick either one??? Very hard to know! So, we have to "bail".
		///
		[TestMethod]
		void TestForIntHiding()
		{
			gSystem->Load("libGuiBld");

			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
			CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");
			CPPNetTypeMapper::instance()->AddTypedefMapping("Width_t", "int");
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTenum("EDragType"));

			auto cinfo = new RootClassInfo("TGuiBldDragManager");
			const vector<RootClassMethod> protos (cinfo->GetAllPrototypesForThisClass(true));

			///
			/// There should be only one GetDragType method
			///

			int count = 0;
			for (int i = 0; i < protos.size(); i++) {
				if (protos[i].NETName() == "GetDragType")
					count++;
			}
			Assert::AreEqual(1, count, "incorrect count of GetDragType");

			auto m = FindMethod("GetDragType", protos);
			Assert::IsTrue("EDragType" == m->return_type(), "Incorrect return type");

		}
#endif

		[TestMethod]
		void TypeMapLookups()
		{
			/// Make sure we can't translate TObject!
			Assert::IsFalse(CPPNetTypeMapper::instance()->has_mapping("TObject"), "Type mapper claims we can translate TObject - that is bad!");
		}

		const RootClassMethod *FindMethod(const string &mname, const vector<RootClassMethod> &protos)
		{
			for (int i = 0; i < protos.size(); i++) {
				if (protos[i].CPPName() == mname) {
					return &protos[i];
				}
			}
			return 0;
		}

		[TestMethod]
		void MethodTranslationAbility()
		{
			RootClassInfo *cinfo = new RootClassInfo ("TH1F");

			// Get a method
			vector<RootClassMethod> protos (cinfo->GetPrototypesImplementedByThisClass());

			// Find the Copy method...
			const RootClassMethod *method = FindMethod("Copy", protos);
			Assert::IsTrue (method != 0, "Could not find the Copy method in the list of TH1F methods!");
			Assert::IsFalse(method->can_be_translated(), "We shouldn't be able to translate the Copy method!");

			// The Class method, which is static and returns TClass, shouldn't translate.
			const RootClassMethod *classMethod = FindMethod("Class", protos);
			Assert::IsTrue (classMethod != 0, "Could not find the raw Class method");
			Assert::IsFalse (classMethod->can_be_translated(), "We shouldn't be able to translate the Class method!");

			// The IsA method, which is static and returns TClass, shouldn't translate.
			const RootClassMethod *isaMethod = FindMethod("IsA", protos);
			Assert::IsTrue (isaMethod != 0, "Could not find the raw IsA method");
			Assert::IsFalse (isaMethod->can_be_translated(), "We shouldn't be able to translate the IsA method!");

			delete cinfo;
		}

		[TestMethod]
		void FindResetMethodInTH1F()
		{
			/// Reset is an inherrited method, which should be implemented in both clean and unclean versions
			/// - it has a Option_t argument, but that is a default argument.
			RootClassInfo *cinfo = new RootClassInfo ("TH1F");
			vector<RootClassMethod> protos (cinfo->GetPrototypesImplementedByThisClass());
			Assert::IsTrue(FindMethod("Reset", protos) != 0, "Unable to find TH1F::Reset in complete list of methods");
			vector<RootClassMethod> cprotos (cinfo->GetPrototypesImplementedByThisClass(true));
			Assert::IsTrue(FindMethod("Reset", cprotos) != 0, "Unable to find TH1F::Reset in clean list of methods");
			delete cinfo;
		}

		[TestMethod]
		void TranslatorMeansFinding()
		{
			/// Make sure if we declare something to the type system that we can then translate an associated method!
			RootClassInfo *cinfo = new RootClassInfo ("TH1F");
			vector<RootClassMethod> protos (cinfo->GetPrototypesImplementedByThisClass());
			const RootClassMethod *method = FindMethod("AddBinContent", protos);
			Assert::IsTrue(method != nullptr, "Unable to find the AddBinContent method in TH1F");
			Assert::IsFalse(method->can_be_translated(), "Method is marked as being able to translate - should not be!");

			/// Declare the missing type - Int_t.
			CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType ("int", "int"));

			Assert::IsTrue(method->can_be_translated(), "Method should not be translatable, but isn't!");
		}

		[TestMethod]
		void TestIllegalMethodNames()
		{
			/// TRef has a weird operator= thing in there -- that really shouldn't be allowed since it returns void!
			RootClassInfo cinfo ("TRef");
			vector<RootClassMethod> protos (cinfo.GetAllPrototypesForThisClass(false));
			bool seen_one = false;
			for (int i = 0; i < protos.size(); i++) {
				const RootClassMethod &m (protos[i]);
				if (m.CPPName() == "operator=") {
					Assert::IsTrue(m.has_return_value() || !m.can_be_translated(), "This operator should always have a return type!");
					seen_one = true;
				}
			}
			Assert::IsTrue(seen_one, "There were no operator= guys in here! Yikes!");
		}

		[TestMethod]
		void TestBadMethodNames()
		{
			/// Make sure if we declare something to the type system that we can then translate an associated method!
			RootClassInfo cinfo ("TH1F");
			set<string> bad_methods;
			bad_methods.insert("Reset");
			cinfo.set_bad_method_names(bad_methods);

			vector<RootClassMethod> protos (cinfo.GetPrototypesImplementedByThisClass(false));
			const RootClassMethod *method = FindMethod("Reset", protos);
			Assert::IsTrue(method == 0, "The Reset should not have been translated");

			RootClassInfo cinfo1 ("TH1F");
			vector<RootClassMethod> protos1 (cinfo1.GetPrototypesImplementedByThisClass(false));
			method = FindMethod("Reset", protos1);
			Assert::IsTrue(method != 0, "The Reset should have been translated");
		}

		[TestMethod]
		void TestRooOperatorEqual()
		{
			/// make sure a Int_t operator Int_t() method is marked as having a return type (ops!).

			RootClassInfo cinfo ("RooCategoryProxy");
			vector<RootClassMethod> protos(cinfo.GetPrototypesImplementedByThisClass(false));

			const RootClassMethod *m = FindMethod("operator Int_t", protos);
			Assert::IsTrue (m != 0, "Could not find the operator method!");
			Assert::IsTrue (m->return_type() == "Int_t", "Return type is not correct!");
			Assert::IsTrue (m->has_return_value(), "This guy does have a return type, but is marked as not!");
		}

		[TestMethod]
		void TestSetLineWidthIsOK()
		{
			/// make sure a Int_t operator Int_t() method is marked as having a return type (ops!).

			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
			CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");
			CPPNetTypeMapper::instance()->AddTypedefMapping("Width_t", "int");
			RootClassInfo cinfo ("TH1F");
			vector<RootClassMethod> protos(cinfo.GetAllPrototypesForThisClass(false));

			const RootClassMethod *m = FindMethod("SetLineWidth", protos);
			Assert::IsTrue (m != 0, "Could not find the SetLineWidth method!");
			Assert::IsTrue (m->can_be_translated(), "Should be able to translate this method from all methods!");

			vector<RootClassMethod> protos1(cinfo.GetAllPrototypesForThisClass(true));

			m = FindMethod("SetLineWidth", protos1);
			Assert::IsTrue (m != 0, "Could not find the SetLineWidth method in the list of clean methods!");
			Assert::IsTrue (m->can_be_translated(), "Should be able to translate this method from clean methods!");
		}

		[TestMethod]
		void TestForTextAngleProperty()
		{
			/// error C3766: 'ROOTNET::NTVirtualX' must provide an implementation for the interface method
			/// 'float ROOTNET::Interface::NTAttMarker::MarkerSize::get(void)'

			RootClassInfo *cinfo = new RootClassInfo("TVirtualX");
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("float", "float"));
			CPPNetTypeMapper::instance()->AddTypedefMapping("Float_t", "float");

			vector<RootClassProperty> props (cinfo->GetProperties());
			bool found_it = false;
			for (int i = 0; i < props.size(); i++) {
				if (props[i].name() == "TextAngle") {
					found_it = true;
				}
			}
			Assert::IsTrue(found_it, "Did not find the TextAngle property!");
		}

		[TestMethod]
		void TestOrderPropFetch()
		{
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("float", "float"));
			CPPNetTypeMapper::instance()->AddTypedefMapping("Float_t", "float");

			/// Depending on who is using the RootClassInfo object, different answers are being returned. Seems to have to
			/// do with how the global state is getting stored. This is a regression test to prevent that. :-)
			/// First, the simple (and I think correct) way:

			RootClassInfo cgood ("TVirtualX");
			vector<RootClassProperty> pgood(cgood.GetProperties());
			vector<RootClassMethod> pgoodm (cgood.GetAllPrototypesForThisClass(true));

			/// Now the convoluted way

			RootClassInfo cbad("TVirtualX");
			cbad.set_bad_method_names(WrapperConfigurationInfo::GetListOfBadMethods());
			cbad.GetDirectInheritedClasses();
			cbad.include_filename();
			cbad.LibraryName();
			cbad.GetDirectInheritedClasses();
			cbad.GetReferencedClasses();
			cbad.GetReferencedEnums();
			cbad.GetAllPrototypesForThisClass(true);

			vector<RootClassProperty> pbad (cbad.GetProperties());
			vector<RootClassMethod> pbadm (cbad.GetAllPrototypesForThisClass(true));

			Assert::AreEqual(pgoodm.size(), pbadm.size(), "Should have same number of methods no matter what!");
			Assert::AreEqual(pgood.size(), pbad.size(), "Should have same number of properties no matter what!");
		}

		[TestMethod]
		void TestClassEnumEvents()
		{
			gSystem->Load("libRIO");
			RootClassInfo buffer ("TBuffer");
			auto enums = buffer.GetClassEnums();
			RootEnum *e = 0;
			for (int i = 0; i < enums.size(); i++) {
				auto name (enums[i].NameUnqualified());
				if (name.find("EMode") != name.npos) {
					e = &enums[i];
				}
			}
			Assert::IsTrue(e != 0, "Did not find an enum called EMode");

			Assert::IsTrue(e->NameQualified() == "TBuffer::EMode", "name wasn't as expected");
			Assert::IsTrue(e->NameUnqualified() == "EMode", "unqualified name is not right");
		}

		[TestMethod]
		void TestClassEnumEmptyDoneRight()
		{
			gSystem->Load("libRIO");
			RootClassInfo buffer ("TBuffer");
			auto enums = buffer.GetClassEnums();
			for (int i = 0; i < enums.size(); i++) {
				Assert::IsFalse(enums[i].NameQualified() == "TBuffer::", "No enum should be called TBuffer::!");
			}
		}

		[TestMethod]
		void TestEnumKnows()
		{
			auto enY = new RootEnum("TBuffer::EMode");
			auto enN = new RootEnum("EUser");

			Assert::IsTrue(enY->IsClassDefined(), "class qualifier should have been detected");
			Assert::IsFalse(enN->IsClassDefined(), "class qualifier missing, global guy");
		}

		[TestMethod]
		void TestRemovedInherritance()
		{
			/// Make sure that if we remove inherritance that they don't show up in the list of methods!

			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("float", "float"));
			CPPNetTypeMapper::instance()->AddTypedefMapping("Float_t", "float");

			RootClassInfo cgood ("TVirtualX");
			vector<RootClassProperty> pgood(cgood.GetProperties());
			vector<RootClassMethod> pgoodm (cgood.GetAllPrototypesForThisClass(true));

			/// Now truncate the inheritance hierarchy...

			RootClassInfo cbad("TVirtualX");
			vector<string> parents(cbad.GetDirectInheritedClasses());
			for (int i = 0; i < parents.size(); i++) {
				cbad.RemoveInheritedClass(parents[i]);
			}

			vector<RootClassProperty> pbad (cbad.GetProperties());
			vector<RootClassMethod> pbadm (cbad.GetAllPrototypesForThisClass(true));

			Assert::AreNotEqual(pgoodm.size(), pbadm.size(), "Should have same number of methods no matter what!");
			Assert::AreNotEqual(pgood.size(), pbad.size(), "Should have same number of properties no matter what!");
		}

		[TestMethod]
		void TestPropertyInheritanceWithDifferentTypes()
		{
			/// SetMarkerSize is defined in TAttMarker with an argument of "Size_t".
			/// It is overriden in TVirtualX with an argument of Float_t. In the end this
			/// doesn't matter because both go to float. However, we have to make sure the
			/// logic is there in our stuff to make this happen!

			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("float", "float"));
			CPPNetTypeMapper::instance()->AddTypedefMapping("Float_t", "float");
			CPPNetTypeMapper::instance()->AddTypedefMapping("Size_t", "float");

			RootClassInfo cgood ("TVirtualX");
			vector<RootClassProperty> pgood(cgood.GetProperties());

			bool found_it = false;
			for (int i = 0; i < pgood.size(); i++) {
				const RootClassProperty &p(pgood[i]);
				if (p.name() == "MarkerSize") {
					found_it = true;
				}
			}

			Assert::IsTrue(found_it, "The MarkerSize property should have been in there!");
		}

		[TestMethod]
		void TestBadArgumentNotOK()
		{
			/// TSortedList::Qsort is a static method that has a TObject** as an argument -- something we don't
			/// support in .NET wrappers. However, it is emitted in the source code, which is very odd. That method
			/// should never appear on anyone's list of valid methods after cleaning is done! This makes sure that
			/// doesn't happen! :-)

			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TObject", true));
			CPPNetTypeMapper::instance()->AddTypedefMapping ("Int_t", "int");

			RootClassInfo cinfo ("TSortedList");

			const vector<RootClassMethod> &methods(cinfo.GetAllPrototypesForThisClass(true));
			const vector<RootClassMethod> &methods_dirty (cinfo.GetAllPrototypesForThisClass(false));

			const RootClassMethod *m = FindMethod("QSort", methods_dirty);
			Assert::IsTrue (m != 0, "Unable to get QSort out of the dirty method list!");
			Assert::IsFalse (m->can_be_translated(), "QSort method can't be translated!");

			Assert::IsTrue(FindMethod("QSort", methods) == 0, "The QSort method should not be part of the list of good methods!");

		}

		[TestMethod]
		void TestSimilarMethodCount()
		{
			/// Make sure two methods with same NET args, but disimilar CPP args are treated as one. :-)!

			RootClassInfo cls ("TPMERegexp");

			vector<RootClassMethod> cls_protos(cls.GetAllPrototypesForThisClass(false));

			vector<RootClassMethod> sub_methods;
			for (int i = 0; i < cls_protos.size(); i++) {
				const RootClassMethod &m (cls_protos[i]);
				if (m.NETName() == "Substitute" && m.arguments().size() == 2) {
					sub_methods.push_back(m);
				}
			}
			Assert::AreEqual ((int) 1, (int) sub_methods.size(), "Expected only one method with 2 args called substitute!");
		}

		[TestMethod]
		void TestSimilarMethodCountClean()
		{
			/// Same test, but after cleaning! Since not being able to convert to NET names culd have bad consequences!

			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
			CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");

			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TString", true, "", false));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TString", true, "", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TString", true, "&", false));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TString", true, "&", true));

			RootClassInfo cls ("TPMERegexp");

			vector<RootClassMethod> cls_protos(cls.GetAllPrototypesForThisClass(true));

			vector<RootClassMethod> sub_methods;
			for (int i = 0; i < cls_protos.size(); i++) {
				const RootClassMethod &m (cls_protos[i]);
				if (m.NETName() == "Substitute" && m.arguments().size() == 2) {
					sub_methods.push_back(m);
				}
			}

			Assert::AreEqual ((int) 1, (int) sub_methods.size(), "Expected only one method with 2 args called substitute!");
		}

		[TestMethod]
		void TestPropNamedCorrectly()
		{
			RootClassInfo cinfo ("TConfidenceLevel");
			CPPNetTypeMapper::instance()->AddTypedefMapping("Double_t", "double");
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType ("double", "double"));
			vector<RootClassProperty> props (cinfo.GetProperties());

			for (int i = 0; i < props.size(); i++) {
				Assert::IsTrue (props[i].name().find_first_of ("0123456789") != 0, "Property name should not start with a number");
			}
		}

		[TestMethod]
		void TestPropNameSameAsMethodName()
		{
			gSystem->Load("libPhysics");

			/// TVector3 has a GetMag and a Mag. So we have to make sure the property isn't defined in this case!
			/// Part of the key was the 3 character long method name -- there was special logic for ignoring 3 charachter long
			/// methods (gecause you had to have GetX at least), and that is what hid this guy!
			///

			CPPNetTypeMapper::instance()->AddTypedefMapping("Double_t", "double");
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType ("double", "double"));

			RootClassInfo cinfo ("TVector3");

			vector<RootClassProperty> props (cinfo.GetProperties());
			vector<RootClassMethod> methods (cinfo.GetAllPrototypesForThisClass(true));

			Assert::IsTrue(FindMethod ("Mag", methods) != 0, "Did not find the Mag method - should have been there");

			for (int i = 0; i < props.size(); i++) {
				Assert::IsTrue(props[i].name() != "Mag", "Found a property with name Mag!");
			}
		}
	};
}
