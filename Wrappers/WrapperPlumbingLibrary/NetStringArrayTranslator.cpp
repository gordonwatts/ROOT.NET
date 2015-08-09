///
/// Translate a string into something useful.
///
#include "NetStringArrayTranslator.hpp"

using namespace System;

namespace ROOTNET {
	namespace Utility {

		///
		/// Be aggresive and do the translation right away
		///
		NetStringArrayTranslator::NetStringArrayTranslator(array<String^> ^string_list)
		{
			_native_array = new char*[string_list->Length];
			_converter_list = gcnew array<NetStringToConstCPP^>(string_list->Length);
			for (int i = 0; i < string_list->Length; i++) {
				_converter_list[i] = gcnew NetStringToConstCPP (string_list[i]);
				_native_array[i] = _converter_list[i];
			}
		}

		///
		/// Something has changed. Make sure we get it back. Ugly, but as long as no one has messed
		/// with pointers inbetween we should be ok.
		///
		void NetStringArrayTranslator::Update(array<System::String ^> ^%string_list)
		{
			for (int i = 0; i < string_list->Length; i++) {
				string_list[i] = gcnew String(_native_array[i]);
			}
		}
	}
}