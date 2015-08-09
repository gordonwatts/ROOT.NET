///
/// Code to help with translating from C++ string list into .NET strings.
///
#include "CPPStringArray.hpp"

namespace ROOTNET
{
	namespace Utility
	{
		///
		/// The user is going to tell us exactly how big this guy is. Convert it all at once.
		/// This is really dangerous -- I hope thye know what they are doing. :-)
		///
		array<System::String^> ^CPPStringArray::as_array_sized(int siz)
		{
			array<System::String^> ^result = gcnew array<System::String^>(siz);
			for (int i = 0; i < siz; i++) {
				result[i] = gcnew System::String(_string_list[i]);
			}
			return result;
		}
	}
}