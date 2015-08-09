#include "stdafx.h"

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

#include "TAttLine.h"
#include "TClass.h"

#ifdef nullptr
#undef nullptr
#endif

namespace t_CPPPointerInteractions
{
	///
	/// Test the pointer table, make sure it works correctly.
	///
	[TestClass]
	public ref class t_NonTObjectPointerTable
	{
	private:

	public: 
		[TestInitialize]
		void Setup()
		{
			ROOTNET::Utility::ROOTCreator::ResetROOTCreator();
		}

		///
		/// The object table should remember the objects that we are tracking. So make sure it does.
		///
		[TestMethod]
		void TestIsRemembered()
		{
			auto p = new TAttLine();
			auto cls = TClass::GetClass("TAttLine");

			Assert::IsTrue (p != nullptr, "TAttLine ptr");
			Assert::IsTrue (cls != nullptr, "TClass for TAttLine");

			auto wrapper1 = ROOTNET::Utility::ROOTObjectServices::GetBestNonTObjectObject (p, cls);
			auto wrapper2 = ROOTNET::Utility::ROOTObjectServices::GetBestNonTObjectObject (p, cls);

			// Warning: Get the wrapper lookup stuff working here!
			//Assert::AreEqual (wrapper1, wrapper2, "two wrappers equal");
		};

		[TestMethod]
		void TestDroppedFromTable()
		{
			auto p = new TAttLine();
			auto cls = TClass::GetClass("TAttLine");

			Assert::IsTrue (p != nullptr, "TAttLine ptr");
			Assert::IsTrue (cls != nullptr, "TClass for TAttLine");

			auto wrapper1 = ROOTNET::Utility::ROOTObjectServices::GetBestNonTObjectObject (p, cls);
			wrapper1->DropObjectFromTables();
			auto wrapper2 = ROOTNET::Utility::ROOTObjectServices::GetBestNonTObjectObject (p, cls);

			// Warning: Get the wrapper lookup stuff working here!
			//Assert::AreNotEqual (wrapper1, wrapper2, "two wrappers equal");
		}
	};
}
