#pragma once
///
/// Kep track of the link between a C++ object and a .NET object.
/// Use a weak reference so the .NET garbage collector doesn't count
/// it against us -- and will still garbage collect an item...
///

namespace ROOTNET
{
	namespace Utility
	{
		ref class ROOTDOTNETBaseTObject;

		///
		/// Contains the info we track about a particular root object.
		public ref class ROOTObjectInfo
		{
		public:
			ROOTObjectInfo(::TObject *root_object_to_track, const ROOTNET::Utility::ROOTDOTNETBaseTObject ^net_obj);

			/// Return the .NET object. Returns null if the weak reference is no longer good.
			const ROOTNET::Utility::ROOTDOTNETBaseTObject ^GetNETObject(void);

		private:
			System::WeakReference ^_net_obj;
		};
	}
}
