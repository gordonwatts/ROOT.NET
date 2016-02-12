#pragma unmanaged
#include "CPPNetTypeMapper.hpp"
#include "TTSimpleType.hpp"
#include "TVectorArray.hpp"
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

	[TestMethod]
	void TestSystemReset()
	{
		CPPNetTypeMapper::Reset();
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
		CPPNetTypeMapper::Reset();
	  CPPNetTypeMapper::instance()->AddTypedefMapping("Float_t", "float");
	  CPPNetTypeMapper::instance()->AddTypedefMapping("Size_t", "float");

	  Assert::IsTrue(CPPNetTypeMapper::instance()->AreSameType("Size_t", "Float_t"), "The two should be the same type!");
	}

	[TestMethod]
	void VectorOfNormalType()
	{
		CPPNetTypeMapper::Reset();
		CPPNetTypeMapper::instance()->AddTypeMapper(new TVectorArray("double", false, false));
		CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("double", "double"));
		Assert::IsTrue(CPPNetTypeMapper::instance()->has_mapping("vector<double>"), "Should have vector type mapping");
	}

	[TestMethod]
	void VectorOfTypeDefType()
	{
		CPPNetTypeMapper::Reset();
		CPPNetTypeMapper::instance()->AddTypeMapper(new TVectorArray("double", false, false));
		CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("double", "double"));
		CPPNetTypeMapper::instance()->AddTypedefMapping("Double_t", "double");
		Assert::IsTrue(CPPNetTypeMapper::instance()->has_mapping("vector<Double_t>"), "Should have vector type mapping");
	}

	[TestMethod]
	void ConstVectorOfNormalType()
	{
		CPPNetTypeMapper::Reset();
		CPPNetTypeMapper::instance()->AddTypeMapper(new TVectorArray("double", false, true));
		CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("double", "double"));
		Assert::IsTrue(CPPNetTypeMapper::instance()->has_mapping("const vector<double>"), "Should have vector type mapping");
	}

	[TestMethod]
	void ConstVectorOfTypedefType()
	{
		CPPNetTypeMapper::Reset();
		CPPNetTypeMapper::instance()->AddTypeMapper(new TVectorArray("double", false, true));
		CPPNetTypeMapper::instance()->AddTypedefMapping("Double_t", "double");
		CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("double", "double"));
		Assert::IsTrue(CPPNetTypeMapper::instance()->has_mapping("const vector<Double_t>"), "Should have vector type mapping");
	}

	[TestMethod]
	void ConstVectorOfNormalTypeRef()
	{
		CPPNetTypeMapper::Reset();
		CPPNetTypeMapper::instance()->AddTypeMapper(new TVectorArray("double", true, true));
		CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("double", "double"));
		Assert::IsTrue(CPPNetTypeMapper::instance()->has_mapping("const vector<double>&"), "Should have vector type mapping");
	}

	[TestMethod]
	void ConstVectorOfTypedefTypeRef()
	{
		CPPNetTypeMapper::Reset();
		CPPNetTypeMapper::instance()->AddTypeMapper(new TVectorArray("double", true, true));
		CPPNetTypeMapper::instance()->AddTypedefMapping("Double_t", "double");
		CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("double", "double"));
		Assert::IsTrue(CPPNetTypeMapper::instance()->has_mapping("const vector<Double_t>&"), "Should have vector type mapping");
	}
  };
}
