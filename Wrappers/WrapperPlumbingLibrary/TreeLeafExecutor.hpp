///
/// Hold together all the code required to actually run and get a result
/// for a leaf. This is so that the DLR can run us inline automatically,
/// at high speed without having to try to sort out who we are.
///
/// This is a base class - what is actually done where will depend on what type is being
/// dealt with.
///
#pragma once

namespace ROOTNET
{
	namespace Utility
	{
		public ref class TreeLeafExecutor abstract
		{
		public:
			inline TreeLeafExecutor(void)
			{}

			inline virtual ~TreeLeafExecutor (void)
			{}

			virtual System::Object ^execute (unsigned long entry) = 0;
		};
	}
}

