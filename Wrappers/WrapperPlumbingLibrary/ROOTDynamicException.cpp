///
/// Methods to implement our custom exception when something goes wrong in the ROOT dynamic processing.
///

#include "ROOTDynamicException.h"


namespace ROOTNET
{
	namespace Utility
	{
		ROOTDynamicException::ROOTDynamicException(void)
		{
		}

		ROOTDynamicException::ROOTDynamicException(System::String ^message)
			: Exception(message)
		{
		}
	}
}
