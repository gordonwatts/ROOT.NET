//
// Helper functions for dealing with dynamic infrastructure and the DLR
//
#pragma once

#include "ROOTDOTNETBaseTObject.hpp"
#include <Rtypes.h>
#include <string>
#include <vector>

class TMethod;
class TObject;
namespace Cint {
	class G__CallFunc;
}

namespace ROOTNET
{
	namespace Utility
	{
		/// Abstract class to do the conversions we need done.
		class ROOTTypeConverter
		{
		public:
			virtual ~ROOTTypeConverter() {}

			// Return the C++ type of the argument
			virtual std::string GetArgType() const = 0;

			// Do a call, return the thing
			virtual bool Call (Cint::G__CallFunc *func, void* ptr, System::Object^% result) = 0;

			virtual void SetArg (System::Object^ arg, Cint::G__CallFunc *func) = 0;
		};

		class DynamicCaller
		{
		public:
			DynamicCaller (std::vector<ROOTTypeConverter*> &converters, ROOTTypeConverter* rtn_converter, ::TMethod *method);
			~DynamicCaller (void);

			bool IsValid() const;

			System::Object^ CallCtor(::TClass *ptr, array<System::Object^> ^args);
			bool Call (void *ptr, array<System::Object^> ^args, System::Object^% result);

		private:
			std::vector<ROOTTypeConverter*> _arg_converters;
			ROOTTypeConverter *_rtn_converter;
			::TMethod *_method;

			/// CINT info for accessing the method
			Cint::G__CallFunc* _methodCall;
			long _offset;

			// Helpers
			// Return a list of the arguments, comma seperated.
			std::string ArgList() const;
		};

		class DynamicHelpers
		{
		public:
			DynamicHelpers(void);

			// Given a list of arguments, generate an argument list.
			static std::vector<std::string> GeneratePrototype(array<System::Object^> ^args);

			static ::TClass *ExtractROOTClassInfoPtr (const std::string &tname);

			static std::string resolveTypedefs(const std::string &type);

			static DynamicCaller *GetFunctionCaller(::TClass *cls_info, const std::string &method_name, array<System::Object^> ^args, bool is_ctor = false);
		};
	}
}

