#pragma unmanaged
#include "ClassTranslator.hpp"
#include "RootClassInfo.hpp"
#include "CPPNetTypeMapper.hpp"
#include "TTROOTClass.hpp"
#include "TTSimpleType.hpp"
#include "RootClassInfoCollection.hpp"
#include "TTCPPString.hpp"
#include "WrapperConfigurationInfo.hpp"

#include "TApplication.h"
#include "TSystem.h"
#pragma managed

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace	Microsoft::VisualStudio::TestTools::UnitTesting;
using namespace TestUtilities;

#include <string>
#include <vector>

using std::vector;
using std::string;

namespace t_Translation
{
	[TestClass]
	public ref class t_SimpleClassTranslator
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
		void TestCtorDTor()
		{
			ClassTranslator *trans = new ClassTranslator(".\\TestCtorDTor");
			delete trans;
		}

		[TestMethod]
		void TestTranslateTArray()
		{
			/// Translate TArray with nothing special setup...

			System::IO::Directory::CreateDirectory(".\\TestTranslateTArray");
			ClassTranslator *trans = new ClassTranslator(".\\TestTranslateTArray");
			trans->translate(RootClassInfo("TArray"));
			Assert::IsTrue(System::IO::File::Exists(".\\TestTranslateTArray\\NTArray.hpp"), "the header file is missing");
			Assert::IsTrue(System::IO::File::Exists(".\\TestTranslateTArray\\NTArray.cpp"), "the header file is missing");
			delete trans;
		};

		[TestMethod]
		void TestTranslateTArrayF()
		{
			/// Translate TArray with nothing special setup...

			System::IO::Directory::CreateDirectory(".\\TestTranslateTArrayF");
			ClassTranslator *trans = new ClassTranslator(".\\TestTranslateTArrayF");
			trans->translate(RootClassInfo("TArrayF"));
			Assert::IsTrue(System::IO::File::Exists(".\\TestTranslateTArrayF\\NTArrayF.hpp"), "the header file is missing");
			Assert::IsTrue(System::IO::File::Exists(".\\TestTranslateTArrayF\\NTArrayF.cpp"), "the source file is missing");
			delete trans;
		};

		[TestMethod]
		void TestTranslateTArrayFWithInher()
		{
			/// Translate TArray with nothing special setup...

			System::IO::Directory::CreateDirectory(".\\TestTranslateTArrayFWithInher");
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TArray", true));
			ClassTranslator *trans = new ClassTranslator(".\\TestTranslateTArrayFWithInher");
			trans->translate(RootClassInfo("TArrayF"));
			Assert::IsTrue(System::IO::File::Exists(".\\TestTranslateTArrayFWithInher\\NTArrayF.hpp"), "the header file is missing");
			Assert::IsTrue(System::IO::File::Exists(".\\TestTranslateTArrayFWithInher\\NTArrayF.cpp"), "the source file is missing");
			delete trans;
		};

