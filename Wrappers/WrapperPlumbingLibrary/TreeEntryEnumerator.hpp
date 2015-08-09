///
/// Enumerator class that returns an tree iterator that moves through the tree.
///
#pragma once

#include "TreeEntry.hpp"

#include "TTree.h"

#ifdef nullptr
#undef nullptr
#endif

namespace ROOTNET
{
	namespace Utility
	{
		///
		/// This is the main enumerator for a Tree. Its responsibility is actually quite simple, in that it
		/// keeps track of all the various data. It will correctly handle the symantics of jumping around 
		/// in entries, but that just isn't a very efficient thing!
		/// This is an opinionated run-through:
		///
		///  - All branches are disabled upon first fetch of the enumerator! (but only there, so you can change it
		///    if you wish by getting the enumerator and then using it).
		///
		public ref class TreeEntryEnumerator : System::Collections::Generic::IEnumerator<ROOTNET::Utility::TreeEntry^>
		{
		public:
			// Create an enumerator that will run through a tree.
			TreeEntryEnumerator(::TTree *treePtr);

			inline ~TreeEntryEnumerator (void)
			{}

			bool MoveNext (void);

			virtual bool MoveNext2() sealed = System::Collections::IEnumerator::MoveNext
			{ return MoveNext(); }

			property ROOTNET::Utility::TreeEntry ^Current
			{
				virtual ROOTNET::Utility::TreeEntry ^get()
				{ create_cached_ptr (); return _cached_ptr; }
			}

			property Object^ Current2
			{
				virtual Object^ get() sealed = System::Collections::IEnumerator::Current::get
				{ create_cached_ptr (); return _cached_ptr; }
			}

			void Reset()
			{}

			virtual void Reset2 () sealed = System::Collections::IEnumerator::Reset
			{ Reset(); }

		private:
			// Pointer to the tree we need to be looking at.
			::TTree *_tree;
			unsigned long _current_entry;
			ROOTNET::Utility::TreeEntry ^_cached_ptr;
			TreeManager ^_manager;

			void create_cached_ptr (void);
		};

	}
}

