#pragma unmanaged
#include "RootClassInfo.hpp"
#include "RootClassMethod.hpp"
#include "RootClassInfoCollection.hpp"
#include "WrapperConfigurationInfo.hpp"

#include "CPPNetTypeMapper.hpp"
#include "TTSimpleType.hpp"
#include "TTROOTClass.hpp"
#include "TTCPPString.hpp"

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

namespace t_RootClassInfo
{
  [TestClass]
  public ref class t_SpecialCases
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

	/// Make sure the type translators are empty!
	[TestCleanup]
	[TestInitialize]
	void CleanOutTypeSystem()
	{
	  CPPNetTypeMapper::Reset();
	  RootClassInfoCollection::Reset();
	}

	const RootClassMethod *FindMethod (const std::string &method_name, int n_args, const vector<RootClassMethod> &methods)
	{
	  for (int i = 0; i < methods.size(); i++) {
		const RootClassMethod &m(methods[i]);
		if (m.CPPName() == method_name && m.arguments().size() == n_args) {
		  return &m;
		}
	  }
	  return 0;
	}

	RootClassProperty *FindProperty (const string &pr_name, vector<RootClassProperty> &props)
	{
	  for (int i = 0; i < props.size(); i++) {
		if (props[i].name() == pr_name) {
		  return &props[i];
		}
	  }
	  return 0;
	}

	[TestMethod]
	void TestForRebin2D()
	{
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

	  RootClassInfo base_info ("TH2D");
	  RootClassInfo cinfo ("TProfile2D");

	  const RootClassMethod *base_m = FindMethod("Rebin2D", 0, base_info.GetAllPrototypesForThisClass(true));
	  const RootClassMethod *cinfo_m = FindMethod("Rebin2D", 0, cinfo.GetAllPrototypesForThisClass(true));

	  Assert::IsTrue(base_m != 0, "Could not find a clean Rebin2D method in the base class!");
	  Assert::IsFalse(base_m->IsCovarReturn(), "The Rebin2D on the base class should not be covarient");

	  Assert::IsTrue(cinfo_m != 0, "Could not find a clean Rebin2D method in the derived class!");
	  Assert::IsTrue(cinfo_m->IsCovarReturn(), "The derived class Rebin2D should be covarient!");

	  Assert::IsTrue (base_m->return_type() == cinfo_m->return_type(), "The returns of the two methods must be the same1");
	}

	[TestMethod]
	void TestCovarRtnIsGoodForEmitting()
	{
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

	  const RootClassMethod *cinfo_m = FindMethod("Rebin2D", 0, cinfo.GetAllPrototypesForThisClass(true));

	  Assert::IsTrue(cinfo_m != 0, "Could not find a clean Rebin2D method in the derived class!");
	  Assert::IsTrue(cinfo_m->IsCovarReturn(), "The derived class Rebin2D should be covarient!");
	  Assert::IsTrue(cinfo_m->IsGoodForClass(), "The derived class is not marked as good!");
	}

	[TestMethod]
	void TestProtectedPropertyMarkedHidden()
	{
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTCPPString());
	  CPPNetTypeMapper::instance()->AddTypedefMapping("Option_t*", "const char*");
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TNamed", true));

	  RootClassInfo &cinfo (RootClassInfoCollection::GetRootClassInfo("TSocket"));
	  vector<RootClassProperty> props(cinfo.GetProperties());

	  const RootClassProperty *p = FindProperty("Option", props);

	  Assert::IsFalse(p == 0, "Could not find the GetOption property!");
	  Assert::IsTrue(p->isGetter(), "It isn't a getter!?");
	  Assert::IsTrue(p->getter_method() != 0, "Getter is null!??");
	  Assert::IsTrue(p->getter_method()->IsHidden(), "Getter method should be marked as hidden!");
	}

	[TestMethod]
	void TestMethodHiddenByBadMethod()
	{
	  /// AccessPath is declared with a default argument in TSystem, but not in TNetSystem. The defualted
	  /// argument is not something we can currently translate. From C++'s point of view, the method should
	  /// be totally hidden! Ops! :(

	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTCPPString());
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TSystem", true));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TNetSystem", true));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("bool", "bool"));
	  CPPNetTypeMapper::instance()->AddTypedefMapping("Bool_t", "bool");

	  RootClassInfo cinfo ("TNetSystem");
	  vector<RootClassMethod> protos (cinfo.GetAllPrototypesForThisClass(true));

	  const RootClassMethod *m = FindMethod ("AccessPathName", 1, protos);
	  Assert::IsTrue(m != 0, "Could not find the access path method");
	  Assert::IsTrue(m->IsHidden(), "Method AccessPathName should be marked hidden!");
	}

	[TestMethod]
	void TestPropertyHiddenByMethod()
	{
	  /// TCanvas has a method called Selected which should hide the Selected property!
	  /// The problem is the Selected property is part of TPad (superclass), and the Selected
	  /// method is part of the TCanvas class. So, now we need to hide "methods" due to the
	  /// properties. Pehrpas we should prefix all properties with a letter combo to avoid
	  /// this.

	  gSystem->Load("libGpad");

	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TObject", true));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TObject", true, "*"));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TPad", true));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TVirtualPad", true));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TVirtualPad", true, "*"));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
	  CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");

	  RootClassInfo cinfo ("TCanvas");

	  vector<RootClassMethod> protos (cinfo.GetAllPrototypesForThisClass(true));
	  const RootClassMethod *m (FindMethod ("Selected", 3, protos));

	  vector<RootClassProperty> props (cinfo.GetProperties());

	  /// This test now trivially succeeds because we have renamed all properties with a prefix "PR".
	  /// If we ever find a method with a prefix PR, then we will be in trouble.

	  RootClassProperty *p = FindProperty ("Selected", props);
	  Assert::IsTrue ((m == 0 && p != 0) || (m != 0 && p == 0), "Selected property should not be listed or the Selected Method should be gone!");
	}

	[TestMethod]
	void TestPropertyInSuperClassButCovarRtnForLocal()
	{
	  /// TLeafL's SetMaximum takes a 64 bit integer. TLeaf takes a 32 bit integer. So the code
	  /// thinks there is no property. However, there is one in the interface for TLeaf, so there
	  /// has to be done in TLeafL.

	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("long", "long"));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("long long", "long long"));
	  CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");
	  CPPNetTypeMapper::instance()->AddTypedefMapping("Long64_t", "long long");
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TLeaf", true));

	  gSystem->Load("libTree");

	  RootClassInfo cinfo ("TLeafL");

	  vector<RootClassProperty> props (cinfo.GetProperties());
	  vector<RootClassMethod> methods (cinfo.GetAllPrototypesForThisClass(true));

	  const RootClassMethod *m = FindMethod ("GetMaximum", 0, methods);
	  Assert::IsTrue(m != 0, "GetMaximum was not found!");
	  m = FindMethod("SetMaximum", 1, methods);
	  Assert::IsTrue(m != 0, "SetMaximum was not found");

	  RootClassProperty *p = FindProperty ("Maximum", props);

	  Assert::IsTrue(p != 0, "Did not find the Maximum property!");

	}

