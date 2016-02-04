#pragma unmanaged
#include "RootClassInfo.hpp"
#include "RootClassMethod.hpp"
#include "RootClassInfoCollection.hpp"

#include "CPPNetTypeMapper.hpp"
#include "TTSimpleType.hpp"
#include "TTROOTClass.hpp"
#include "WrapperConfigurationInfo.hpp"

#include "TApplication.h"
#include "TSystem.h"
#include "TROOT.h"

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

using std::string;
using std::vector;
using std::find;
using std::find_if;
using namespace std;
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
  public ref class MethodInheritance
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
	[TestCleanup]
	[TestInitialize]
	void CleanOutTypeSystem()
	{
	  CPPNetTypeMapper::Reset();
	  RootClassInfoCollection::Reset();
	}

	[TestMethod]
	void TestNonImplementedFetch()
	{
	  RootClassInfo *cinfo = new RootClassInfo ("TNamed");
	  const vector<RootClassMethod> protos (cinfo->GetPrototypesImplementedByThisClass(true));
	  Assert::IsTrue(find_if(protos.begin(),protos.end(),findmethodname("AppendPad")) == protos.end(), "AppendPad is not implemented by this class, but is present in its direct implemented proto list! BAD!");
	  Assert::IsFalse(find_if(protos.begin(),protos.end(),findmethodname("Clear")) == protos.end(), "The clean inherrited method Clear should be present list - it isn't!");

	  const vector<RootClassMethod> protosH (cinfo->GetAllPrototypesForThisClass(true));
	  Assert::IsFalse(find_if(protosH.begin(),protosH.end(),findmethodname("Clear")) == protosH.end(), "The clean inherrited method Clear should be present full inheritted list - it isn't!");
	  Assert::IsFalse(find_if(protosH.begin(),protosH.end(),findmethodname("AppendPad")) == protosH.end(), "AppendPad should be listed in the all prototypes type!");

	  delete cinfo;
	};

	[TestMethod]
	void TestForProperParent()
	{
	  RootClassInfo cinfo ("TH1");
	  const vector<RootClassMethod> protos (cinfo.GetAllPrototypesForThisClass(false));
	  Assert::IsFalse(find_if(protos.begin(),protos.end(),findmethodname("SetLineWidth")) == protos.end(), "SetLineWidth should be visible, but it isn't!");
	  Assert::IsTrue(find_if(protos.begin(),protos.end(),findmethodname("SetLineWidth"))->OwnerClass().CPPQualifiedName() == "TH1", "SetLineWidth's parent class should be TH1");
	}

	[TestMethod]
	void TestForProperParentAll()
	{
	  RootClassInfo cinfo("TArrayF");
	  const vector<RootClassMethod> protos (cinfo.GetAllPrototypesForThisClass(false));
	  for (int i = 0; i < protos.size(); i++) {
		Assert::IsTrue (protos[i].OwnerClass().CPPQualifiedName() == "TArrayF", "The parent class should be the same!");
	  }
	}

	[TestMethod]
	void TestForProperOperatorEQAll()
	{
	  RootClassInfo cinfo("TArrayF");
	  const vector<RootClassMethod> protos (cinfo.GetAllPrototypesForThisClass(false));
	  for (int i = 0; i < protos.size(); i++) {
		if (protos[i].CPPName() == "operator=") {
		  Assert::IsTrue (protos[i].OwnerClass().CPPQualifiedName() == "TArrayF", "The parent class should be the same for operator=!");
		}
	  }
	}

	/// Count now many methods have a particular name
	int CountMethods(const vector<RootClassMethod> &methods, const string &mname)
	{
	  int count = 0;
	  for (int i = 0; i < methods.size(); i++) {
		const RootClassMethod &m (methods[i]);
		if (m.CPPName() == mname) {
		  count++;
		}
	  }
	  return count;
	}

	// Filter out methods that match the name.
	vector<RootClassMethod> FilterMethods(const vector<RootClassMethod> &methods, const string &mname)
	{
	  vector<RootClassMethod> result;
	  for (int i = 0; i < methods.size(); i++) {
		const RootClassMethod &m (methods[i]);
		if (m.CPPName() == mname) {
			result.push_back(m);
		}
	  }
	  return result;
	}

	[TestMethod]
	void TestForOnlyOneMethod()
	{
	  RootClassInfo cinfo ("TH1");
	  const vector<RootClassMethod> protos (cinfo.GetAllPrototypesForThisClass(false));
	  Assert::IsTrue(CountMethods(protos, "SetLineWidth") == 1, "Expecting only one SetLineWidth method...");
	}

	[TestMethod]
	void TestForMethodFillCorrect()
	{
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TArray", true));
	  RootClassInfo *cinfo = new RootClassInfo("TArrayF");
	  const vector<RootClassMethod> protos (cinfo->GetPrototypesImplementedByThisClass(true));
	  for (vector<RootClassMethod>::const_iterator itr = protos.begin(); itr != protos.end(); itr++) {
		itr->IsGoodForInterface();
	  }
	}

	[TestMethod]
	void TestForCorrectTextMethods()
	{
	  WrapperConfigurationInfo::InitTypeTranslators();
	  RootClassInfo *cinfo = new RootClassInfo("TVirtualPS");
	  const vector<RootClassMethod> protos (cinfo->GetPrototypesImplementedByThisClass(true));
	  auto textmethods (FilterMethods(protos, "Text"));
	  for (size_t i = 0; i < textmethods.size(); i++) {
		  Assert::IsTrue(textmethods[i].IsVirtual(), "Text guys isn't virtual");
		  auto proto = textmethods[i].generate_method_header();
		  System::Console::WriteLine(gcnew System::String(proto.c_str()));
	  }
	  Assert::AreEqual((size_t) 1, textmethods.size(), "Expecting only one Text method...");
	}
  };
}
