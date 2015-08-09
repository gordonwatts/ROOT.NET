#include "stdafx.h"

#pragma unmanaged
#include "RootClassInfo.hpp"
#include "RootClassMethod.hpp"
#include "TTSimpleType.hpp"
#include "TTROOTClass.hpp"
#include "WrapperConfigurationInfo.hpp"
#include "RootClassInfoCollection.hpp"

#include "TApplication.h"
#include "TSystem.h"
#include "TROOT.h"

#pragma managed

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace	Microsoft::VisualStudio::TestTools::UnitTesting;

#include <vector>
#include <string>

using std::string;
using std::vector;

///
/// Test if two methods are equal in some way...
/// If this works then operator == can be defined for these
/// things and they can be used in STL.
///
namespace t_RootClassInfo
{
	[TestClass]
	public ref class MethodEquality
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

		[TestCleanup]
		[TestInitialize]
		void CleanOutTypeSystem()
		{
			CPPNetTypeMapper::Reset();
			RootClassInfoCollection::Reset();
		}

		/// Find a particular method in a list.
		const RootClassMethod *FindMethod(const string &mname, const vector<RootClassMethod> &protos)
		{
			for (int i = 0; i < protos.size(); i++) {
				if (protos[i].CPPName() == mname) {
					return &protos[i];
				}
			}
			return 0;
		}

		/// Find a particular method in a list.
		const vector<RootClassMethod> FindMethods(const string &mname, const vector<RootClassMethod> &protos)
		{
			vector<RootClassMethod> results;
			for (int i = 0; i < protos.size(); i++) {
				if (protos[i].CPPName() == mname) {
					results.push_back(protos[i]);
				}
			}
			return results;
		}

		[TestMethod]
		void TestReflexiveEquality()
		{
			/// Make sure the same method is, uh, the same! :-)
			WrapperConfigurationInfo::InitTypeTranslators();
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TObject", true, "&"));

			RootClassInfo *cinfo = new RootClassInfo ("TNamed");
			vector<RootClassMethod> protos (cinfo->GetPrototypesImplementedByThisClass());

			const RootClassMethod *m = FindMethod("Copy", protos);
			Assert::IsTrue (m != 0, "Could not find the Copy method in the list of TH1F methods!");
			const RootClassMethod &method(*m);
			Assert::IsTrue (method.is_equal(method), "Comparison of method to itself failed!");
		};

		[TestMethod]
		void TestObviousInequality()
		{
			/// Names are different! :-)
			RootClassInfo *cinfo = new RootClassInfo ("TNamed");
			vector<RootClassMethod> protos (cinfo->GetPrototypesImplementedByThisClass());

			const RootClassMethod *m1 = FindMethod("Copy", protos);
			const RootClassMethod *m2 = FindMethod("Compare", protos);
			Assert::IsTrue (m1 != 0, "Could not find the Copy method in the list of TH1F methods!");
			Assert::IsTrue (m2 != 0, "Could not find the Compare method in the list of TH1F methods!");
			Assert::IsFalse (m1->is_equal(*m2), "Comparison of method to a different one said they were the same!");
		}

		[TestMethod]
		void TestArgsDifferent()
		{
			/// Names are different! :-)
			RootClassInfo *cinfo = new RootClassInfo ("TNamed");
			vector<RootClassMethod> protos (cinfo->GetPrototypesImplementedByThisClass());

			vector<RootClassMethod> all_clears (FindMethods("Clear", protos));
			Assert::IsTrue (all_clears.size() > 1);
			Assert::IsFalse(all_clears[0].is_equal(all_clears[1]), "Methods that differ by arg list length should not be marked as different!");
			delete cinfo;
		}

		[TestMethod]
		void TestArgTypesDifferent()
		{
			WrapperConfigurationInfo::InitTypeTranslators();
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TMethod", true, "*"));
			RootClassInfo *cinfo = new RootClassInfo ("TObject");
			vector<RootClassMethod> protos (cinfo->GetPrototypesImplementedByThisClass());

			vector<RootClassMethod> all_clears (FindMethods("Execute", protos));

			///
			/// Find the correct two!
			///

			vector<RootClassMethod> all_executes;
			for (int i = 0; i < all_clears.size(); i++) {
				if (all_clears[i].arguments().size() == 3) {
					all_executes.push_back(all_clears[i]);
				}
			}

			Assert::IsTrue(all_executes.size() >= 2, "Could not find the proper Execute methods to do the compare!");

			///
			/// Test for equality!
			///

			Assert::IsFalse(all_executes[0].is_equal(all_executes[1]), "Methods that differ by arg list types should not be marked as different!");
			delete cinfo;      
		}

#ifdef notyet
		// We can't use this any longer b/c the names of the methods used below (for TVirtualDragManager)
		// changed.
		[TestMethod]
		void TestArgTypeReturnEquality()
		{
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
			CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");

			gSystem->Load("libGui");

			RootClassInfo cinfo1 ("TGFrame");
			RootClassInfo cinfo2 ("TVirtualDragManager");

			vector<RootClassMethod> methods1 (cinfo1.GetPrototypesImplementedByThisClass());
			vector<RootClassMethod> methods2 (cinfo2.GetPrototypesImplementedByThisClass());

			const RootClassMethod *m1 (FindMethod("GetDragType", methods1));
			const RootClassMethod *m2 (FindMethod("GetDragType", methods2));

			Assert::IsTrue (m1 != 0, "Could not find #1 method");
			Assert::IsTrue (m2 != 0, "Could not find #2 method");

			Assert::IsTrue(m1->is_equal(*m2), "Methods should be equal!");
			Assert::IsFalse (m1->is_equal(*m2, true), "Methods should not be equal when considering the return type!");
		}

		[TestMethod]
		void TestArgTypeReturnLessThan()
		{
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
			CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");

			gSystem->Load("libGui");

			RootClassInfo cinfo1 ("TGFrame");
			RootClassInfo cinfo2 ("TVirtualDragManager");

			vector<RootClassMethod> methods1 (cinfo1.GetPrototypesImplementedByThisClass());
			vector<RootClassMethod> methods2 (cinfo2.GetPrototypesImplementedByThisClass());

			const RootClassMethod *m1 (FindMethod("GetDragType", methods1));
			const RootClassMethod *m2 (FindMethod("GetDragType", methods2));

			Assert::IsTrue (m1 != 0, "Could not find #1 method");
			Assert::IsTrue (m2 != 0, "Could not find #2 method");

			Assert::IsTrue(m1->is_less_than(*m2) == m2->is_less_than(*m1), "The two methods should be equal");
			Assert::IsTrue(m1->is_less_than(*m2, true) != m2->is_less_than(*m1, true), "The two methods should not be equal when considering return type");
		}
#endif
	};
}