		[TestMethod]
		void TestTranslateTArrayForMethods()
		{
			/// Translate TArray with nothing special setup...

			System::IO::Directory::CreateDirectory(".\\TestTranslateTArrayForMethods");
			ClassTranslator *trans = new ClassTranslator(".\\TestTranslateTArrayForMethods");
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTCPPString());
			CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");
			trans->translate(RootClassInfo("TArray"));
			Assert::IsTrue(FileUtilities::FindString(".\\TestTranslateTArrayForMethods\\NTArray.hpp", "DropObjectFromTables (void)"), "The method DropObjectFromTables was not there!");
			Assert::IsTrue(FileUtilities::FindString(".\\TestTranslateTArrayForMethods\\NTArray.hpp", "Set (int"), "The method Set (int n) was not there!");
			delete trans;
		};

		[TestMethod]
		void TestTranslateTArrayFForMethods()
		{
			/// Translate TArray with nothing special setup...

			System::IO::Directory::CreateDirectory(".\\TestTranslateTArrayFForMethods");
			ClassTranslator *trans = new ClassTranslator(".\\TestTranslateTArrayFForMethods");
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTCPPString());
			CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");
			trans->translate(RootClassInfo("TArrayF"));
			Assert::IsTrue(FileUtilities::FindString(".\\TestTranslateTArrayFForMethods\\NTArrayF.hpp", "Set (int"), "The method Set (int n) was not there!");
			delete trans;
		};

		void AddInRootClass (const string &classname)
		{
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass(classname, true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass(classname, true, "&"));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass(classname, true, "*"));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass(classname, true, "*&"));

			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass(classname, true, "", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass(classname, true, "&", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass(classname, true, "*", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass(classname, true, "*&", true));
		}

		[TestMethod]
		void TestTranslateTNamedInherrit()
		{
			/// ctor's from inherrited objects seem to be showing up in the build! Not good!

			System::IO::Directory::CreateDirectory(".\\TestTranslateTNamedInherrit");
			ClassTranslator *trans = new ClassTranslator(".\\TestTranslateTNamedInherrit");
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTCPPString());
			CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");
			AddInRootClass("TObject");
			trans->translate(RootClassInfo("TNamed"));
			Assert::IsFalse(FileUtilities::FindString(".\\TestTranslateTNamedInherrit\\NTNamed.hpp", "NTObject ();"), "The method NTObject was there - should not have been!");
			delete trans;
		};

		[TestMethod]
		void TestTranslateTH1SourceMethodRightClass()
		{
			/// The implementation of the method was getting the actual definiton class type rather
			/// than the class it was getting defined in! Ops!

			System::IO::Directory::CreateDirectory(".\\TestTranslateTH1SourceMethodRightClass");
			ClassTranslator *trans = new ClassTranslator(".\\TestTranslateTH1SourceMethodRightClass");
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("short", "short"));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTCPPString());
			CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");
			CPPNetTypeMapper::instance()->AddTypedefMapping("Short_t", "short");
			CPPNetTypeMapper::instance()->AddTypedefMapping("Width_t", "short");
			AddInRootClass("TObject");
			AddInRootClass("TNamed");
			AddInRootClass("TAttLine");
			trans->translate(RootClassInfo("TH1"));
			Assert::IsFalse(FileUtilities::FindString(".\\TestTranslateTH1SourceMethodRightClass\\NTH1.cpp", "NTAttLine::SetLineWidth"), "The method NTAttLine::SetLineWidth should not have been defined");
			Assert::IsTrue(FileUtilities::FindString(".\\TestTranslateTH1SourceMethodRightClass\\NTH1.cpp", "NTH1::SetLineWidth"), "The method NTH1::SetLineWidth should have been defined");
			delete trans;
		};

		[TestMethod]
		void TestInterfaceStaticMethod()
		{
			/// Make sure the static methods are correctly generated!

			System::IO::Directory::CreateDirectory(".\\TestInterfaceStaticMethod");
			ClassTranslator *trans = new ClassTranslator(".\\TestInterfaceStaticMethod");
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTCPPString());
			trans->translate(RootClassInfo("TArray"));
			Assert::IsFalse(FileUtilities::FindString(".\\TestInterfaceStaticMethod\\NTArray.cpp", "Class_name"), "The method Class_Name doesn't seem to be here!");
			Assert::IsTrue(FileUtilities::FindString(".\\TestInterfaceStaticMethod\\NTArray.cpp", "namespace Interface {"), "The interface static impl for Class_Name is missing");
			delete trans;
		};

		[TestMethod]
		void TestForPropertyRender()
		{
			/// Make sure MarkerSize is correctly emitted when we do this translation
			System::IO::Directory::CreateDirectory(".\\TestForPropertyRender");
			ClassTranslator *trans = new ClassTranslator(".\\TestForPropertyRender");
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("float", "float"));
			CPPNetTypeMapper::instance()->AddTypedefMapping("Float_t", "float");
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TNamed", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TAttLine", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TAttFill", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TAttText", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TAttMarker", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TObject", true));

			trans->translate(RootClassInfo("TVirtualX"));

			Assert::IsTrue(FileUtilities::FindString(".\\TestForPropertyRender\\NTVirtualX.hpp", "property float MarkerSize"), "The property MarkerSize isn't there!?!");
			Assert::IsTrue(FileUtilities::FindString(".\\TestForPropertyRender\\NTVirtualX.hpp", "property float TextAngle"), "The property TextAngle isn't there!?!");

			delete trans;
		}

		[TestMethod]
		void TestMultiRender()
		{
			/// Make sure MarkerSize is correctly emitted when we do this translation
			System::IO::Directory::CreateDirectory(".\\TestForPropertyRender");
			ClassTranslator *trans = new ClassTranslator(".\\TestForPropertyRender");
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("float", "float"));
			CPPNetTypeMapper::instance()->AddTypedefMapping("Float_t", "float");
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TNamed", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TAttLine", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TAttFill", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TAttText", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TAttMarker", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TObject", true));

			trans->translate(RootClassInfoCollection::GetRootClassInfo("TObject"));
			trans->translate(RootClassInfoCollection::GetRootClassInfo("TAttMarker"));
			trans->translate(RootClassInfoCollection::GetRootClassInfo("TAttText"));
			trans->translate(RootClassInfoCollection::GetRootClassInfo("TAttFill"));
			trans->translate(RootClassInfoCollection::GetRootClassInfo("TAttLine"));
			trans->translate(RootClassInfoCollection::GetRootClassInfo("TAttMarker"));
			trans->translate(RootClassInfoCollection::GetRootClassInfo("TVirtualX"));

			Assert::IsTrue(FileUtilities::FindString(".\\TestForPropertyRender\\NTVirtualX.hpp", "property float MarkerSize"), "The property MarkerSize isn't there!?!");
			Assert::IsTrue(FileUtilities::FindString(".\\TestForPropertyRender\\NTVirtualX.hpp", "property float TextAngle"), "The property TextAngle isn't there!?!");

			delete trans;
		}

		[TestMethod]
		void TestProtectedInher()
		{
			/// If a class has a protected inheritance, then the interface conversion method should be missing.

			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TPRegexp", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TPMERegexp", true));

			System::IO::Directory::CreateDirectory(".\\TestProtectedInher");
			ClassTranslator *trans = new ClassTranslator(".\\TestProtectedInher");
			trans->translate(RootClassInfoCollection::GetRootClassInfo("TPRegexp"));
			trans->translate(RootClassInfoCollection::GetRootClassInfo("TPMERegexp"));

			Assert::IsFalse(FileUtilities::FindString(".\\TestProtectedInher\\NTPMERegexp.hpp", "virtual ::TPRegexp *CPP_Instance_TPRegexp"), "No conversion to protected classes!?!");

			delete trans;	  
		}

		/// THis no longer works with 5.28 - the mehtods are missing now. :(
		[TestMethod]
		void TestRenamedProperties()
		{
			/// Make sure a property that has been renamed is coded up right in the cpp file

			RootClassInfo cinfo ("TConfidenceLevel");
			CPPNetTypeMapper::instance()->AddTypedefMapping("Double_t", "double");
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType ("double", "double"));
			vector<RootClassProperty> props (cinfo.GetProperties());

			const RootClassProperty *p;
			for (int i = 0; i < props.size(); i++) {
				if (props[i].name() == "A3sProbability") {
					p = &props[i];
					break;
				}
			}	

			Assert::IsTrue (p != 0, "Couldn't find the A3sPropbability property!");

			System::IO::Directory::CreateDirectory(".\\TestRenamedProperties");
			ClassTranslator *trans = new ClassTranslator(".\\TestRenamedProperties");
			trans->translate(RootClassInfoCollection::GetRootClassInfo("TConfidenceLevel"));

			Assert::IsFalse(FileUtilities::FindString(".\\TestRenamedProperties\\NTConfidenceLevel.cpp", "_instance->GetA3sProbability"), "Found reference to the modified name for a property - that won't build!");

			delete trans;
		}

