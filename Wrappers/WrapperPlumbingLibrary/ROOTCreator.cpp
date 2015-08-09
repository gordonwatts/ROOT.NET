///
/// ROOTCreator
///
///  This class enables easily creating new ROOT objects dynamically.
///

#include "ROOTCreator.h"
#include "NetStringToConstCPP.hpp"
#include "DynamicHelpers.h"
#include "ROOTDotNet.h"
#include "ROOTDOTNETBaseTObject.hpp"
#include "ROOTDynamicException.h"

#include <TClass.h>
#include <TMethodCall.h>
#include <TApplication.h>
#include <TFunction.h>

#include <string>
using std::string;

#ifdef nullptr
#undef nullptr
#endif

namespace ROOTNET
{
	namespace Utility
	{
		ROOTCreator::ROOTCreator(void)
		{
		}

		///
		/// Create an object, but do it by name rather than using the DLR... when an object name contains template arguments, etc.
		/// It should be cast to a dynamic type by the calling guy.
		///
		System::Object^ ROOTCreator::CreateByName(System::String^ root_object_name, ...array<Object^> ^args)
		{
			//
			// Some basic init. Very quick if it has been done already
			//

			TApplication::CreateApplication();

			//
			// Check out the class that we are going to create. Even possible?
			//

			ROOTNET::Utility::NetStringToConstCPP class_name(root_object_name);
			auto c = TClass::GetClass(class_name);
			if (c == nullptr)
				throw gcnew ROOTDynamicException("ROOT does not know about the class");

			//
			// Next parse through the ctor arguments, and see if we can find the constructor method.
			//

			auto cn = c->GetTitle();
			auto caller = DynamicHelpers::GetFunctionCaller(c, (string) class_name, args, true);
			if (caller == nullptr)
				throw gcnew ROOTDynamicException("This ROOT Object does not have a ctor with these arguments");

			//
			// Now that we have a caller, call it!
			//

			auto obj = caller->CallCtor (c, args);
			if (obj == nullptr)
				return nullptr;

			return obj;
		}

		///
		/// Called to create a new ROOT object. The method name is the name of the object we want to create.
		/// The args are the ctor arguments.
		/// And the result will be the new objet!
		///
		/// Class must come from TObject for now...
		///
		bool ROOTCreator::TryInvokeMember (System::Dynamic::InvokeMemberBinder ^binder, array<Object^> ^args, Object^% result)
		{
			result = CreateByName(binder->Name, args);
			return result != nullptr;
		}

		///
		/// Return a global creator - this is just something to make it easy to deal with this stuff.
		///
		ROOTCreator^ ROOTCreator::ROOT::get ()
		{
			if (gCreator == nullptr)
				gCreator = gcnew ROOTCreator();
			return gCreator;
		}

		///
		/// Rest our held creator... mostly for testing.
		///
		void ROOTCreator::ResetROOTCreator()
		{
			gCreator = nullptr;
		}
	}
}
