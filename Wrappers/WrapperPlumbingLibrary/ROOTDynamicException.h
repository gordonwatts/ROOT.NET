///
/// When we throw an exception from ROOT Dynamic, make it our own.
#pragma once


namespace ROOTNET
{
	namespace Utility
	{

		public ref class ROOTDynamicException : public System::Exception
		{
		public:
			ROOTDynamicException(void);
			ROOTDynamicException(System::String ^message);
		};

	}
}
