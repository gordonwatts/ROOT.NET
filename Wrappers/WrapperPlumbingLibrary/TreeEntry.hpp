///
/// Master class that controls dynamic access to a ttree
///
#pragma once

#include "TreeManager.hpp"

namespace ROOTNET
{
	namespace Utility
	{
		///
		/// This class represents a single entry in the TTree. We come and go very often as
		/// we iterate through a tree, so we attempt to remain very small. We also manage the
		/// interaction between the tree manager and the actual items using the DLR.
		///
		public ref class TreeEntry : System::Dynamic::DynamicObject
		{
			// Make sure this stays only inside the assembly.
		public protected:
			inline TreeEntry(TreeManager ^mgr, unsigned long entry)
				: _mgr (mgr), _entry (entry)
			{}

		public:
			// The property override. This is the main way that a leaf is found in the TTree.
			virtual bool TryGetMember (System::Dynamic::GetMemberBinder ^binder, Object^% result) override;

		private:
			TreeManager ^_mgr;
			unsigned long _entry;
		};

	}
}
