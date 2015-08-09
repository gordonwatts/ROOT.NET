#include "stdafx.h"

#pragma unmanaged
#include "MethodInheritanceDriver.hpp"
#include "RootClassMethod.hpp"
#include "RootClassInfo.hpp"
#include "CPPNetTypeMapper.hpp"
#include "TTSimpleType.hpp"
#include "TTROOTClass.hpp"
#include "TPointerSimpleType.hpp"
#include "RootClassInfoCollection.hpp"

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

namespace t_MethodInherritance
{
  [TestClass]
  public ref class driver
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
    void CreateInheritancePolicyAndDriver()
    {
      ///
      /// Make sure basic create and delete work ok.
      ///

      MethodInheritanceDriver *mh = new MethodInheritanceDriver();
      delete mh;
    };

    [TestMethod]
    void CreateOnStack()
    {
      /// Make sure we can create the method guy on the stack!
      MethodInheritanceDriver mh;
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

    [TestMethod]
    void GetMethodsForBaseClass()
    {
      ///
      /// Get a list of the methods for a base class object -- TObject!
      /// This should require nothing special!
      ///

      MethodInheritanceDriver *mh = new MethodInheritanceDriver();
      vector<RootClassMethod> methods (mh->GetMethodsToImplement("TAttLine"));
      Assert::IsTrue (FindMethod("Modify", methods) != 0, "Could not find the expected method Modify");
      delete mh;
    }

    [TestMethod]
    void GetMethodsForSimpleSubclass()
    {
      ///
      /// Get methods for a class that has no hiding issues!
      ///

      MethodInheritanceDriver *mh = new MethodInheritanceDriver();
      vector<RootClassMethod> methods (mh->GetMethodsToImplement("TNamed"));
      Assert::IsTrue (methods.size() > 0, "No methods registered for TNamed!");
      Assert::IsTrue (FindMethod("Clear", methods) != 0, "Could not find the expected method Clear - implemented in TNamed");
      Assert::IsTrue (FindMethod("AppendPad", methods) != 0, "Could not find the expected method AppendPad - implemented in TObject");
      delete mh;
    }

    [TestMethod]
    void GetMethodForBogusClass()
    {
      ///
      /// Make sure we die!
      ///

      MethodInheritanceDriver *mh = new MethodInheritanceDriver();

      try {
        vector<RootClassMethod> methods(mh->GetMethodsToImplement("TH11232"));
	Assert::IsTrue(false, "Did not throw an exception trying to look up methods to bogus class!");
      } catch (...) {
	/// Do nothing -- we did ok here.
      }
      delete mh;
    }

    [TestMethod]
    void GetRootClassInfoObject()
    {
      ///
      /// Make sure we can get the ROOT class obj and make sure it is cached.
      ///

      MethodInheritanceDriver *mh = new MethodInheritanceDriver();
      const RootClassInfo &ri1 (mh->GetClassInfo("TObject"));
      const RootClassInfo &ri2 (mh->GetClassInfo("TObject"));
      
      Assert::IsTrue(ri1.CPPName() == "TObject", "Didn't get the TObject when we asked for it!");
      const RootClassInfo *ri1p = &ri1;
      const RootClassInfo *ri2p = &ri2;
      Assert::IsTrue(ri1p == ri2p, "The two TObject pointers that came back aren't the same!");

      delete mh;
    }

    /// Check to see if a particular method is implemented where we think it should be.
    void CheckMethodImplementation (const vector<RootClassMethod> &methods, const string &method_name, const string &impl_class)
    {
      const RootClassMethod *addbincontent = FindMethod(method_name, methods);
      string message ("Unable to find the " + method_name + " Method!");
      Assert::IsTrue(addbincontent != nullptr, gcnew String(message.c_str()));
      Assert::IsTrue(addbincontent->ClassOfMethodDefinition() == impl_class,
	gcnew String((method_name + " Implemented by the wrong class (" + addbincontent->ClassOfMethodDefinition() + " not TH1F)!").c_str()));
    }

    [TestMethod]
    void GetMethodImpelementation()
    {
      /// Make sure that the method we get back that is over-ridden is, indeed, overridden. - that is
      /// make sure the implemented object of the method is correct. Do it in a 3-rd order guy, and
      /// check to see if the middle guy is correct. This is because the level below is not done the
      /// same way as the top level when creating these guys.

      MethodInheritanceDriver *mh = new MethodInheritanceDriver();

      /// Declare the types required to get a hold of these guys
      CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");
      CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
      CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("Double_t", "double"));
      CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TBrowser", true));
      CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TBrowser*", true));
      CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("Option_t*", "double"));

      ///
      /// Look at TH1F - a fairly complex object with more than 3 layers of inherritance.
      ///
      vector<RootClassMethod> methods (mh->GetMethodsToImplement("TH1F"));

      /// AddBinContent is defined in TH1 but overriden in TH1F - test that the last guy is correctly represented!

      CheckMethodImplementation (methods, "AddBinContent", "TH1F");

      /// Finally, make sure somethign in TObject really is from TObject.
      CheckMethodImplementation (methods, "AppendPad", "TObject");

      /// Browse is implemented in TH1, but defined in TObject.
      CheckMethodImplementation (methods, "Browse", "TH1");

      delete mh;
    }
  };
}
