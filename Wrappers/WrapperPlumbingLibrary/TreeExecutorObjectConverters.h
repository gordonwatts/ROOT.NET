#pragma once
//
// traites classes to make converting from C++ to CLI easy(er) in the code
//

namespace ROOTNET {
	namespace Utility {

		// ConvertToObject::Convert<int>((int) 5) will return a boxed int.
		template<class ValueType>
		class ConvertToObject
		{
		public:
			static System::Object ^Convert(const ValueType &v) {return v;}
		};
		template<>
		class ConvertToObject<bool>
		{
		public:
			static System::Object ^Convert(const bool &b) { return gcnew System::Boolean(b); }
		};
		template<>
		class ConvertToObject<std::string>
		{
		public:
			static System::Object ^Convert(const std::string &s) { return gcnew System::String(s.c_str()); }
		};

	}
}
