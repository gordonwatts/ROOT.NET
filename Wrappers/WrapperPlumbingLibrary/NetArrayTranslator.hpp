///
/// NetArrayTranslator - a generic that helps with moving arrays back and forth.
///
#pragma once

namespace ROOTNET
{
	namespace Utility
	{

		template<class T>
		ref class NetArrayTranslator
		{
		public:
			NetArrayTranslator()
				: _array(0)
			{
			}
			NetArrayTranslator (array<const T> ^info)
			{
				/// Be aggressive in translating things right now.
				_array = new T[info->Length];
				for (int i = 0; i < info->Length;i++) {
					_array[i] = info[i];
				}
			}
			/// Clean up resources (no memory leaks!!).
			~NetArrayTranslator (void)
			{
				delete[] _array;
			}
			/// Going to be used, return the internal pointer...
			operator T*()
			{
				return _array;
			}
		protected:
			T *_array;
		};

		template<class T>
		ref class NetArrayTranslatorU : public NetArrayTranslator<T>
		{
		public:
			NetArrayTranslatorU (array<T> ^info)
			{
				/// Be aggressive in translating things right now.
				_array = new T[info->Length];
				for (int i = 0; i < info->Length;i++) {
					_array[i] = info[i];
				}
			}

			/// Could have been modified. Make sure we copy back any modifications.
			void Update(array<T> ^info)
			{
				for (int i = 0; i < info->Length;i++) {
					info[i] = _array[i];
				}
			}
		};
	}
}
