#include "TreeEntryEnumerator.hpp"

#ifdef nullptr
#undef nullptr
#endif

namespace ROOTNET
{
	namespace Utility
	{
			TreeEntryEnumerator::TreeEntryEnumerator(::TTree *treePtr)
				: _tree (treePtr), _current_entry (-1), _cached_ptr(nullptr), _manager (gcnew TreeManager(treePtr))
			{
			}

			///
			/// Move onto the next entry. Invalidate the current object
			/// so that we are looking at the next one.
			///
			bool TreeEntryEnumerator::MoveNext (void)
			{
				_current_entry++;
				_cached_ptr = nullptr;
				return _current_entry < _tree->GetEntries();
			}

			///
			/// User is actually going to look at this entry. We had better get the cached entry object!
			void TreeEntryEnumerator::create_cached_ptr (void)
			{
				_cached_ptr = gcnew TreeEntry (_manager, _current_entry);
			}
	}
}
