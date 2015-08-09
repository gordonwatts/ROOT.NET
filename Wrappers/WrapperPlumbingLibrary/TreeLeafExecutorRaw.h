#pragma once
///
/// Base class for a tree leaf exeuctor. This contains some common
/// code for dealing with updates to the leaf using TBranch::GetEntry,
/// and attempts to optimize and prevent re-getting the data.
///
/// Each leaf type will implement a sub-class of this and use its functionality.
/// mixin is probably a better way to do this, but we choose inherritance for
/// whatever reason.
///

#include "TBranch.h"

namespace ROOTNET {
	namespace Utility {
		class TreeLeafExecutorRaw
		{
		public:
			// Track a branch assocaited with a particular item.
			TreeLeafExecutorRaw (::TBranch *b)
				: _last_entry(-1), _last_entry_good(false), _branch(b)
			{}

			// Make sure that this branch is no longer pointing to some
			// data when it goes away. This is more safe programming (with ROOT!!)
			// than anything else.
			inline virtual ~TreeLeafExecutorRaw (void)
			{
				_branch->SetAddress(nullptr);
			}

			// Called to update the branch to the current entry. Done only
			// if the entry has changed.
			inline void update (unsigned long entry)
			{
				if (_last_entry != entry)
					_last_entry_good = false;
				if (!_last_entry_good) {
					_branch->GetEntry(entry);
					_last_entry_good = true;
					_last_entry = entry;
				}
			}

		private:
			unsigned long _last_entry;
			bool _last_entry_good;
			::TBranch *_branch;
		};
	}
}
