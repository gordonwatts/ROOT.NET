#pragma once
///
/// Implementations of tree executors to help with reading the value of
/// a vector of simple types (like vector<int>) or a slightly more complex
/// type, vector<string>.
///
///
/// The design is a bit more complex.
///
///  1. The basic vector object.
///			This allows for size() and Length, as well as [] access. Finally,
///			when asked, it will return an Enumerable<Object^>.
///  2. The enumerator object
///			Manages iterating through the vector, no matter what the type it
///	 3.	Access object
///			This is the low level accessor that returns an item in the vector<> as
///			an Object and thus avoids having to deal with native/CLI issues in trying
///			to keep this stuff working correctly.
///

#include "TreeLeafExecutorRaw.h"
#include "TreeExecutorObjectConverters.h"

#include <vector>

namespace ROOTNET {
	namespace Utility {
		///
		/// Basic common interface for all the templated access to the objects. Allows basic info return,
		/// along with getting whatever is at an index as a System::Object. To be implemented by a template
		/// specialized super-class.
		///
		class TreeLeafExecutorVectorRaw : public TreeLeafExecutorRaw
		{
		public:
			TreeLeafExecutorVectorRaw (::TBranch *b)
				: TreeLeafExecutorRaw(b)
			{}
			virtual size_t size(void) = 0;
			virtual System::Object^ at (size_t index) = 0;
		};

		///
		/// An enumerator over the vector class. Since we are enumerating only Object's, there is no
		/// need for template specalization.
		///
		public ref class vector_accessor_enumerator : System::Collections::Generic::IEnumerator<System::Object^>
		{
		public:
			inline vector_accessor_enumerator (TreeLeafExecutorVectorRaw *exe)
				: _exe (exe), _index (-1)
			{
			}

			inline ~vector_accessor_enumerator (void)
			{}

			// Get the next value. Note that index can continue to increment...
			bool MoveNext (void)
			{
				_index++;
				return _index < _exe->size();
			}

			virtual bool MoveNext2() sealed = System::Collections::IEnumerator::MoveNext
			{ return MoveNext(); }

			// Throw or return whatever is at the current index as a Object^.
			property System::Object^ Current
			{
				virtual System::Object^ get() {
					if (_index < 0 || _index >= _exe->size())
						throw gcnew System::IndexOutOfRangeException();
					return _exe->at(_index);
				}
			}

			property Object^ Current2
			{
				virtual Object^ get() sealed = System::Collections::IEnumerator::Current::get
				{ return Current; }
			}

			void Reset()
			{ _index = -1; }

			virtual void Reset2 () sealed = System::Collections::IEnumerator::Reset
			{ Reset(); }
		private:
			TreeLeafExecutorVectorRaw *_exe;
			long _index;
		};

		/// The vector<> object. Returns Object^ for values, and holds onto a generic accessor. This implements
		/// things like size(), Length, [], and getting the enumerator.
		public ref class vector_accessor_enumerable : System::Collections::Generic::IEnumerable<System::Object^>
		{
		public:
			// Return the size of the vector as a vector::size() like call, or as a Length property.
			size_t size() { return _exe->size(); }
			property size_t Length {
				size_t get (void) { return _exe->size(); }
			}

			// Get the object at a particular index.
			property System::Object ^default[int] {
				virtual System::Object ^get (int index)
				{
					if (index < 0 || index > _exe->size())
						throw gcnew System::IndexOutOfRangeException();
					return _exe->at(index);
				}
		    }

			// Return an enumerator so we can use LINQ or foreach.
			virtual System::Collections::Generic::IEnumerator<System::Object^> ^GetEnumerator()
			{
				return gcnew vector_accessor_enumerator (_exe);
			}

			virtual System::Collections::IEnumerator ^GetEnumerator2() sealed = System::Collections::IEnumerable::GetEnumerator
			{
				 return GetEnumerator();
			}

		public protected:
			vector_accessor_enumerable (TreeLeafExecutorVectorRaw *exe)
				: _exe (exe)
			{}

		private:
			TreeLeafExecutorVectorRaw *_exe;
		};

		/// Specific implementation of the accessor when we have a vector<int> or other similar
		/// type that will just translate directly to .NET.
		template <class ValueType>
		class tle_vector_type_exe : public TreeLeafExecutorVectorRaw
		{
		public:
			tle_vector_type_exe (::TBranch *b)
				: TreeLeafExecutorVectorRaw(b), _value(nullptr)
			{
				b->SetAddress (&_value);
			}

			size_t size(void) { return _value->size(); }
			System::Object ^at (size_t index) { return ConvertToObject<ValueType>::Convert(_value->at(index)); }

		private:
			std::vector<ValueType> *_value;
		};

		/// Hold onto a vector type for some simple type (string, or int, etc.).
		ref class tle_vector_type : public TreeLeafExecutor
		{
		public:
			tle_vector_type (TreeLeafExecutorVectorRaw *exe)
				: _exe (exe)
			{
			}

			~tle_vector_type()
			{
				delete _exe;
			}

			///
			/// Read in the vector, and notify our accessor.
			///
			virtual System::Object ^execute (unsigned long entry) override
			{
				_exe->update(entry);
				return gcnew vector_accessor_enumerable(_exe);
			}

		private:
			TreeLeafExecutorVectorRaw *_exe;
		};
	}
}
