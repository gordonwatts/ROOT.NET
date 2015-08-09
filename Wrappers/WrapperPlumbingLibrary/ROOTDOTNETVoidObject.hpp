///
/// Represents an object known by ROOT's CINT dictionaries, but
/// not coming from TObject or any wrapper we know about. In short, this
/// guy is accessible via dynamic calls only!
///
#pragma once

#include "ROOTDOTNETBaseTObject.hpp"

namespace ROOTNET
{
	namespace Utility
	{
		ref class ROOTDOTNETVoidObject : ROOTDOTNETBaseTObject
		{
		public:
			ROOTDOTNETVoidObject(void);
			ROOTDOTNETVoidObject (void *instance, ::TClass *class_info);

			virtual void SetNull (void) override;

			virtual void DeleteHeldObject (void) override;

			virtual void DropObjectFromTables (void) override;

		public protected:
			/// So we can get at the pointer.
			virtual ::TObject *GetTObjectPointer(void) override;

			virtual void* GetVoidPointer (void) override
			{return _instance;}

			virtual ::TClass *GetAssociatedTClassInfo (void) override
			{ return _class; }

		private:
			void *_instance;
			::TClass *_class;
		};
	}
}