#ifdef notanymore
	/// In ROOT 5.28 they have fixed this inconsitency, and so this test no longer is relavent.

	[TestMethod]
	void TestHiddenPropertyWithWeirdParameter()
	{
	  /// TGLOrthoCamera has a method "set" whose argument changes from base class to super class. The result
	  /// is that the property when translating TGLOrthoCamera needs to have the base property declared (due
	  /// to the usual interface semantics problem). This isn't happening, because it is getting hidden.

	  gSystem->Load("libRGL");

	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TObject", true));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TGLCamera", true));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TGLPaintDevice", true, "*"));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TGLRect", true, "&", true));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TGLRect", true, "&"));

	  RootClassInfo cinfo ("TGLOrthoCamera");

	  vector<RootClassProperty> props (cinfo.GetProperties());

	  RootClassProperty *p = FindProperty ("Viewport", props);

	  Assert::IsTrue (p != 0, "Could not find the Viewport property!");
	  Assert::IsTrue (p->isSetter(), "The Viewport property isn't a setter!");

	  vector<RootClassMethod> methods (cinfo.GetAllPrototypesForThisClass(true));
	  const RootClassMethod *m = FindMethod ("SetViewport", 1, methods);
	  Assert::IsTrue(m != 0, "Couldn't find the set method!");
	  Assert::IsTrue(m->arguments()[0].CPPTypeName() == "TGLPaintDevice*", "SetViewport is incorrect!");

	  /// Since hidden, setter should be "busted"
	  const RootClassMethod *ms = p->setter_method();
	  Assert::IsTrue (ms->IsHidden(), "Method isn't hidden!");
	}
#endif

	[TestMethod]
	void TestForProtectedAndPublicMethodConst()
	{
	  /// A non-const protected method has the same signature as a public const method. Make sure
	  /// the access occurs ok (since we loose the "const" moniker here in .NET).

	  gSystem->Load("libRoo");

	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
	  CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("double", "double"));
	  CPPNetTypeMapper::instance()->AddTypedefMapping("Double_t", "double");
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTCPPString());


	  RootClassInfo cinfo ("RooGrid");
	  vector<RootClassMethod> methods (cinfo.GetAllPrototypesForThisClass(true));
	  const RootClassMethod *m = FindMethod ("coord", 2, methods);
	  Assert::IsTrue (m != 0, "Couldn't find the method!");

	  /// Make sure it will be marked as being "const" here!

	  Assert::IsTrue (m->IsConst(), "Method is not marked as const!");

	  /// Second, make sure this funky boy is not marked as const!
	  RootClassInfo cinfo2 ("TVirtualX");
	  vector<RootClassMethod> protos2 (cinfo2.GetAllPrototypesForThisClass(true));
	  m = FindMethod("DisplayName", 0, protos2);
	  Assert::IsTrue (m != 0, "Could not find the DisplayName method");
	  Assert::IsFalse(m->IsConst(), "DisplayName is not a const method");

	}

	[TestMethod]
	void TestHiddenGetPropMethodGetsLostInSubclass()
	{
	  /// TGuiBldDragManager has no defintion of GetDragType. However, both of its sub-classes do, with
	  /// different returns. Worse, one of its sub classes' definiton is not translatable yet. And some
	  /// how the method is missing from the translated version of TBuiBldDragmanager. This makes sure that
	  /// doesn't happen again.

	  gSystem->Load("libGuiBuild");

	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
	  CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");

	  RootClassInfo cinfo ("TGuiBldDragManager");

	  vector<RootClassMethod> methods(cinfo.GetAllPrototypesForThisClass(true));
	  const RootClassMethod *m = FindMethod("GetDragType", 0, methods);
	  Assert::IsTrue(m != 0, "Could not find GetDragType method!");

	  vector<RootClassProperty> props (cinfo.GetProperties());
	  RootClassProperty *p = FindProperty ("DragType", props);
	  Assert::IsTrue (p != 0, "Could not find the Drag Property!");
	  Assert::IsTrue (p->isGetter(), "Property should be a getter!");
	}
  };
}
