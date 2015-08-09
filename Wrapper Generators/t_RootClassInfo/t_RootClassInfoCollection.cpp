#pragma unmanaged
#include "RootClassInfoCollection.hpp"
#include "ClassTranslator.hpp"
#include "TTSimpleType.hpp"
#include "RootClassMethod.hpp"
#include "WrapperConfigurationInfo.hpp"

#include "TApplication.h"
#include "TSystem.h"
#include "TROOT.h"

#pragma managed

#include <vector>
#include <set>

using std::vector;
using std::set;
using std::string;

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace	Microsoft::VisualStudio::TestTools::UnitTesting;
using namespace TestUtilities;

namespace t_RootClassInfo
{
	[TestClass]
	public ref class t_RootClassInfoCollection
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

#pragma region Additional test attributes
		//
		//You can use the following additional attributes as you write your tests:
		//
		//Use ClassInitialize to run code before running the first test in the class
		//[ClassInitialize()]
		//static void MyClassInitialize(TestContext^ testContext) {};
		//
		//Use ClassCleanup to run code after all tests in a class have run
		//[ClassCleanup()]
		//static void MyClassCleanup() {};
		//
		//Use TestInitialize to run code before running each test
		//[TestInitialize()]
		//void MyTestInitialize() {};
		//
		//Use TestCleanup to run code after each test has run
		//[TestCleanup()]
		//void MyTestCleanup() {};
		//
#pragma endregion 
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
		void TestBasicRecall()
		{
			RootClassInfo *c = RootClassInfoCollection::GetRootClassInfoPtr ("TH1F");
			Assert::IsTrue (c != 0, "Null pointer came back!");
			Assert::IsTrue (c->NETName() == std::string("NTH1F"), "Class name wasn't right");
		}

		[TestMethod]
		void TestSamePointer()
		{
			RootClassInfo *c1 = RootClassInfoCollection::GetRootClassInfoPtr ("TH1F");
			RootClassInfo *c2 = RootClassInfoCollection::GetRootClassInfoPtr ("TH1F");
			Assert::IsTrue(c1 == c2, "The pointers are not the same!");
		}

		[TestMethod]
		void TestBadMethodRemoval()
		{
			/// If a function is on the "blocked" list then it shouldn't get translated!
			/// And RootClassInfoCollection should handle this for us!

			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("bool", "bool"));
			CPPNetTypeMapper::instance()->AddTypedefMapping("Bool_t", "bool");

			gSystem->Load("libRooFitCore");

			RootClassInfo cinfo ("RooAbsReal");
			vector<RootClassMethod> protos (cinfo.GetAllPrototypesForThisClass(true));
			const RootClassMethod *m = FindMethod ("clearEvalErrorLog", protos);
			Assert::IsTrue(m != 0, "clearEvalErrorLog should have been in the list!");
			Assert::IsTrue(m->IsStatic(), "Method should be listed as static!");

			set<string> bad_methods;
			bad_methods.insert("clearEvalErrorLog");
			RootClassInfoCollection::SetBadMethods(bad_methods);

			RootClassInfo &cinfo1 (RootClassInfoCollection::GetRootClassInfo("RooAbsReal"));
			vector<RootClassMethod> protos1 (cinfo1.GetAllPrototypesForThisClass(true));
			m = FindMethod ("clearEvalErrorLog", protos1);
			Assert::IsTrue(m == 0, "clearEvalErrorLog should not have been in the list!");
		}
	};
}
