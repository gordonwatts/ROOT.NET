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
		void NamespaceDotNetNameGood()
		{
			gSystem->Load("libTMVA");
			auto c = RootClassInfoCollection::GetRootClassInfo("TMVA::Factory");
			Assert::IsTrue("NTMVA::NFactory" == c.NETName(), "Didn't get TMVA::NFactory");
		}

		[TestMethod]
		void CPPNamespaceParsing()
		{
			gSystem->Load("libTMVA");
			auto histo = RootClassInfoCollection::GetRootClassInfo("TH1F");
			auto factory = RootClassInfoCollection::GetRootClassInfo("TMVA::Factory");
			auto source = RootClassInfoCollection::GetRootClassInfo("ROOT::TSchemaRule::TSource");

			auto nhisto = histo.CPPNameSpace();
			auto nfactory = factory.CPPNameSpace();
			auto nsource = source.CPPNameSpace();

			Assert::IsTrue(nhisto == "");
			Assert::IsTrue(nfactory == "TMVA");
			Assert::IsTrue(nsource == "ROOT");
		}

		[TestMethod]
		void NCPPNamespaceParsing()
		{
			gSystem->Load("libTMVA");
			auto histo = RootClassInfoCollection::GetRootClassInfo("TH1F");
			auto factory = RootClassInfoCollection::GetRootClassInfo("TMVA::Factory");
			auto source = RootClassInfoCollection::GetRootClassInfo("ROOT::TSchemaRule::TSource");

			auto nhisto = histo.NETNameSpace();
			auto nfactory = factory.NETNameSpace();
			auto nsource = source.NETNameSpace();

			Assert::IsTrue(nhisto == "");
			Assert::IsTrue(nfactory == "NTMVA");
			Assert::IsTrue(nsource == "NROOT");
		}

		[TestMethod]
		void CPPQualifiedName()
		{
			gSystem->Load("libTMVA");
			auto histo = RootClassInfoCollection::GetRootClassInfo("TH1F");
			auto factory = RootClassInfoCollection::GetRootClassInfo("TMVA::Factory");
			auto source = RootClassInfoCollection::GetRootClassInfo("ROOT::TSchemaRule::TSource");

			auto nhisto = histo.CPPQualifiedName();
			auto nfactory = factory.CPPQualifiedName();
			auto nsource = source.CPPQualifiedName();

			Assert::IsTrue(nhisto == "TH1F");
			Assert::IsTrue(nfactory == "TMVA::Factory");
			Assert::IsTrue(nsource == "ROOT::TSchemaRule::TSource");
		}

		[TestMethod]
		void NCPPQualifiedName()
		{
			gSystem->Load("libTMVA");
			auto histo = RootClassInfoCollection::GetRootClassInfo("TH1F");
			auto factory = RootClassInfoCollection::GetRootClassInfo("TMVA::Factory");
			auto source = RootClassInfoCollection::GetRootClassInfo("ROOT::TSchemaRule::TSource");

			auto nhisto = histo.NETQUalifiedName();
			auto nfactory = factory.NETQUalifiedName();
			auto nsource = source.NETQUalifiedName();

			Assert::IsTrue(nhisto == "NTH1F");
			Assert::IsTrue(nfactory == "NTMVA::NFactory");
			Assert::IsTrue(nsource == "NROOT::NTSchemaRule::NTSource");
		}

		[TestMethod]
		void CPPClassName()
		{
			gSystem->Load("libTMVA");
			auto histo = RootClassInfoCollection::GetRootClassInfo("TH1F");
			auto factory = RootClassInfoCollection::GetRootClassInfo("TMVA::Factory");
			auto source = RootClassInfoCollection::GetRootClassInfo("ROOT::TSchemaRule::TSource");

			auto nhisto = histo.CPPClassName();
			auto nfactory = factory.CPPClassName();
			auto nsource = source.CPPClassName();

			Assert::IsTrue(nhisto == "TH1F");
			Assert::IsTrue(nfactory == "Factory");
			Assert::IsTrue(nsource == "TSource");
		}

		[TestMethod]
		void NCPPClassName()
		{
			gSystem->Load("libTMVA");
			auto histo = RootClassInfoCollection::GetRootClassInfo("TH1F");
			auto factory = RootClassInfoCollection::GetRootClassInfo("TMVA::Factory");
			auto source = RootClassInfoCollection::GetRootClassInfo("ROOT::TSchemaRule::TSource");

			auto nhisto = histo.NETClassName();
			auto nfactory = factory.NETClassName();
			auto nsource = source.NETClassName();

			Assert::IsTrue(nhisto == "NTH1F");
			Assert::IsTrue(nfactory == "NFactory");
			Assert::IsTrue(nsource == "NTSource");
		}

		[TestMethod]
		void CPPQualifiedClassName()
		{
			gSystem->Load("libTMVA");
			auto histo = RootClassInfoCollection::GetRootClassInfo("TH1F");
			auto factory = RootClassInfoCollection::GetRootClassInfo("TMVA::Factory");
			auto source = RootClassInfoCollection::GetRootClassInfo("ROOT::TSchemaRule::TSource");

			auto nhisto = histo.CPPQualifiedClassName();
			auto nfactory = factory.CPPQualifiedClassName();
			auto nsource = source.CPPQualifiedClassName();

			Assert::IsTrue(nhisto == "TH1F");
			Assert::IsTrue(nfactory == "Factory");
			Assert::IsTrue(nsource == "TSchemaRule::TSource");
		}

		[TestMethod]
		void NCPPQualifiedClassName()
		{
			gSystem->Load("libTMVA");
			auto histo = RootClassInfoCollection::GetRootClassInfo("TH1F");
			auto factory = RootClassInfoCollection::GetRootClassInfo("TMVA::Factory");
			auto source = RootClassInfoCollection::GetRootClassInfo("ROOT::TSchemaRule::TSource");

			auto nhisto = histo.NETQualifiedClassName();
			auto nfactory = factory.NETQualifiedClassName();
			auto nsource = source.NETQualifiedClassName();

			Assert::IsTrue(nhisto == "NTH1F");
			Assert::IsTrue(nfactory == "NFactory");
			Assert::IsTrue(nsource == "NTSchemaRule::NTSource");
		}

		// Split into namespace recal and class naem recal... so we can deal with namespaces and nested classes.

		[TestMethod]
		void FilenameStemForNormalClass()
		{
			auto c = RootClassInfoCollection::GetRootClassInfo("TH1F");
			Assert::IsTrue(c.source_filename_stem() == "NTH1F", "Improper filename stem");
		}

		[TestMethod]
		void FilenameStemForTemplateClass()
		{
			gSystem->Load("libTMVA");
			auto c = RootClassInfoCollection::GetRootClassInfo("TMVA::Factory");
			Assert::IsTrue(c.source_filename_stem() == "NTMVA__NFactory", "Improper filename stem");
		}

		[TestMethod]
		void InterfaceNamespaceDotNetNameGood()
		{
			gSystem->Load("libTMVA");
			auto c = RootClassInfoCollection::GetRootClassInfo("TMVA::Factory");
			Assert::IsTrue("TMVA::Factory" == c.CPPName());
		}

		[TestMethod]
		void IncludeFileForNormalClass()
		{
			auto c = RootClassInfoCollection::GetRootClassInfo("TH1F");
			auto incFile = c.include_filename();
			Assert::IsTrue(incFile == "TH1.h", "Improper include name");
		}

		[TestMethod]
		void ClassNameOnlyForNormalClass()
		{
			auto c = RootClassInfoCollection::GetRootClassInfo("TH1F");
			auto incFile = c.NETName_ClassOnly();
			Assert::IsTrue(incFile == "NTH1F", "Improper include name");
		}

		[TestMethod]
		void ClassNameOnlyForNamespaceClass()
		{
			gSystem->Load("libTMVA");
			auto c = RootClassInfoCollection::GetRootClassInfo("TMVA::Factory");
			auto incFile = c.NETName_ClassOnly();
			Assert::IsTrue(incFile == "NFactory", "Improper include name");
		}

		[TestMethod]
		void CPPClassNameOnlyForNormalClass()
		{
			auto c = RootClassInfoCollection::GetRootClassInfo("TH1F");
			auto incFile = c.CPPName_ClassOnly();
			Assert::IsTrue(incFile == "TH1F", "Improper include name");
		}

		[TestMethod]
		void CPPClassNameOnlyForNamespaceClass()
		{
			gSystem->Load("libTMVA");
			auto c = RootClassInfoCollection::GetRootClassInfo("TMVA::Factory");
			auto incFile = c.CPPName_ClassOnly();
			Assert::IsTrue(incFile == "Factory", "Improper include name");
		}

		[TestMethod]
		void IncludeFileClassInSubDirectory()
		{
			gSystem->Load("libTMVA");
			auto c = RootClassInfoCollection::GetRootClassInfo("TMVA::Factory");
			auto incFile = c.include_filename();
			Assert::IsTrue(incFile == "TMVA/Factory.h", "Improper include name");
		}

		[TestMethod]
		void CPPNameUnqualifiedForNormalClass()
		{
			auto c = RootClassInfoCollection::GetRootClassInfo("TH1F");
			auto name = c.CPPNameUnqualified();
			Assert::IsTrue(name == "TH1F", "Improper include name");
		}

		[TestMethod]
		void CPPNameUnqualifiedForNamespaceClass()
		{
			gSystem->Load("libTMVA");
			auto c = RootClassInfoCollection::GetRootClassInfo("TMVA::Factory");
			auto name = c.CPPNameUnqualified();
			Assert::IsTrue(name == "TMVA__Factory", "Improper include name");
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
