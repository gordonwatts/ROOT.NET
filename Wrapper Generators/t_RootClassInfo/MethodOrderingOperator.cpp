#include "stdafx.h"

#pragma unmanaged
#include "RootClassInfo.hpp"
#include "RootClassMethod.hpp"
#include "RootClassInfoCollection.hpp"

#include "TTSimpleType.hpp"
#include "TTCPPString.hpp"
#include "TPointerSimpleType.hpp"
#include "TTROOTClass.hpp"
#pragma managed


#include <set>

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace	Microsoft::VisualStudio::TestTools::UnitTesting;
using std::string;
using std::vector;
using std::set;

///
/// Check to see if the < operator works correctly!
///
namespace t_RootClassInfo
{
  /// Our less operator for using in set...
  struct MOLess : public std::less<RootClassMethod>
  {
    bool operator() (const RootClassMethod &left, const RootClassMethod &right)
    {
	return left.is_less_than(right);
    }
  };

  [TestClass]
  public ref class MethodOrderingOperator
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
      return nullptr;
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
    void TestSetInsertion()
    {
      /// Make sure we can use the < operator in a set - which was
      /// one of the design goals!
      set<RootClassMethod, MOLess> smethods;
      Assert::IsTrue(smethods.size() == 0, "Size of set is not zero");

      RootClassInfo *cinfo = new RootClassInfo ("TNamed");
      vector<RootClassMethod> protos (cinfo->GetPrototypesImplementedByThisClass());
      const RootClassMethod *m = FindMethod("Copy", protos);
      Assert::IsTrue (m != 0, "Could not find the Copy method in the list of TH1F methods!");

      smethods.insert(*m);
      Assert::IsTrue(smethods.size() == 1, "Size of set is not one after one insertiong!");
      smethods.insert(*m);
      Assert::IsTrue(smethods.size() == 1, "Size of set is not one after inserting an object that is already in there!");

      delete cinfo;
    };

    [TestMethod]
    void TestLessThanSame()
    {
      /// Make sure the same method is never true!
      RootClassInfo *cinfo = new RootClassInfo ("TNamed");
      vector<RootClassMethod> protos (cinfo->GetPrototypesImplementedByThisClass());
      const RootClassMethod *mc = FindMethod("Copy", protos);
      Assert::IsTrue (mc != 0, "Could not find the Copy method!");

      Assert::IsFalse(mc->is_less_than(*mc), "We are not less than our selves!");
      delete cinfo;
    }

    [TestMethod]
    void TestLessThanOnName()
    {
      /// "Copy" < "Delete"
      RootClassInfo *cinfo = new RootClassInfo ("TNamed");
      vector<RootClassMethod> protos (cinfo->GetPrototypesImplementedByThisClass());
      const RootClassMethod *mc = FindMethod("Copy", protos);
      const RootClassMethod *md = FindMethod("FillBuffer", protos);

      Assert::IsFalse(mc == nullptr || md == nullptr, "Couldn't find either the copy or delete method!");
      Assert::IsTrue(mc->is_less_than(*md), "Copy method was not less than FilllBuffer method!");
      Assert::IsFalse(md->is_less_than(*mc), "FillBuffer method was not greater than Copy method!");
      delete cinfo;
    }

    [TestMethod]
    void TestLessThanOnArgs()
    {
      RootClassInfo *cinfo = new RootClassInfo ("TNamed");
      vector<RootClassMethod> protos (cinfo->GetPrototypesImplementedByThisClass());

      vector<RootClassMethod> all_clears (FindMethods("Clear", protos));
      Assert::IsTrue (all_clears.size() > 1);
      bool answer = all_clears[0].is_less_than(all_clears[1]);
      answer = (answer && all_clears[0].arguments().size() == 0)
	|| (!answer && all_clears[0].arguments().size() == 1);
      Assert::IsTrue (answer, "The method with zero arguments is not less than the method with 1 argument!");
      Assert::IsTrue (all_clears[0].is_less_than(all_clears[1]) == !all_clears[1].is_less_than(all_clears[0]), "The inverse is not the opposite!");
      delete cinfo;
    }

	[TestMethod]
	void TestLessThanOnArgTypes()
	{
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTCPPString());
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TPointerSimpleType("int"));

	  CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");
	  CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t*", "int*");

	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass ("TMethod", true));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass ("TObjArray", true));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass ("TMethod", true , "*"));
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass ("TObjArray", true, "*"));

	  RootClassInfo *cinfo = new RootClassInfo ("TObject");
	  vector<RootClassMethod> protos (cinfo->GetPrototypesImplementedByThisClass(true));

	  vector<RootClassMethod> all_clears (FindMethods("Execute", protos));

	  ///
	  /// Find the correct two!
	  ///

	  string temp (all_clears[0].generate_method_header());
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

	  Assert::IsTrue(all_executes[0].is_less_than(all_executes[1]) == !all_executes[1].is_less_than(all_executes[0]), "The inverse is not true!");
	  delete cinfo;      
	}
  };
}
