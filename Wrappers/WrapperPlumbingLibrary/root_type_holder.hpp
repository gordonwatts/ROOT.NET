#pragma once
///
/// helper class (internal to this assembly) that will keep track of
/// the types and the various assemblies we have scanned.
///

#include <string>

class TBaseClass;

namespace ROOTNET {
	namespace Utility {

		/// Helper to hold onto data on the classes we know about in root...
		ref class root_type_holder {
		public:
			/// Static initalizer that will hook us into the various points to monitor what libraries
			/// are loaded, etc.
			static root_type_holder (void);

			/// Returns the best match type for a class -- it will walk the inherritance hierarchy
			static System::Type ^GetBestMatchType (::TClass *class_info);

		private:
			/// Returns the best match type for a class -- it will walk the inherritance hierarchy
			static System::Type ^GetBestMatchType (::TBaseClass *class_info);

			/// This is the main library of all types.
			static System::Collections::Generic::Dictionary<System::String^, System::Type^> ^_class_map
				= gcnew System::Collections::Generic::Dictionary<System::String^, System::Type^>();

			/// Fill in any missing types in our lookup table by looking at all unscanned assemblies.
			static void fill_type_table (void);

			/// Scans a single assembly and fills the type table.
			static void scan_assembly_for_root_types (System::Reflection::Assembly ^assembly);

			/// Returns the name of the translated class if we know, or null otherwise.
			static System::String ^NetTranslatedClass(System::String ^root_class_name);

			/// Returns true if we find the class in some assembly that we have loaded
			static bool findClassInLoadedAssemblies (System::String ^root_class_name, System::String ^net_class_name);

			/// True if the given assembly contains ROOT types. False otherwise. If true, load it into our
			/// table.
			static bool is_root_assembly (System::Reflection::Assembly ^assembly);

			/// True if no scan has ever been run
			static System::Collections::Generic::Queue<System::Reflection::Assembly^> ^_assemblies_to_scan
				= gcnew System::Collections::Generic::Queue<System::Reflection::Assembly^>();
			static System::Collections::Generic::Dictionary<System::Reflection::Assembly^, bool> ^_assemblies_scanned
				= gcnew System::Collections::Generic::Dictionary<System::Reflection::Assembly^, bool> ();
		};

	}
}