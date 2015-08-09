#pragma once
///
/// Contains conversion code to help with converting a .NET string into a CPP string. We do this because
/// we have to clean up -- so we use stack semantics to deal with this. :-)
///

namespace ROOTNET
{
	namespace Utility
	{
		public ref class NetStringToConstCPP
		{
		public:
			NetStringToConstCPP(System::String ^source_string);
			~NetStringToConstCPP(void);
			operator  char *();

		private:
			System::IntPtr _trans;
		};
	}
}