#include "stdafx.h"

#pragma unmanaged
#include "MethodInheritanceDriver.hpp"
#include "RootClassMethod.hpp"
#include "RootClassInfo.hpp"
#include "RootClassInfoCollection.hpp"
#include "CPPNetTypeMapper.hpp"
#include "TTSimpleType.hpp"
#include "TPointerSimpleType.hpp"
#include "TTRootClass.hpp"
#include "TTCPPString.hpp"

#include "TApplication.h"
#include "TSystem.h"

#include <vector>
#include <string>
using std::vector;
using std::string;
#pragma managed

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace	Microsoft::VisualStudio::TestTools::UnitTesting;

///
/// Test the method hiding logic. These are usually methods that
/// we have to keep around because they are defined in the interface,
/// but should never ever be called by the code itself because
/// in C++ they are hidden. In short -- there is an impedence mis-match
/// between Interface inherritance and C++ inherritance.
///
/// Here is how it works in C++. If you have a method name, "mA", and it is implemented in several different
/// arg lists, etc., in base class A. class B defines a new method mA -- this now hides all the stuff in mA.
///
namespace t_MethodInherritance
{
  [TestClass]
  public ref class HiddenMethods
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
      gSystem->Load("libRooFitCore");
    }

    /// Make sure the type translators are empty!
    [TestCleanup]
	[TestInitialize]
    void CleanOutTypeSystem()
    {
      CPPNetTypeMapper::Reset();
	  RootClassInfoCollection::Reset();
    }

    /// Helper function for lookup below.
    const RootClassMethod *FindMethod(const string &mname, const vector<RootClassMethod> &protos)
    {
      for (unsigned int i = 0; i < protos.size(); i++) {
	if (protos[i].CPPName() == mname) {
	  return &protos[i];
	}
      }
      return 0;
    }
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

    const RootClassMethod *FindMethodWithArg1Type (const string &arg1type, const vector<RootClassMethod> &methods)
    {
      for (int i = 0; i < methods.size(); i++) {
	const RootClassMethod &method (methods[i]);
	if (method.arguments().size() > 0) {
	  string temp = method.arguments()[0].CPPTypeName();
	  if (method.arguments()[0].CPPTypeName() == arg1type) {
	    return &method;
	  }
	}
      }
      return nullptr;
    }

	[TestMethod]
    void TestTH1FOperatorEqualsForArrays()
    {
      /// TH1F inherrits from lots and lots of stuff -- one of the things it gets is operator=
      /// from lots of parents. These get hidden - it doesn't make sense to set a TH1F object
      /// = to a Array and leave it at that. So the method should be marked as hidden!

      /// Declare a bunch of things as being translatable!
      AddInRootClass ("TArray");
      AddInRootClass ("TArrayF");
      AddInRootClass ("TAttFill");
      AddInRootClass ("TAttLine");
      AddInRootClass ("TAttMarker");
      AddInRootClass ("TNamed");
      AddInRootClass ("TObject");
      AddInRootClass ("TH1F");

      /// Now, get the methods.
      MethodInheritanceDriver *mh = new MethodInheritanceDriver();
      vector<RootClassMethod> methods (mh->GetMethodsToImplement("TH1F"));

      /// Find all the operator= guys
      const vector<RootClassMethod> equals (FindMethods("operator=", methods));
      Assert::IsTrue(equals.size() > 1, "One or less operator= found in TH1F! Not possible!");

      /// Great - now find a few of the unprotected ones...
      const RootClassMethod *m = FindMethodWithArg1Type ("const TArray&", methods);
      Assert::IsTrue(m != nullptr, "Unable to find operator=(TArray)");
      Assert::IsTrue(m->IsHidden(), "operator=(TArray) is not marked hidden!");

      /// Find one of teh NTH1F guys and make sure it is not marked hidden!
      m = FindMethodWithArg1Type ("const TH1F&", methods);
      Assert::IsTrue(m != nullptr, "Unable to find operator=(TH1F)");
      Assert::IsFalse(m->IsHidden(), "operator=(TH1F) is marked hidden!");
      
      delete mh;
    };

	[TestMethod]
    void TestForSetTitleHiding()
    {
	  /// TNamed has a SetTitle method that takes an optional argument. However, TH1, etc., has
	  /// SetTitle that hides the TNamed one. So it shouldn't show up in the TH1's arg list.
	  /// It does right now, however... :-)

      RootClassInfo *cinfo = new RootClassInfo ("TH1");
	  vector<RootClassMethod> protos (cinfo->GetAllPrototypesForThisClass(false));

	  int count (0);
	  for (int i = 0; i < protos.size(); i++) {
		const RootClassMethod &m (protos[i]);
		if (m.CPPName() == "SetTitle") {
		  count++;
		  if (m.arguments().size() == 1) {
			Assert::IsFalse(m.IsHidden(), "1 argument SetTitle should not be hidden!");
		  } else {
			Assert::IsTrue(m.IsHidden(), "0 argument SetTitle shoudl be hidden!");
		  }
		}
	  }

	  Assert::AreEqual (count, 2, "SetTitle must in there twice!");
      delete cinfo;
    }

	[TestMethod]
	void TestForHiddenPropertyVisitibility()
	{
	  /// error C3766: 'ROOTNET::NTVirtualX' must provide an implementation for the interface method
	  /// 'float ROOTNET::Interface::NTAttMarker::MarkerSize::get(void)'

	  RootClassInfo *cinfo = new RootClassInfo("TVirtualX");
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("float", "float"));
	  CPPNetTypeMapper::instance()->AddTypedefMapping("Float_t", "float");

	  vector<RootClassMethod> protos (cinfo->GetAllPrototypesForThisClass(false));

	  Assert::IsTrue(FindMethod("GetMarkerSize", protos) != 0, "Couldn't find GetMarkerSize!");
	  Assert::IsTrue(FindMethod("SetMarkerSize", protos) != 0, "Couldn't find SetMarkerSize!");

	  vector<RootClassProperty> props (cinfo->GetProperties());
	  bool found_it = false;
	  for (int i = 0; i < props.size(); i++) {
		if (props[i].name() == "MarkerSize") {
		  found_it = true;
		}
	  }
	  Assert::IsTrue(found_it, "Did not find the MarkerSize property!");
	}

	[TestMethod]
	void TestForProtectedMethodHiding()
	{
	  /// When a sub class has a public method Read, and a super class overrides it, but makes it
	  /// protected, it shouldn't show up in the method list. :-(

	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TBuffer", true));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTCPPString());

	  RootClassInfo info ("TBuffer");
	  vector<RootClassMethod> protos (info.GetAllPrototypesForThisClass(false));

	  const RootClassMethod *m = FindMethod("Read", protos);

	  Assert::IsTrue(m != 0, "The read method should be on of the methods we have to implement!");
	  Assert::IsTrue(m->IsHidden(), "The Read method should be hidden!");
	}
  };
}
