#include "ROOTObjectInfo.h"

namespace ROOTNET {
	namespace Utility {

		/// Save info for later fetch-back.
		ROOTObjectInfo::ROOTObjectInfo(::TObject *root_object_to_track, const ROOTDOTNETBaseTObject ^net_obj)
		{
			_net_obj = gcnew System::WeakReference((System::Object^) net_obj);
		}

		/// Return the .net object.
		const ROOTDOTNETBaseTObject ^ROOTObjectInfo::GetNETObject(void)
		{
			return (ROOTDOTNETBaseTObject^)_net_obj->Target;
		}

	}
}