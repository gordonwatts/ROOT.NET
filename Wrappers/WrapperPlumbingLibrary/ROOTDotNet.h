// ROOTDotNet.h
// Some services for use by everyone else. Very low level.
//
#pragma once

#include <string>

namespace ROOTNET
{
	namespace Utility
	{
		ref class ROOTDOTNETBaseTObject;

		public ref class ROOTObjectServices
		{
		public:
			static void DoIt () {}
			// Return the best wrapper we can find for an object that has TObject in its inherritance list.
			generic<class T> where T: ref class
				static T GetBestObject (const ::TObject *obj);

			// Return the best wrapper we can find for an object that doesn't have TObject in its inherritance list.
			static ROOTDOTNETBaseTObject ^ GetBestNonTObjectObject (const void *obj, ::TClass *class_info);

		};
	}
}