#pragma once

#include <string>

namespace ROOTNET
{
	namespace Utility
	{
		///
		/// Why was the pointer set to null. This is a little helpful for trying to debug things.
		///
		public enum class ReasonPointerNullEnum {
			kNothingHappenedYet = 0, // The pointer shoul dnot be null yet
			kSetNullCalled = 1, // SetNull method was called
			kObjectDeleted = 2, // The object was deleted with DeleteHeldObject.
			kObjectNotDeleted = 3, // The object was deleted with DeleteHeldObject, but it isn't an object we are allowed to delete due to the generation!
			kObjectFinalized = 4, // Object finalizer was called - you should never see this!!
		};

		///
		/// The base-base-base-base object that is used for all objects that are wrapped by TObject.
		/// Actually, the main reason this is hear is so that we can cleanly factor the plumbing library code
		/// and the actual generated classes. :-)
		///
		/// Also includes simple Dynamic implementation. Note that the DLR will use any static implemenations first,
		/// and then call one of the Tryxxx methods that we override here.
		///
		public ref class ROOTDOTNETBaseTObject abstract : System::Dynamic::DynamicObject
		{
		public:
			ROOTDOTNETBaseTObject(void);
			/// Set the held object pointer to null, but don't delete it.
			virtual void SetNull (void) = 0;
			/// Delete the object and set the pointer to null.
			virtual void DeleteHeldObject (void) = 0;
			/// Drop the object from our internal tables. This is done just before it is to be deleted!
			virtual void DropObjectFromTables (void) = 0;

			/// Reset ownership to be not us so we don't delete it. Used when ROOT takes over ownership because of something we do.
			void SetNativePointerOwner(bool owner);

			/// Set why our pointer is null. Really shouldn't be called by anyone other than
			/// this library in order to not trample on the info.
			void SetNullReason (ReasonPointerNullEnum reason) { _whyNull = reason; }

			/// Dynamic implementions.
			virtual bool TryInvokeMember (System::Dynamic::InvokeMemberBinder ^binder, array<Object^> ^args, Object^% result) override;
			virtual bool TryGetMember (System::Dynamic::GetMemberBinder ^binder, Object^% result) override;
			virtual bool TrySetMember (System::Dynamic::SetMemberBinder ^binder, Object^ value) override;

			/// Destructors and finalizers. The finalizer will delete the ROOT memory.
			/// The dtor will call the finalizer (so it doesn't happen twice!). Probably no
			/// other language besides C++ will ever use the dtor.
			~ROOTDOTNETBaseTObject(void);
			!ROOTDOTNETBaseTObject(void);

		// public inside the assembly, protected outside the assembly... weird, but CLI C++ access rules.
		public protected:
			/// So we can get at the pointer.
			virtual ::TObject *GetTObjectPointer(void) = 0;

			/// Get the base void* pointer. Mostly this is just a call to above.
			virtual void* GetVoidPointer (void) = 0;

			/// Get the TClass for this guy. Usually used mostly by dynamic dudes.
			virtual ::TClass *GetAssociatedTClassInfo (void) = 0;

		protected:
			/// True if we are the owner and should delete the underlying C++ object
			/// if this guy gets garbage collected.
			bool _owner;

			/// Keep track of why our pointer was set to null. Mostly helpful is
			/// tracking down bombs and crashes.
			ReasonPointerNullEnum _whyNull;
		};
	}
}