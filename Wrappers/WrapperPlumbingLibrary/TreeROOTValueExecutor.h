#pragma once
///
/// Implement a leaf executor that can deal with an arbitrary CINT-known object
/// (that is, the object must be known to TClass::GetClass).
///

#include "TBranch.h"
#include "TClass.h"

namespace ROOTNET {
	namespace Utility {
		/// Implementation of the leaf executor. Since we use the dynamic infrastructure of
		/// ROOT.NET to do all the heavy lifting there is very little for us to do!
		ref class tle_root_object : public TreeLeafExecutor
		{
		public:
			tle_root_object (::TBranch *b, ::TClass *classInfo)
				:_value (0), _branch(b), _last_entry (-1), _class_info(classInfo)
			{
				_value = new void*();
				_branch->SetAddress(_value);
				_is_tobject = classInfo->InheritsFrom("TObject");
			}

			~tle_root_object()
			{
				delete _value;
			}

			///
			/// Read in the vector, and notify our accessor.
			///
			virtual System::Object ^execute (unsigned long entry) override
			{
				if (_last_entry != entry) {
					_branch->GetEntry(entry);
					_last_entry = entry;
				}

				if (_is_tobject) {
					auto obj = ROOTObjectServices::GetBestObject<ROOTDOTNETBaseTObject^>(reinterpret_cast<::TObject*>(*_value));
					obj->SetNativePointerOwner (false); // TTree owns this!
					return obj;
				} else {
					auto obj = ROOTObjectServices::GetBestNonTObjectObject(*_value, _class_info);
					obj->SetNativePointerOwner (false); // TTree owns this!
					return obj;
				}
			}

		private:
			void **_value;
			::TBranch *_branch;
			::TClass *_class_info;
			unsigned long _last_entry;
			bool _is_tobject;
		};

	}
}