#ifdef notyet
		// With the new way inherritance is being done, Rebin2D no longer appears in TProfile2D's files.
		// So we expect not to see this there.
		[TestMethod]
		void TestCovarGeneration()
		{
			WrapperConfigurationInfo::InitTypeTranslators();
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TH2", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TH2D", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TProfile", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TProfile2D", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TH2", true, "*"));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TH2D", true, "*"));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TProfile", true, "*"));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TProfile2D", true, "*"));

			/// The 2D binning in TProfile2D seems to not show up as a method to be properly
			/// "done". This is a case of covar return should be "shutting" it down, but doesn't
			/// seemed to have happened for whatever reason.

			RootClassInfo cinfo ("TProfile2D");

			System::IO::Directory::CreateDirectory(".\\TestCovarGeneration");
			ClassTranslator *trans = new ClassTranslator(".\\TestCovarGeneration");
			trans->translate(RootClassInfoCollection::GetRootClassInfo("TProfile2D"));

			Assert::IsTrue(FileUtilities::FindString(".\\TestCovarGeneration\\NTProfile2D.cpp", "Rebin2D"), "Did not find a reference to the Rebin2D method!?");

			delete trans;
		}
#endif
		[TestMethod]
		void TestProtectedHiddenProperty()
		{
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTCPPString());
			CPPNetTypeMapper::instance()->AddTypedefMapping("Option_t*", "const char*");
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TNamed", true));

			System::IO::Directory::CreateDirectory(".\\TestProtectedHiddenProperty");
			ClassTranslator *trans = new ClassTranslator(".\\TestProtectedHiddenProperty");
			trans->translate(RootClassInfoCollection::GetRootClassInfo("TSocket"));

			Assert::IsFalse(FileUtilities::FindString(".\\TestProtectedHiddenProperty\\NTSocket.cpp", "_instance->GetOption()"), "The protected GetOption is being referenced!");
		}

		[TestMethod]
		void TestForConstConversion()
		{
			/// A non-const protected method has the same signature as a public const method. Make sure
			/// the access occurs ok (since we loose the "const" moniker here in .NET).

			CPPNetTypeMapper::instance()->AddTypeMapper(new TTCPPString());
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TObject", true, "*"));

			RootClassInfo cinfo ("TObject");
			System::IO::Directory::CreateDirectory(".\\TestForConstConversion");
			ClassTranslator *trans = new ClassTranslator(".\\TestForConstConversion");
			trans->translate(RootClassInfoCollection::GetRootClassInfo("TObject"));

			Assert::IsTrue(FileUtilities::FindString(".\\TestForConstConversion\\NTObject.cpp", "(const TObject *)"), "Found not const conversions!");
			delete trans;
		}

		[TestMethod]
		void TestForBlockedStaticFunction()
		{
			/// If a function is on the "blocked" list then it shouldn't get translated!

			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("bool", "bool"));
			CPPNetTypeMapper::instance()->AddTypedefMapping("Bool_t", "bool");

			gSystem->Load("libRooFitCore");

			RootClassInfoCollection::SetBadMethods(WrapperConfigurationInfo::GetListOfBadMethods());

			System::IO::Directory::CreateDirectory(".\\TestForBlockedStaticFunction");
			ClassTranslator *trans = new ClassTranslator(".\\TestForBlockedStaticFunction");
			trans->translate(RootClassInfoCollection::GetRootClassInfo("RooAbsReal"));

			Assert::IsFalse(FileUtilities::FindString(".\\TestForBlockedStaticFunction\\NRooAbsReal.hpp", "evalErrorLoggingEnabled"), "evalErrorLoggingEnabled method shouldn't be listed!");

			delete trans;
		}
	};
}
