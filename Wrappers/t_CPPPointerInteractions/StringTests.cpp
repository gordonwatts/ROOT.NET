#include "stdafx.h"

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace	Microsoft::VisualStudio::TestTools::UnitTesting;
using namespace System::Runtime::InteropServices;

namespace t_CPPPointerInteractions
{
	[TestClass]
	public ref class UnitTest
	{
	public: 
		[TestMethod]
		void ConvertFromChar()
		{
			char a = 'a';
			System::Char ^neta = gcnew System::Char(a);
			System::Console::WriteLine(neta);
		}

		[TestMethodAttribute]
		void ConvertToChar()
		{
			System::Char neta = System::Char('b');

			//auto c = neta->ToChar(nullptr);
			System::String ^str = gcnew System::String(neta,1);
			auto trans = Marshal::StringToHGlobalAnsi(str->ToString());
			auto cstr = static_cast<const char*>(trans.ToPointer());
			char c = *cstr;
			Marshal::FreeHGlobal(trans);
		}

		[TestMethodAttribute]
		void ConvertToChar2()
		{
			System::Char neta = System::Char('b');

			//auto c = neta->ToChar(nullptr);
			char c = (char) neta;

			System::Console::WriteLine(System::Char(c));
		}
	};
}
