///
/// Deals with returning a string array. The trick we have to deal with is... we have no idea how
/// many elements there are in the string array - so we have to wait for the user to tell us when
/// they use us.
///
#pragma once

namespace ROOTNET
{
	namespace Utility
	{
		public ref class CPPStringArray
		{
		public:
			inline CPPStringArray(char **string_list)
				: _string_list(string_list)
			{
			}

			/// The user thinks they know how big the array is. Convert everything at once!
			array<System::String^> ^as_array_sized(int siz);

			property System::String^ default[int] {
				System::String ^get(int index) {
					return gcnew System::String(_string_list[index]);
				}
			}
		private:
			char **_string_list;
		};
	}
}
