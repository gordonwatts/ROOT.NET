///
/// Code to manage the link between C++ and .NET objects.
///

#include "ROOTObjectManager.h"
#include "ROOTDOTNETBaseTObject.hpp"

#include "TROOT.h"
#ifdef nullptr
#undef nullptr
#endif

using namespace System::Reflection;
using namespace System;
using namespace System::Threading;

namespace ROOTNET
{
	namespace Utility
	{

		/// Helper class given to root to get called when an object needs some sort of clean up.
		/// NOTE: there is no finalizer on this object. However, if ROOT tries to call it after the GC has shut
		/// down the managed stuff we will be in real trouble. We hope that the ROOTObjectManager will
		/// always be GC at the same time. If that is the case, then the linkage here will get removed.
		class ROOTCleanupCallback : public ::TObject {
			inline void RecursiveRemove (::TObject *obj)
			{
				ROOTObjectManager::instance()->ROOTObjectRemoval(obj);
			}
		};

		///
		/// Create a new instance of the object manager if it isn't around, otherwise provide access
		/// to the singleton.
		///
		ROOTObjectManager ^ROOTObjectManager::instance(void)
		{
			if (_instance == nullptr) {
				_instance = gcnew ROOTObjectManager();
			}
			return _instance;
		}

		///
		/// Someone is explicitly deleting us -- which is odd. Or we are going out of scope and we
		/// are on the stack. Not obvious that this should ever happen. However, we will be good and
		/// call the "finalizer".
		///
		ROOTObjectManager::~ROOTObjectManager (void)
		{
			this->!ROOTObjectManager();
		}

		///
		/// This object is done. Most likely, the garbage collector has come along to
		/// clean this object up. We need to remove our connection to the ROOT callback
		/// system. Or ROOT might try to call us as it shuts down -- after we've already
		/// shut down! Yikes!
		///
		ROOTObjectManager::!ROOTObjectManager(void)
		{
			CleanupManager();
			_instance = nullptr;
		}

		/// Clean up our connection to root.
		void ROOTObjectManager::CleanupManager(void)
		{
			gROOT->GetListOfCleanups()->Remove(_callback);
		}

		/// Allocate the memory we need to keep around.
		ROOTObjectManager::ROOTObjectManager(void)
		{
			_root_mapping = gcnew Dictionary<int, ROOTObjectInfo^>();
			_reader_writer_root_map_lock = gcnew ReaderWriterLockSlim();
			_callback = new ROOTCleanupCallback();
			gROOT->GetListOfCleanups()->Add(_callback);
		}

		///
		/// Start tracking a ROOT object.
		///  - Make sure that ROOT will call our clean up guy!
		///  - Make an entry in our table (indexed by the ::TObject pointer).
		///
		void ROOTObjectManager::RegisterObject (::TObject *obj, ROOTNET::Utility::ROOTDOTNETBaseTObject ^root_obj)
		{
			///
			/// I would never expect this to happen.
			///

			if (obj == 0) {
				return;
			}

			///
			/// Is this object already in the table? If not, then we should put it in!
			///

			_reader_writer_root_map_lock->EnterUpgradeableReadLock();
			try {
				if (!InternalKnowAboutROOTObject(obj)) {
					obj->SetBit(kMustCleanup);
					int myptr = reinterpret_cast<int>(obj);
					_reader_writer_root_map_lock->EnterWriteLock();
					try {
						_root_mapping[myptr] = gcnew ROOTObjectInfo (obj, root_obj);
					} finally {
						_reader_writer_root_map_lock->ExitWriteLock();
					}
				}
			} finally {
				_reader_writer_root_map_lock->ExitUpgradeableReadLock();
			}
		}

		///
		/// See if we can find the object. Return it if we can.
		/// Requires reader lock
		///
		ROOTObjectInfo ^ROOTObjectManager::InternalFindROOTObject (const ::TObject *obj)
		{
			if (InternalKnowAboutROOTObject(obj)) {
				int myptr = reinterpret_cast<int>(obj);
				return _root_mapping[myptr];
			}

			return nullptr;
		}

		///
		/// Do it from the point of view of an integer pointer.
		/// Lock: Reader
		///
		ROOTObjectInfo ^ROOTObjectManager::InternalFindROOTObject (int obj_prj)
		{
			return InternalFindROOTObject (reinterpret_cast<::TObject*>(obj_prj));
		}

		///
		/// Do we know about this particular object?
		/// Lock: Reader
		///
		bool ROOTObjectManager::InternalKnowAboutROOTObject (const ::TObject *obj)
		{
			int myptr = reinterpret_cast<int>(obj);
			return _root_mapping->ContainsKey(myptr);
		}

		///
		/// An object that we are possibly tracking is being removed. Usually this will happen because ROOT
		/// has decided that the object it is holding onto is getting deleted (say, when you close a file
		/// which has a TH1 attached to it).
		///
		void ROOTObjectManager::ROOTObjectRemoval(::TObject *obj)
		{
			_reader_writer_root_map_lock->EnterUpgradeableReadLock();
			try {
				/// Deal with trival things. We aren't the only ones that are doing cleanups...
				ROOTObjectInfo ^info = InternalFindROOTObject (obj);
				if (info == nullptr) {
					return;
				}

				///
				/// We know about it.
				/// - Be careful that the object hasn't already been GC'd (the GC runs in a seperate thread).
				/// - Remove the linkage between the object -- so make it forget.
				///

				const ROOTDOTNETBaseTObject ^netobj = info->GetNETObject();
				if (netobj != nullptr) {
					ROOTDOTNETBaseTObject ^non_const_obj = const_cast<ROOTDOTNETBaseTObject^>(netobj);
					non_const_obj->SetNull();
				}

				///
				/// And drop the object from our internal table. Those that already have the object will
				/// continue to hold onto it. But it can't come back from root any longer as root is the
				/// one telling us to drop it!
				///

				_reader_writer_root_map_lock->EnterWriteLock();
				try {
					InternalForgetAboutObject(obj);
				} finally {
					_reader_writer_root_map_lock->ExitWriteLock();
				}
			} finally {
				_reader_writer_root_map_lock->ExitUpgradeableReadLock();
			}
		}

		///
		/// Drop an object from our intenral table.
		/// Lock: Writer
		///
		void ROOTObjectManager::InternalForgetAboutObject (::TObject *obj)
		{
			_root_mapping->Remove(reinterpret_cast<int>(obj));
		}

		///
		/// Drop an object from our internal table.
		///
		void ROOTObjectManager::ForgetAboutObject (::TObject *obj)
		{
			_reader_writer_root_map_lock->EnterWriteLock();
			try {
				InternalForgetAboutObject(obj);
			} finally {
				_reader_writer_root_map_lock->ExitWriteLock();
			}
		}
	}
}
