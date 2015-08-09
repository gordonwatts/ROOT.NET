#include "stdafx.h"

#pragma unmanaged
#include "RootClassInfoCollection.hpp"

#include "TApplication.h"
#include "TSystem.h"
#pragma managed

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace	Microsoft::VisualStudio::TestTools::UnitTesting;

#include <vector>
#include <string>

using std::vector;
using std::string;

namespace t_MethodInherritance
{
  [TestClass]
  public ref class AmbigMethods
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
    void TestAmbigModify()
    {
      /// The method in TH1F::Modify is ambiguous where it comes. This is
	  /// because it is coming from several different sub-classes!
	  RootClassInfo cinfo ("TH1F");
	  const vector<RootClassMethod> methods (cinfo.GetAllPrototypesForThisClass(false));
	  const RootClassMethod *m = FindMethod("Modify", methods);
	  Assert::IsTrue(m != 0, "The Modify method must exist!");
	  Assert::IsTrue(m->IsAmbiguous(), "The Modify method should be marked ambiguous!");

	  const RootClassMethod *mOK = FindMethod("Class_Name", methods);
	  Assert::IsTrue(mOK != 0, "The Class_Name method should be there!");
	  Assert::IsFalse(mOK->IsAmbiguous(), "The Class_Name method is not ambiguous!!");
    };
  };
}
