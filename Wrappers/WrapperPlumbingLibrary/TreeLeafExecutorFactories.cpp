//
// All the factories for all the leaf types (well, most) that we can
// recognize by simple pattern recognition.
//
#include "TreeLeafExecutorFactories.h"
#include "TreeLeafExecutor.hpp"
#include "TreeManager.hpp"

#include "TreeSimpleValueExecutor.h"
#include "TreeVectorValueExecutor.h"

#include <string>
using namespace std;

#include "TBranch.h"
namespace {
	using namespace ROOTNET::Utility;
	/// Lots of factory objects. It would have been awsome to use lambda's here, however the
	/// ::TBranch means we can't use templates for lambda funcions! These guys also have the function
	/// of linking the generic and C++ template world together.

#define MAKE_TLEF(nt, ct)	ref class TLEF_##ct : TreeLeafExecutorFactory \
	{ \
	public: \
		virtual TreeLeafExecutor ^Generate (::TBranch *branch) override \
		{ \
				return gcnew tle_simple_type<nt> (new tle_simple_type_accessor<ct> (branch)); \
		} \
	};
#define MAKE_UTLEF(nt,ct)	ref class TLEF_u##ct : TreeLeafExecutorFactory \
	{ \
	public: \
		virtual TreeLeafExecutor ^Generate (::TBranch *branch) override \
		{ \
				return gcnew tle_simple_type<unsigned nt> (new tle_simple_type_accessor<unsigned ct> (branch)); \
		} \
	};
#define MAKE_TLEVF(t)	ref class TLEVF_##t : TreeLeafExecutorFactory \
	{ \
	public: \
		virtual TreeLeafExecutor ^Generate (::TBranch *branch) override \
		{ \
				return gcnew tle_vector_type (new tle_vector_type_exe<t>(branch)); \
		} \
	};
#define MAKE_UTLEVF(t)	ref class TLEVF_u##t : TreeLeafExecutorFactory \
	{ \
	public: \
		virtual TreeLeafExecutor ^Generate (::TBranch *branch) override \
		{ \
				return gcnew tle_vector_type (new tle_vector_type_exe<unsigned t>(branch)); \
		} \
	};



#define MAKE_TLEF_REF(r,t) r[#t] = gcnew TLEF_##t()
#define MAKE_UTLEF_REF(r,t) r["unsigned " #t] = gcnew TLEF_u##t()
#define MAKE_TLEVF_REF(r,t) r["vector<" #t ">"] = gcnew TLEVF_##t()
#define MAKE_UTLEVF_REF(r,t) r["vector<unsigned " #t ">"] = gcnew TLEVF_u##t()

	MAKE_TLEF(char,char);
	MAKE_TLEVF(char);
	MAKE_UTLEF(char,char);
	MAKE_UTLEVF(char);

	MAKE_TLEF(int,int);
	MAKE_TLEVF(int);
	MAKE_UTLEF(int,int);
	MAKE_UTLEVF(int);

	MAKE_TLEF(short,short);
	MAKE_TLEVF(short);
	MAKE_UTLEF(short,short);
	MAKE_UTLEVF(short);

	MAKE_TLEF(long,long);
	MAKE_TLEVF(long);
	MAKE_UTLEF(long,long);
	MAKE_UTLEVF(long);

	MAKE_TLEF(bool, bool);
	MAKE_TLEVF(bool);

	MAKE_TLEF(float,float);
	MAKE_TLEVF(float);

	MAKE_TLEF(double, double);
	MAKE_TLEVF(double);

	MAKE_TLEF(System::String^, string);
	MAKE_TLEVF(string);
}

namespace ROOTNET {
	namespace Utility {
		System::Collections::Generic::Dictionary<System::String^, TreeLeafExecutorFactory ^> ^CreateKnownLeafFactories()
		{
			auto result = gcnew System::Collections::Generic::Dictionary<System::String^, TreeLeafExecutorFactory ^> ();

			MAKE_TLEF_REF(result, char);
			MAKE_TLEVF_REF(result, char);
			MAKE_UTLEF_REF(result, char);
			MAKE_UTLEVF_REF(result, char);

			MAKE_TLEF_REF(result, int);
			MAKE_TLEVF_REF(result, int);
			MAKE_UTLEF_REF(result, int);
			MAKE_UTLEVF_REF(result, int);

			MAKE_TLEF_REF(result, short);
			MAKE_TLEVF_REF(result, short);
			MAKE_UTLEF_REF(result, short);
			MAKE_UTLEVF_REF(result, short);

			MAKE_TLEF_REF(result, long);
			MAKE_TLEVF_REF(result, long);
			MAKE_UTLEF_REF(result, long);
			MAKE_UTLEVF_REF(result, long);

			MAKE_TLEF_REF(result, float);
			MAKE_TLEVF_REF(result, float);

			MAKE_TLEF_REF(result, double);
			MAKE_TLEVF_REF(result, double);

			MAKE_TLEF_REF(result, bool);
			MAKE_TLEVF_REF(result, bool);

			MAKE_TLEF_REF(result, string);
			MAKE_TLEVF_REF(result, string);
			return result;
		}
	}
}
