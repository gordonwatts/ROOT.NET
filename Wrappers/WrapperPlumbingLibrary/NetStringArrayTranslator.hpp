#pragma once
///
/// User wants to send in a list of strings as an array. This isn't the
/// cheapest convresion, but we do our best. :-)
///
#include "NetStringToConstCPP.hpp"

namespace ROOTNET {
	namespace Utility {
		public ref class NetStringArrayTranslator
		{
		public:
			NetStringArrayTranslator(array<System::String^> ^string_list);

			/// Return the C++ type...
			inline operator  char **()
			{
				return _native_array;
			}

			/// Copy back the C++ array. A bit dangerous. but assume things haven't changed much.
			void Update (array<System::String^> ^% string_list);

		private:
			char **_native_array;
			array<NetStringToConstCPP^> ^_converter_list;
		};
	}
}
