#pragma once

///
/// Support classes to support dealing with a vector of C++ classes
///

#ifdef nullptr
#undef nullptr
#endif

#include <vector>
#include <TObject.h>

using namespace System::Collections::Generic;

		class VOHolderBase {
		public:
			virtual ~VOHolderBase(void) {};
			virtual unsigned int size() = 0;
			virtual TObject *At(unsigned int index) = 0;
		};

		template<class T>
		class VOHolderObject : public VOHolderBase
		{
		public:
			inline VOHolderObject (std::vector<T> &objects)
				: _objects(objects)
			{
			}

			unsigned int size()
			{
				return _objects.size();
			}

			TObject *At (unsigned int index)
			{
				return &_objects[index];
			}

		private:
			std::vector<T> &_objects;
		};
namespace ROOTNET
{
	namespace Utility
	{

			///
			/// Enumerator that will move through this object vector.
			/// We are not robust against things changing under us!
			///
			template<class T>
			ref struct enumerator : IEnumerator<T>
			{
				enumerator( VOHolderBase *myArr )
				{
					_holder = myArr;
					_currentIndex = -1;
				}

				///
				/// Move us to point to the next guy, if we can. Otherwise, return false.
				///
				bool MoveNext()
				{
					if( _currentIndex < _holder->size() - 1 )
					{
						_currentIndex++;
						return true;
					}
					return false;
				}

				virtual bool MoveNext2() sealed = System::Collections::IEnumerator::MoveNext
				{
					return MoveNext();
				}

				///
				/// Return the current value
				///
				property T Current
				{
					virtual T get()
					{
						if (_currentIndex < 0) {
							return nullptr;
						}
						TObject *o = _holder->At(_currentIndex);
						//auto a = ROOTNET::Utility::ROOTObjectServices::GetBestObject<T> (o);
						return nullptr;
					}
				};

				// This is required as IEnumerator<T> also implements IEnumerator
				property Object^ Current2
				{
					virtual Object^ get() sealed = System::Collections::IEnumerator::Current::get
					{
						if (_currentIndex < 0) {
							return nullptr;
						}
						//return ROOTNET::Utility::ROOTObjectServices::GetBestObject<T> (_holder->At(_currentIndex));
						return nullptr;
					}
				};

				void Reset() {_currentIndex = -1;}
				virtual void Reset2() sealed = System::Collections::IEnumerator::Reset
				{
					Reset();
				}
				~enumerator() {delete _holder;}

				VOHolderBase *_holder;
				unsigned int _currentIndex;
			};
		template<class T>
		public ref class VectorObject : IEnumerable<T>
		{
		private:

		public:
			VectorObject (VOHolderBase *holder)
			{
				_holder = holder;
			}

			virtual IEnumerator<T>^ GetEnumerator()
			{
				return gcnew enumerator<T>(_holder);
			}

			virtual System::Collections::IEnumerator^ GetEnumerator2() sealed = System::Collections::IEnumerable::GetEnumerator
			{
				return gcnew enumerator<T>(_holder);
			}

		private:
			VOHolderBase *_holder;
		};
	}
}
