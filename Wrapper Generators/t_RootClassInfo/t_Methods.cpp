#include "stdafx.h"

#include "RootClassInfo.hpp"
#include "RootClassInfoCollection.hpp"
#include "RootClassMethod.hpp"
#include "CPPNetTypeMapper.hpp"
#include "TTSimpleType.hpp"
#include "TVectorArray.hpp"
#include "TPointerSimpleType.hpp"
#include "WrapperConfigurationInfo.hpp"
#include "TTROOTClass.hpp"

#include "TApplication.h"
#include "TSystem.h"

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace	Microsoft::VisualStudio::TestTools::UnitTesting;

#include <algorithm>

using std::string;
using std::vector;
using std::find_if;

namespace t_RootClassInfo
{
	[TestClass]
	public ref class t_Methods
	{
	public: 
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

		/// Find a method
		const RootClassMethod *FindMethod(const string &mname, const vector<RootClassMethod> &protos)
		{
			for (int i = 0; i < protos.size(); i++) {
				if (protos[i].CPPName() == mname) {
					return &protos[i];
				}
			}
			return 0;
		}

		const vector<const RootClassMethod*> FindMethods(const string &mname, const vector<RootClassMethod> &protos)
		{
			vector<const RootClassMethod*> results;
			for (int i = 0; i < protos.size(); i++) {
				if (protos[i].CPPName() == mname) {
					results.push_back(&protos[i]);
				}
			}
			return results;
		}

		[TestMethod]
		void TestMethodIsVirtual()
		{
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("double", "double"));
			CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");
			CPPNetTypeMapper::instance()->AddTypedefMapping("Double_t", "double");

			RootClassInfo *cinfo = new RootClassInfo ("TArray");
			Assert::IsFalse (cinfo == nullptr, "We should have this class");
			auto method = FindMethod("GetAt", cinfo->GetAllPrototypesForThisClass(true));
			Assert::IsFalse(method == nullptr, "We shoudl have a method");
			Assert::IsTrue(method->IsVirtual(), "Method should be marked virtual");
		}

		void InitEverything()
		{
			WrapperConfigurationInfo::InitTypeTranslators();
		}

		[TestMethod]
		void TestMethodIsOverride()
		{
			InitEverything();

			RootClassInfo *cinfo = new RootClassInfo ("TArrayF");
			auto methods = FindMethods("Set", cinfo->GetAllPrototypesForThisClass(true));
			Assert::IsTrue(methods.size() == 2, "Expected two different set methods");

			// Get the single arg and double arg method.
			int singleArgMethod = methods[0]->generate_normalized_method_header().find(",") == string::npos ? 0 : 1;
			int doubleArgMethod = singleArgMethod == 0 ? 1 : 0;

			// Single arg method is inheerited
			Assert::IsTrue(methods[singleArgMethod]->IsDefaultOverride(), "Single arg method is inherrited");
			Assert::IsFalse(methods[doubleArgMethod]->IsDefaultOverride(), "Double arg method is not inherrited");
		}

		[TestMethod]
		void TestMethodIsOverride2()
		{
			InitEverything();

			RootClassInfo *cinfo = new RootClassInfo ("TInterpreter");
			auto methods = FindMethods("Execute", cinfo->GetAllPrototypesForThisClass(true));
			Assert::IsTrue(methods.size() == 2, "Expected two versions of execute");

			auto m1name = methods[0]->generate_normalized_method_header();
			auto m2name = methods[1]->generate_normalized_method_header();

			bool isArrayOne = m1name.find("array") == m1name.npos;
			auto arrayOne = isArrayOne ? methods[1] : methods[0];
			auto nonArrayOne = isArrayOne ? methods[0] : methods[1];

			Assert::IsTrue(nonArrayOne->IsDefaultOverride(), "2 arg should be inherrited");
			Assert::IsTrue(arrayOne->IsDefaultOverride(), "3 arg should be inherrited");
		}

		[TestMethod]
		void TestMethodIsOverride3()
		{
			InitEverything();

			RootClassInfo *cinfo = new RootClassInfo ("RooBinning");
			auto methods = FindMethods("array", cinfo->GetAllPrototypesForThisClass(true));
			Assert::IsTrue(methods.size() == 1, "Expected two versions of execute");

			Assert::IsTrue(methods[0]->IsDefaultOverride(), "array method should be marked override!");
		}

		///
		/// Find a property from the full list of properties
		///
		const RootClassProperty *FindProperty (const string &pname, const vector<RootClassProperty> properties)
		{
			for (int i = 0; i < properties.size(); i++) {
				if (pname == properties[i].name())
					return &(properties[i]);
			}
			return nullptr;
		}

		[TestMethod]
		void TestPropertyIsNotStaticWhenBothPresent()
		{
			InitEverything();
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TObject", true));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTROOTClass("TObject", true, "*"));

			RootClassInfo *cinfo = new RootClassInfo ("TRef");
			auto propGet = FindProperty("Object", cinfo->GetProperties());
			Assert::IsFalse(propGet == nullptr, "Expected to find the object property");
			Assert::IsFalse(propGet->isStatic(), "Expected Object to be a local, not a static, property");
			delete cinfo;
		}

		[TestMethod]
		void TestPropertyStatic()
		{
			InitEverything();
			RootClassInfo *cinfo = new RootClassInfo("TObject");

			auto propStatic = FindProperty("ObjectStat", cinfo->GetProperties());
			Assert::IsFalse(propStatic == nullptr, "Could not find ObjectStat");
			Assert::IsTrue(propStatic->isStatic(), "ObjectStat is static");

			auto propLocal = FindProperty("Name", cinfo->GetProperties());
			Assert::IsFalse(propLocal == nullptr, "COuld not find Name");
			Assert::IsFalse(propLocal->isStatic(), "Name should be local property");

			delete cinfo;
		}
	};
}
