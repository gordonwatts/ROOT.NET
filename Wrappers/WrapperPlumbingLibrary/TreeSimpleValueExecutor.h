#pragma once
///
/// Code to implement an executor that will deal with simple leaf types - like int, float, etc.
///

#include "TreeLeafExecutorRaw.h"
#include "TreeExecutorObjectConverters.h"

namespace ROOTNET {
	namespace Utility {

		// Helper abstraction that all simple types use. This is to get around the problem
		// that we can't use a CLR generic parameter as an argumen to a C++ template because
		// of the different exe/compile time models.
		class TreeLeafExecutorSingleValueRaw : public TreeLeafExecutorRaw
		{
		public:
			TreeLeafExecutorSingleValueRaw (::TBranch *b)
				: TreeLeafExecutorRaw (b)
			{}

			virtual System::Object^ execute (unsigned long entry)
			{
				update(entry);
				return value();
			}
			virtual System::Object ^value (void) = 0;
		};

		// Helper that does the actual accessing of the leaf type. This is specialized to the
		// leaf type (int, float, etc.).
		template <class ValueType>
		class tle_simple_type_accessor : public TreeLeafExecutorSingleValueRaw
		{
		public:
			tle_simple_type_accessor (::TBranch *b)
				: TreeLeafExecutorSingleValueRaw (b)
			{
				b->SetAddress(&_value);
			}

			System::Object ^value (void) {return ConvertToObject<ValueType>::Convert(_value);}

		public:
			ValueType _value;
		};

		// The generic holder class for this specific type. This is what the dynamic
		// tree object will call in the end.
		generic <class ST>
		ref class tle_simple_type : public TreeLeafExecutor
		{
		public:
			// Init and take ownership of the executor
			tle_simple_type (TreeLeafExecutorSingleValueRaw *exe)
				: _exe(exe)
			{
			}

			~tle_simple_type()
			{
				delete _exe;
			}

			virtual System::Object ^execute (unsigned long entry) override
			{
				_exe->update(entry);
				return _exe->value();
			}

		private:
			TreeLeafExecutorSingleValueRaw *_exe;
		};
	}
}
