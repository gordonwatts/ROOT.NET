///
/// Code to help track what root types we know about -- so we can do a quick lookup of all the ROOT types when we
/// need to get at one to wrap a root object.
///

#include "root_type_holder.hpp"
#include "TList.h"
#include "TClass.h"
#include "TBaseClass.h"
#include "TSystem.h"
#ifdef nullptr
#undef nullptr
#endif

using namespace System;
using namespace System::Reflection;

#include <string>
using std::string;

namespace ROOTNET {
	namespace Utility{
		///
		/// Static constructor -- when this is is called we will hook ourselves
		/// into the assembly loading mechanism of the app domain we are running in.
		/// This will keep us appraised of new loads -- which will mean that we need
		/// to rescan the various things out there.
		///
		/// I have no idea how this works if it tries to go cross-app domain. Not tested. :-)
		///
		static root_type_holder::root_type_holder (void)
		{
			///
			/// Setup a call back as that we will know about all the assemblies that are added
			/// to this app domain in the future. That way, when a new ROOT one is added we
			/// can just have that one re-scanned.
			///

			///
			/// Now, get a list of all the current ones loaded and add them to the list
			/// that should be scanned.
			///

			for each (Assembly ^ass in AppDomain::CurrentDomain->GetAssemblies()) {
				if (is_root_assembly(ass)) {
					_assemblies_to_scan->Enqueue(ass);
				}
			}
		}

		///
		/// Test to see if this is an assembly containing root types. For now we'll just look at the name,
		/// but a custom property could be used...
		///
		bool root_type_holder::is_root_assembly (Assembly ^ass)
		{
			return ass->GetName()->Name->Contains("Wrapper");
		}

		///
		/// If the class info has not been loaded, then load it from either all availible
		/// assemblies or from all assemblies that are indicated as awaiting loading.
		///
		void root_type_holder::fill_type_table(void)
		{
			while (_assemblies_to_scan->Count > 0) {
				Assembly ^ass_to_scan = _assemblies_to_scan->Dequeue();

				/// Make sure we've not already scanned it!
				if (_assemblies_scanned->ContainsKey(ass_to_scan)) {
					continue;
				}
				_assemblies_scanned[ass_to_scan] = true;

				/// Add its types to our table
				scan_assembly_for_root_types (ass_to_scan);
			}
		}

		///
		/// Scan one assmebly for all ROOT types in it.
		///
		void root_type_holder::scan_assembly_for_root_types(System::Reflection::Assembly ^assembly)
		{
			for each (Type ^t in assembly->GetExportedTypes()) {
				if (t->FullName->StartsWith("ROOTNET.") && !t->FullName->StartsWith("ROOTNET.Interface")) {
					_class_map[t->Name->Substring(1)] = t;
				}
			}
		}

		///
		/// Given a TObject do our best to figure out what type we should wrap it with.
		///
		/// One could add dynamic compilation right here for objects that weren't know
		/// this might be useful for a dynamic .NET language (but not so much for something
		/// like C#).
		///
		/// If there are multiple classes that inherrit from this guy we do a branch-search, going
		/// down the first inherritance branch and then the second (recursively). I doubt this
		/// is the best algorithm, but until there is a problem it seems to work!
		///
		Type ^root_type_holder::GetBestMatchType(::TBaseClass *base_class_info)
		{
			///
			/// Move to something more reasonable - that has actual information in it!
			///

			return GetBestMatchType(base_class_info->GetClassPointer());
		}

		Type ^root_type_holder::GetBestMatchType(::TClass *class_info)
		{
			/// Walk the inherritance tree to see if we can find something...
			/// Unloaded classes are death - the class simulation is almost non-existant.

			String ^cpp_class_name = gcnew String(class_info->GetName());
			String ^net_class_name = NetTranslatedClass(cpp_class_name);

			if (net_class_name != nullptr) {
				return _class_map[net_class_name];
			}

			///
			/// Is this a class that is well known, but its wrapper library just isn't loaded yet?
			/// Note: ROOT has the .rootmap facility so it can tell where unknown class libraries are located.
			/// We have to duplicate that functionality here to some extent. To do that we are making a pretty
			/// explicit assumption about the names of the wrapper libraries and how they were generated. If that
			/// isn't correct, this code will fail. So this is a bit brittle.
			///

			if (class_info->GetSharedLibs()) {
				string shared_library (class_info->GetSharedLibs());
				int first_space = shared_library.find(" ");
				if (first_space != shared_library.npos) {
					shared_library = shared_library.substr(0, first_space);
				}

				int dot_index = shared_library.find(".");
				if (dot_index != shared_library.npos) {
					shared_library = shared_library.substr(0, dot_index);
				}

				try {
					if (gSystem->Load(shared_library.c_str()) >= 0) {
						string wrapperLib = shared_library + "Wrapper";
						System::Reflection::Assembly::Load(gcnew String(wrapperLib.c_str()));
						net_class_name = NetTranslatedClass(cpp_class_name);
						if (net_class_name != nullptr) {
							return _class_map[net_class_name];
						}
					}
				} catch (std::exception &)
				{
				} catch (System::Exception ^)
				{
				}
			}

			///
			/// Have to check base classes to see if we have a match. Make sure this class is "good"
			/// first.

			if (!class_info->IsLoaded())
				return nullptr;

			///
			/// Get the list of bases and then recurse through it.
			///

			::TList *base_classes = class_info->GetListOfBases();
			if (base_classes == 0 || base_classes->GetEntries() == 0) {
				/// Wow. Not even TObject! :-)
				return nullptr;
			}

			for (int i = 0; i < base_classes->GetEntries(); i++)
			{
				TBaseClass *c = static_cast<::TBaseClass*>(base_classes->At(0));
				Type ^t = GetBestMatchType(c);
				if (t != nullptr) {
					return t;
				}
			}

			///
			/// No luck - we really know nothing abou this class! Wow!
			///

			return nullptr;
		}

		///
		/// Looks to see if we know about a particular class trnaslated the usual way. If not, return
		/// a null pointer. If so, then return that class name.
		///
		System::String ^root_type_holder::NetTranslatedClass(System::String ^root_class_name)
		{
			System::String ^net_class_name = gcnew System::String ("ROOTNET.N" + root_class_name);
			if (!_class_map->ContainsKey(root_class_name)) {
				if (!findClassInLoadedAssemblies(root_class_name, net_class_name)) {
					return nullptr;
				}
			}
			return root_class_name;
		}

		///
		/// Scan loaded assemblies for this particular class.
		///
		bool root_type_holder::findClassInLoadedAssemblies(System::String ^root_class_name, System::String ^full_net_class_name)
		{
			///
			/// Loop over all assemblies we know about in our app domain. There might be an eaiser way to
			/// do this - but I'm not aware of what it is. :-)
			///

			for each (Assembly ^ass in AppDomain::CurrentDomain->GetAssemblies()) {
				if (is_root_assembly(ass)) {
					Type ^result = ass->GetType(full_net_class_name, false);
					if (result != nullptr) {
						_class_map[root_class_name] = result;
						return true;
					}
				}
			}
			return false;
		}
	}
}