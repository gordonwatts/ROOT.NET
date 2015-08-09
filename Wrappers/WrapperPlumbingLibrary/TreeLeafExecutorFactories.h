#pragma once

// Collecting all the tree leaf factories into one place

namespace ROOTNET {
	namespace Utility {

		ref class TreeLeafExecutorFactory;
		System::Collections::Generic::Dictionary<System::String^, TreeLeafExecutorFactory ^> ^CreateKnownLeafFactories();

	}
}
