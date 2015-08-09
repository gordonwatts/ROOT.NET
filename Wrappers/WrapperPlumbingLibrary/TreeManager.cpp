#include "TreeManager.hpp"
#include "NetStringToConstCPP.hpp"
#include "TreeLeafExecutor.hpp"
#include "ROOTDotNet.h"
#include "ROOTDOTNETBaseTObject.hpp"
#include "ROOTDOTNETVoidObject.hpp"

#include "TreeROOTValueExecutor.h"
#include "TreeLeafExecutorFactories.h"

#include "TTree.h"
#include "TLeaf.h"
#include "TClassEdit.h"

#include <string>
#include <vector>
#include <sstream>

using std::string;
using std::vector;
using std::ostringstream;

#ifdef nullptr
#undef nullptr
#endif

namespace ROOTNET
{
	namespace Utility
	{
		TreeManager::TreeManager(::TTree *tree)
			: _tree (tree)
		{
			_executors = gcnew System::Collections::Generic::Dictionary<System::String^, TreeLeafExecutor ^>();
			if (_executor_factories == nullptr) {
				_executor_factories = CreateKnownLeafFactories();
			}
		}

		///
		/// Our main job. Sort through everything we know about this leaf (if it exists) and attempt to find
		/// or create an executor for it that will return the object.
		///
		TreeLeafExecutor ^TreeManager::get_executor (System::Dynamic::GetMemberBinder ^binder)
		{
			//
			// Get the leaf name, see if we've already looked for one of these guys.
			//

			if (_executors->ContainsKey(binder->Name))
				return _executors[binder->Name];

			//
			// Now we are going to have to see if we can find the branch. If the branch isn't there, then
			// we are done - this is just a case of the user not knowing what the leaf name is. We let the
			// DLR deal with this error.
			//

			NetStringToConstCPP leaf_name_net (binder->Name);
			string leaf_name (leaf_name_net);
			auto branch = _tree->GetBranch(leaf_name.c_str());
			if (branch == nullptr)
				return nullptr;

			auto leaf = branch->GetLeaf(leaf_name.c_str());
			if (leaf == nullptr)
				return nullptr;
			string leaf_type (leaf->GetTypeName());

			//
			// Simplify everything if we can
			//

			leaf_type = TClassEdit::ResolveTypedef (leaf_type.c_str(), true);
			leaf_type = TClassEdit::ShortType (leaf_type.c_str(), TClassEdit::kDropStlDefault);

			//
			// Now we look for a leaf executor. First, is this a type we can just "deal with"?
			//

			TreeLeafExecutor ^result = nullptr;
			auto leaf_type_net (gcnew System::String(leaf_type.c_str()));
			if (_executor_factories->ContainsKey(leaf_type_net)) {
				result = _executor_factories[leaf_type_net]->Generate(branch);
			}

			//
			// Perhaps it is a ROOT object that we can then translate as we please?
			//

			if (result == nullptr) {
				auto cls_info = ::TClass::GetClass(leaf_type.c_str());
				if (cls_info != nullptr)
				{
					result = gcnew tle_root_object (branch, cls_info);
				}
			}

			//
			// If we couldn't match that means we have the proper leaf, but we don't know how to deal with
			// the type. So we actually have a failing of this DLR section. Pop an error so the user doesn't
			// get confused about what went wrong.
			//

			if (result == nullptr)
			{
				ostringstream err;
				err << "TTree leaf '" << leaf_name << "' has type '" << leaf_type << "'; ROOT.NET's dynamic TTree infrastructure doesn't know how to deal with that type, unfortunately!";
				throw gcnew System::InvalidOperationException(gcnew System::String(err.str().c_str()));
			}

			//
			// We have a good reader if we make it here. Cache it so next time through this is fast
			// (eventually the DLR will do the caching for us most of the time!).
			//

			_executors[binder->Name] = result;
			return result;
		}
	}
}
