///
/// Code to help with memory leak free conversion from .NET strings to C++ strings.
///
#include "NetStringToConstCPP.hpp"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace ROOTNET
{
	namespace Utility
	{

		/// Be aggressive -- do the conversion to a string here.
		NetStringToConstCPP::NetStringToConstCPP(String ^str)
		{
			_trans = Marshal::StringToHGlobalAnsi(str);
		}

		/// Make sure we clean up all the memory we are going to use.
		NetStringToConstCPP::~NetStringToConstCPP()
		{
			Marshal::FreeHGlobal(_trans);
		}

		NetStringToConstCPP::operator  char *()
		{
			return static_cast< char*>(_trans.ToPointer());
		}
	}
}