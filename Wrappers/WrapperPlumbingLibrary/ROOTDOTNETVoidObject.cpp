#include "ROOTDOTNETVoidObject.hpp"


#ifdef nullptr
#undef nullptr
#endif

namespace ROOTNET
{
	namespace Utility
	{
		///
		/// Default ctor - not really used for anything (I think).
		///
		ROOTDOTNETVoidObject::ROOTDOTNETVoidObject(void)
			: _instance (nullptr), _class (nullptr)
		{
		}

		///
		/// Unlike a TObject, where the class info is hard coded in, here we have
		/// to keep track of it!
		///
		ROOTDOTNETVoidObject::ROOTDOTNETVoidObject(void *instance, ::TClass *cinfo)
			: _instance (instance), _class (cinfo)
		{
		}

		///
		/// Something has happened - like the object has been dropped by ROOT behind our back. We kill it here
		/// so there is a "clear" error when someone tries to access it.
		///
		void ROOTDOTNETVoidObject::SetNull (void)
		{
			_instance = nullptr;
			_class = nullptr;
		}

		///
		/// Delete the held object. Really, no idea how to do this!
		///
		void ROOTDOTNETVoidObject::DeleteHeldObject (void)
		{
		}

		///
		/// Make sure no one knows about this object in our tables any longer!
		///
		void ROOTDOTNETVoidObject::DropObjectFromTables (void)
		{
		}

		///
		/// Get the TObject pointer. We can't, obviously, so we just
		/// beg off!
		///
		::TObject *ROOTDOTNETVoidObject::GetTObjectPointer(void)
		{
			return nullptr;
		}

	}
}
