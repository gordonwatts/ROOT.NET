#pragma unmanaged
#include "ROOTHelpers.h"
#include "TApplication.h"
#include "TSystem.h"
#include "CPPNetTypeMapper.hpp"
#include "RootClassInfoCollection.hpp"
#pragma managed

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace	Microsoft::VisualStudio::TestTools::UnitTesting;

namespace t_Utilities
{
#ifdef notworkingyet

	[TestClass]
	public ref class t_ROOTHelpers
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
	  //gSystem->Load("libHist");
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
		void TestIsClassOnTemplate()
		{
		  Assert::IsTrue(ROOTHelpers::IsClass("basic_ostream<char,char_traits<char> >"), "basic_ostream<char,char_traits<char> > is a class!");
		};
	};
#endif
}
