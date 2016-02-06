#include "ClassTraversal.h"

#include "TSystem.h"

#include <vector>
#include <string>
#include <iostream>

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace	Microsoft::VisualStudio::TestTools::UnitTesting;

using namespace std;

namespace t_Translation
{
	/// Test the ClassTraversal utilities
	[TestClass]
	public ref class UnitTest
	{
	public: 
		[TestMethod]
		void InherritedClassesCanHaveNamespaces()
		{
			gSystem->Load("libTMVA");
			auto r = ClassTraversal::FindInheritedClasses("TMVA::Factory");
			Assert::AreEqual((size_t)1, (size_t)r.size());
		}

		[TestMethod]
		void RelatedClassesIncludeNamespaced()
		{
			gSystem->Load("libTMVA");
			vector<string> relatedClasses;
			relatedClasses.push_back("TMVA::Factory");
			vector<string> r = ClassTraversal::FindAllRelatedClasses(relatedClasses);

			for (auto i : r) {
				cout << i << endl;
			}

			Assert::AreEqual((size_t)3, r.size());
		}
	};
}
