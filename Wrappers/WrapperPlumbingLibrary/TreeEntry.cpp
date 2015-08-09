#include "TreeEntry.hpp"
#include "TreeLeafExecutor.hpp"

#ifdef nullptr
#undef nullptr
#endif

namespace ROOTNET
{
	namespace Utility
	{
		///
		/// See if this is a leaf name (or something).
		///
		bool TreeEntry::TryGetMember (System::Dynamic::GetMemberBinder ^binder, Object^% result)
		{
			auto executor = _mgr->get_executor (binder);
			if (executor == nullptr) {
				result = nullptr;
				return false;
			}

			result = executor->execute (_entry);
			return true;
		}

	}
}
