#pragma unmanaged
#include "CPPNetTypeMapper.hpp"
#include "TTSimpleType.hpp"
#pragma managed

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace	Microsoft::VisualStudio::TestTools::UnitTesting;

namespace t_CPPNetTypeMapper
{
  [TestClass]
  public ref class CPPNetTypeMapperInfrastructure
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

#ifdef notyet
	[TestMethod]
	void TestSystemReset()
	{
	  /// Make sure that all type info is lost after we do a reset!
	  Assert::IsFalse(CPPNetTypeMapper::instance()->has_mapping("int"), "Should not know how to map an int yet!");
	  CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
	  Assert::IsTrue(CPPNetTypeMapper::instance()->has_mapping("int"), "Should not know how to map an int yet!");

	  /// Reset things back to normal.
	  CPPNetTypeMapper::Reset();
	  Assert::IsFalse(CPPNetTypeMapper::instance()->has_mapping("int"), "Should not know how to map an int any longer!");
	};

	[TestMethod]
	void TestSimpleTypeComparison()
	{
	  CPPNetTypeMapper::instance()->AddTypedefMapping("Float_t", "float");
	  CPPNetTypeMapper::instance()->AddTypedefMapping("Size_t", "float");

	  Assert::IsTrue(CPPNetTypeMapper::instance()->AreSameType("Size_t", "Float_t"), "The two should be the same type!");
	}
#endif
  };
}
