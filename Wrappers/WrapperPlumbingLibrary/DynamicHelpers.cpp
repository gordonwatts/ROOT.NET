#include "DynamicHelpers.h"
#include "ROOTDOTNETBaseTObject.hpp"
#include "NetStringToConstCPP.hpp"
#include "ROOTDOTNETBaseTObject.hpp"
#include "ROOTDotNet.h"

#include <Api.h>
#include <TClass.h>
#include <TList.h>
#include <TMethod.h>
#include <TMethodArg.h>
#include <TROOT.h>
#include <TDataType.h>
#include "TClassEdit.h"

#include <typeinfo>
#include <algorithm>
#include <iterator>
#include <sstream>

using std::string;
using std::vector;
using std::ostringstream;
using std::max;
using std::back_inserter;

#ifdef nullptr
#undef nullptr
#endif

namespace {
	using namespace ROOTNET::Utility;

	//
	// Deal with CINT when we have a string argument or return.
	//
	class RTCString : public ROOTTypeConverter
	{
	public:
		RTCString(bool isConst)
			: _cache (nullptr), _isConst (isConst)
		{}
		~RTCString()
		{
			delete[] _cache;
		}

		string GetArgType() const
		{
			if (_isConst)
				return "const char*";
			return "char*";
		}

		void SetArg (System::Object ^obj, Cint::G__CallFunc *func)
		{
			ROOTNET::Utility::NetStringToConstCPP carg((System::String^) obj);
			string sarg(carg);
			_cache = new char[sarg.size()+1];
			strncpy (_cache, sarg.c_str(), sarg.size());
			_cache[sarg.size()] = 0;
			func->SetArg(reinterpret_cast<long>(_cache));
		}
		
		bool Call (G__CallFunc *func, void *ptr, System::Object^% result)
		{
			auto c = reinterpret_cast<char*>(func->ExecInt(ptr));
			result = gcnew System::String(c);
			return true;
		}
	private:
		// We need to make sure the string stays around...
		char *_cache;
		bool _isConst;
	};


	///// Type Traits to deal with the actual calling for simple types.

	template<typename T>
	struct RTCBasicTypeCallerTraits
	{
		static T Call (G__CallFunc *func, void *ptr);
	};

	template<>
	struct RTCBasicTypeCallerTraits<long>
	{
		static long Call (G__CallFunc *func, void *ptr)
		{
			return func->ExecInt(ptr);
		}
	};
	template<>
	struct RTCBasicTypeCallerTraits<unsigned long>
	{
		static unsigned long Call (G__CallFunc *func, void *ptr)
		{
			return (unsigned long) func->ExecInt(ptr);
		}
	};
	template<>
	struct RTCBasicTypeCallerTraits<double>
	{
		static double Call (G__CallFunc *func, void *ptr)
		{
			return func->ExecDouble(ptr);
		}
	};

	//////
	// Converter for an actual type in the code. A little tricky because we have to deal with
	// all sorts of conversions, etc.
	//
	// ArgType - this is the type that is in the method prototype (e.g. float)
	// CintType - when dealing with CInt's call interface (e.g. double in this case)
	// ActualType - this is the type of the actual argument object (e.g. int)
	//
	//  So the int needs to be converterd to the double to call ROOT, and upon a return, it
	// should be getting back a double. :-)
	//
	template<typename ArgType, typename CintType, typename ActualType>
	class RTCBasicType : public ROOTTypeConverter
	{
	public:
		RTCBasicType () {}
		string GetArgType() const { return typeid(ArgType).name(); }
		void SetArg (System::Object ^obj, Cint::G__CallFunc *func)
		{
			auto val = *reinterpret_cast<ActualType^>(obj);
			CintType r = (CintType) val;
			func->SetArg(r);
		}
		bool Call (G__CallFunc *func, void *ptr, System::Object^% result)
		{
			CintType r = RTCBasicTypeCallerTraits<CintType>::Call(func, ptr);
			result = (ArgType) r;
			return true;
		}
	private:
	};

	//
	// Pointer to a ROOT TObject based object.
	//
	class RTCROOTType : public ROOTTypeConverter
	{
	public:
		inline RTCROOTType (TClass *cls, const string &type_spec)
			: _cls(cls), _type_spec (type_spec)
		{}

		string GetArgType() const { return _type_spec; }

		//
		// Do the object pointer.
		// 
		// Since we handle only TObject's here, we don't have to worry about multiple inherritance,
		// and offests for various pointers. Everything has to come from a TObject, and we start from there.
		//
		void SetArg (System::Object ^obj, Cint::G__CallFunc *func)
		{

			auto rdnobj = (ROOTDOTNETBaseTObject^) obj;
			auto robj = rdnobj->GetTObjectPointer();
			void *ptr = static_cast<void*>(robj);

			func->SetArg(reinterpret_cast<long>(ptr));
		}

		//
		// Go get an object. A null pointer is ok to come back. But we can't
		// do much about its type in this dynamic area.
		// Ownership is not ours - the ROOT system (or someone else) is responsible.
		bool Call (G__CallFunc *func, void *ptr, System::Object^% result)
		{
			void *r = (void*) func->ExecInt(ptr);
			if (r == nullptr)
			{
				return true;
			}

			auto obj = reinterpret_cast<::TObject*>(r);
			auto rdnobj = ROOTObjectServices::GetBestObject<ROOTDOTNETBaseTObject^>(obj);
			result = rdnobj;
			return true;
		}
	private:
		::TClass *_cls;
		const string _type_spec;
	};

	//
	// Pointer to a non-TObject object, but one in ROOT's dictionaries.
	//
	class RTCNonTObjectType : public ROOTTypeConverter
	{
	public:
		inline RTCNonTObjectType (TClass *cls, const string &fully_qualified_type)
			: _cls(cls), _type_spec(fully_qualified_type)
		{}

		string GetArgType() const { return _type_spec; }

		//
		// Do the object pointer. We are making a basic assumption here that there
		// is no multiple inherritance. Which is going to be wrong sometimes and cause
		// a crash!
		// WARNING: this does not deal correctly with multiple inherritance!
		// 
		void SetArg (System::Object ^obj, Cint::G__CallFunc *func)
		{

			auto rdnobj = (ROOTDOTNETBaseTObject^) obj;
			auto robj = rdnobj->GetVoidPointer();

			func->SetArg(reinterpret_cast<long>(robj));
		}

		//
		// Go get an object. A null pointer is ok to come back. But we can't
		// do much about its type in this dynamic area.
		// Ownership is not ours - the ROOT system (or someone else) is responsible.
		bool Call (G__CallFunc *func, void *ptr, System::Object^% result)
		{
			void *r = (void*) func->ExecInt(ptr);
			if (r == nullptr)
			{
				return true;
			}

			throw gcnew System::InvalidOperationException ("do not know how to call the Get Best Object here");
#ifdef notyet
			auto rdnobj = ROOTObjectServices::GetBestObject<ROOTDOTNETBaseTObject^>(obj);
			result = rdnobj;
			return true;
#endif
		}
	private:
		::TClass *_cls;
		const string _type_spec;
	};

	// Direct object, no pointer. we own the thing now.
	class RTCROOTTypeCtorObject : public ROOTTypeConverter
	{
	public:
		inline RTCROOTTypeCtorObject (TClass *cls)
			: _cls (cls)
		{}

		string GetArgType() const { return string(_cls->GetName()); }
		void SetArg (System::Object ^obj, Cint::G__CallFunc *func) {}

		// We own the memory we come back with.
		bool Call (G__CallFunc *func, void *ptr, System::Object^% result)
		{
			void *r = (void*) func->ExecInt(ptr);
			if (r == nullptr)
				return false;

			// Assume it is a TObject (we should be safe here)
			auto obj = reinterpret_cast<::TObject*>(r);
			auto rdnobj = ROOTObjectServices::GetBestObject<ROOTDOTNETBaseTObject^>(obj);
			rdnobj->SetNativePointerOwner(true); // We track this and delete it when it is done!
			result = rdnobj;
			return true;
		}
	private:
		::TClass *_cls;
	};

	// Direct object, no pointer. we own the thing, further, this is for creating these guys!
	class RTCNonTObjectTypeCtorObject : public ROOTTypeConverter
	{
	public:
		inline RTCNonTObjectTypeCtorObject (TClass *cls)
			: _cls (cls)
		{}

		string GetArgType() const { return string(_cls->GetName()); }
		void SetArg (System::Object ^obj, Cint::G__CallFunc *func) {}

		// We own the memory we come back with.
		bool Call (G__CallFunc *func, void *ptr, System::Object^% result)
		{
			void *r = (void*) func->ExecInt(ptr);
			if (r == nullptr)
				return false;

			//
			// Get some sort of non-tobject guy back.
			//

			auto rdnobj = ROOTObjectServices::GetBestNonTObjectObject (r, _cls);
			rdnobj->SetNativePointerOwner(true); // We track this and delete it when it is done!
			result = rdnobj;
			return true;
		}
	private:
		::TClass *_cls;
	};

	class RTCVoidType : public ROOTTypeConverter
	{
	public:

		string GetArgType() const { return "void";}
		void SetArg(System::Object ^obj, Cint::G__CallFunc *func) {}

		bool Call (G__CallFunc *fun, void *ptr, System::Object^% result)
		{
			fun->Exec(ptr);
			result = nullptr;
			return true;
		}
	};

	//
	// Create a converter to do the last bit of conversion between types.
	template<typename T, typename D>
	ROOTTypeConverter *make_basic_converter (System::Object^ obj)
	{
		if (obj == nullptr)
			return new RTCBasicType<T, D, int>();

		auto t = obj->GetType();
		if (t == int::typeid)
			return new RTCBasicType<T, D, int>();
		if (t == short::typeid)
			return new RTCBasicType<T, D, short>();
		if (t == long::typeid)
			return new RTCBasicType<T, D, long>();

		if (t == double::typeid)
			return new RTCBasicType<T, D, double>();
		if (t == float::typeid)
			return new RTCBasicType<T, D, float>();

		// Yikes! No idea!
		return nullptr;
	}

	//
	// Get a type converter for the given name.
	//
	ROOTTypeConverter *FindConverter (const string &tname, System::Object ^arg)
	{
		//
		// Any typedefs we need to worry about?
		//

		auto resolvedName = DynamicHelpers::resolveTypedefs(tname);

		if (resolvedName == "const char*")
			return new RTCString(true);
		if (resolvedName == "char*")
			return new RTCString(false);

		if (resolvedName == "short" || resolvedName == "const short" || resolvedName == "const short&")
			return make_basic_converter<short, long>(arg);
		if (resolvedName == "int" || resolvedName == "const int" || resolvedName == "const int&")
			return make_basic_converter<int, long>(arg);
		if (resolvedName == "long" || resolvedName == "const long" || resolvedName == "const long&")
			return make_basic_converter<long, long>(arg);

		if (resolvedName == "unsigned short" || resolvedName == "const unsigned short" || resolvedName == "const unsigned short&")
			return make_basic_converter<unsigned short, long>(arg);
		if (resolvedName == "unsigned int" || resolvedName == "const unsigned int" || resolvedName == "const unsigned int&")
			return make_basic_converter<unsigned int, long>(arg);
		if (resolvedName == "unsigned long" || resolvedName == "const unsigned long" || resolvedName == "const unsigned long&")
			return make_basic_converter<unsigned long, long>(arg);

		if (resolvedName == "double" || resolvedName == "const double" || resolvedName == "const double&")
			return make_basic_converter<double, double>(arg);

		if (resolvedName == "float" || resolvedName == "const float" || resolvedName == "const float&")
			return make_basic_converter<float, double>(arg);

		if (resolvedName == "void")
			return new RTCVoidType();

		auto cls_info = DynamicHelpers::ExtractROOTClassInfoPtr(resolvedName);
		if (cls_info != nullptr)
		{
			if (cls_info->InheritsFrom("TObject"))
				return new RTCROOTType(cls_info, resolvedName);
			return new RTCNonTObjectType (cls_info, resolvedName);
		}

		return nullptr;
	}

	///
	/// CTor returns require some special handling.
	///
	ROOTTypeConverter *FindConverterROOTCtor (const string &tname)
	{
		auto resolvedName = DynamicHelpers::resolveTypedefs(tname);

		auto cls_info = TClass::GetClass(resolvedName.c_str());
		if (cls_info != nullptr)
		{
			if (cls_info->InheritsFrom("TObject"))
				return new RTCROOTTypeCtorObject(cls_info); // Ctor return...
			return new RTCNonTObjectTypeCtorObject(cls_info);
		}
		return nullptr;
	}

	//
	// Return a type converter for this argument
	//
	ROOTTypeConverter *FindConverter (TMethodArg *arg, System::Object ^argv)
	{
		return FindConverter(arg->GetFullTypeName(), argv);
	}

	vector<string> do_all_arg_possibilities (const string &inital_args,
		const vector<vector<string>>::const_iterator &next_arg,
		const vector<vector<string>>::const_iterator &last_arg
		)
	{
		vector<string> result;
		if (next_arg == last_arg)
		{
			result.push_back(inital_args);
		} else {
			for (int i = 0; i < next_arg->size(); i++) {
				auto new_initial_args (inital_args);
				if (new_initial_args.size() != 0)
					new_initial_args += ",";
				new_initial_args += (*next_arg)[i];
				auto possibles (do_all_arg_possibilities(new_initial_args, next_arg+1, last_arg));
				copy (possibles.begin(), possibles.end(), back_inserter(result));
			}
		}
		return result;
	}
}

namespace ROOTNET
{
	namespace Utility
	{
		DynamicHelpers::DynamicHelpers(void)
		{
		}

		//
		// Given the list of arguments, generate a prototype string
		// that CINT can understand for argument lookup. Some things have more
		// than one way we can hand them in. So we return a list of items in that
		// case.
		//
		vector<string> DynamicHelpers::GeneratePrototype(array<System::Object^> ^args)
		{
			string result = "";

			vector<vector<string>> all_arg_possibilities;
			for (int index = 0; index < args->Length; index++)
			{
				vector<string> thisType;
				auto arg = args[index];
				auto gt = arg->GetType();
				if (gt == int::typeid)
				{
					thisType.push_back("int");
				} else if (gt == long::typeid)
				{
					thisType.push_back("long");
				} else if (gt == short::typeid)
				{
					thisType.push_back("short");
				} else if (gt == unsigned int::typeid)
				{
					thisType.push_back("unsigned int");
				} else if (gt == unsigned long::typeid)
				{
					thisType.push_back("unsigned long");
				} else if (gt == unsigned short::typeid)
				{
					thisType.push_back("unsigned short");
				} else if (gt == float::typeid)
				{
					thisType.push_back("float");
				} else if (gt == double::typeid)
				{
					thisType.push_back("double");
				} else if (gt == System::String::typeid)
				{
					thisType.push_back("const char*");
					thisType.push_back("string");
				} else {
					// See if this is a class ptr that is part of the ROOT system.
					if (gt->IsSubclassOf(ROOTNET::Utility::ROOTDOTNETBaseTObject::typeid))
					{
						auto robj = static_cast<ROOTNET::Utility::ROOTDOTNETBaseTObject^>(arg);
						string rootname = string(robj->GetAssociatedTClassInfo()->GetName());
						thisType.push_back(rootname);
						thisType.push_back(rootname + "*");
					} else {
						return vector<string>();
					}
				}
				all_arg_possibilities.push_back(thisType);
			}

			//
			// Now, go through all the possibilities and see if we can't create a bunch of prototype strings.
			//

			return do_all_arg_possibilities ("", all_arg_possibilities.begin(), all_arg_possibilities.end());
		}

		///
		/// Look to see if the fully qualified class name (that has had all typedef's resolved)
		/// refers to a ROOT class. If so, return it, stripping it of all modifiers. Return null
		/// if we can't find a known class.
		///
		::TClass *DynamicHelpers::ExtractROOTClassInfoPtr (const string &tname)
		{
			//
			// Is there a const at the front?
			//

			auto cname (tname);
			if (cname.find("const ") == 0) {
				cname = cname.substr(6);
			}

			//
			// Remove any trailing pointer things
			//

			while (true)
			{
				int ptr = cname.rfind("*");
				if (ptr != cname.npos) {
					cname = cname.substr(0, ptr);
					continue;
				}
				ptr = cname.rfind("&");
				if (ptr != cname.npos) {
					cname = cname.substr(0, ptr);
					continue;
				}
				break;
			}

			return ::TClass::GetClass(cname.c_str());
		}

		//
		// Make sure all type-defs are taken care of. Deal with pointer info as well.
		//
		string DynamicHelpers::resolveTypedefs(const std::string &type)
		{
			string current (type);
			while (true)
			{
				current = TClassEdit::ResolveTypedef(current.c_str(), true);
				auto dtinfo = gROOT->GetType(current.c_str());
				if (dtinfo == nullptr)
					return current;
				auto newcurrent = dtinfo->GetTypeName();
				if (newcurrent == current)
					return current;
				current = newcurrent;
			}
		}

		///
		/// We will put together a caller for this list of arguments.
		///
		DynamicCaller *DynamicHelpers::GetFunctionCaller(::TClass *cls_info, const std::string &method_name_bg, array<System::Object^> ^args, bool is_ctor)
		{
			//
			// If this is a ctor, then there is a chance that the method name isn't our actual class name. This is because
			// we have to deal with vector<int> and vector<int, allocator<int>>... :-) Good old ROOT!
			//

			string method_name (method_name_bg);
			if (is_ctor)
			{
				if (cls_info->GetMethodAny(method_name.c_str()) == nullptr)
				{
					TIter iM(cls_info->GetListOfMethods());
					TMethod *m (nullptr);
					while ((m = (TMethod*) iM()))
					{
						if (TClass::GetClass(m->GetName()) == cls_info)
						{
							method_name = m->GetName();
							break;
						}
					}
				}
			}

			//
			// See if we can get the method that we will be calling for this function. Just go through the prototyupes until
			// one of them works!
			//

			auto protos = DynamicHelpers::GeneratePrototype(args);
			::TMethod *method (nullptr);
			for (int i = 0; i < protos.size(); i++) {
				method = cls_info->GetMethodWithPrototype(method_name.c_str(), protos[i].c_str());
				if (method != nullptr)
					break;
			}
			if (method == nullptr)
				return nullptr;

			//
			// Now, we can get a list of converters for the input types.
			//

			auto arg_list = method->GetListOfMethodArgs();
			TIter next(arg_list);
			::TObject *obj;
			vector<ROOTTypeConverter*> converters;
			int index = 0;
			while((obj = next()) != nullptr)
			{
				System::Object ^arg = nullptr;
				if (index < args->Length)
					arg = args[index];
				converters.push_back(FindConverter(static_cast<TMethodArg*>(obj), arg));
				index++;
			}

			//
			// And the output type. We do something special for ctor's here as the resulting
			// output needs to be dealt with carefully.
			//

			ROOTTypeConverter *rtn_converter;
			if (is_ctor) {
				rtn_converter = FindConverterROOTCtor(method->GetReturnTypeName());
			} else {
				rtn_converter = FindConverter(method->GetReturnTypeName(), nullptr);
			}

			//
			// ANd create the guy for that will do the actual work with the above information. Make sure it is
			// valid. If not, trash it, and return null.
			//

			auto caller = new DynamicCaller(converters, rtn_converter, method);
			if (caller->IsValid()) {
				return caller;
			}
			delete caller;
			return nullptr;
		}

		///
		/// Create a holder that will keep converters, etc., for a function.
		///
		DynamicCaller::DynamicCaller (std::vector<ROOTTypeConverter*> &converters, ROOTTypeConverter* rtn_converter, ::TMethod *method)
			: _arg_converters(converters), _rtn_converter(rtn_converter), _method(method), _methodCall (nullptr)
		{
		}

		///
		/// Clean up everything.
		///
		DynamicCaller::~DynamicCaller(void)
		{
			delete _rtn_converter;
			for (int i = 0; i < _arg_converters.size(); i++) {
				delete _arg_converters[i];
			}

			delete _methodCall;
		}

		//
		// Make sure everything is cool to go!
		//
		bool DynamicCaller::IsValid() const
		{
			if (_rtn_converter == nullptr || _method == nullptr)
				return false;

			for each (ROOTTypeConverter*o in _arg_converters)
			{
				if (o == nullptr)
					return false;
			}

			return true;
		}

		///
		/// Do the call
		///  Code is basically copied from ConstructorHolder in pyroot.
		///
		System::Object^ DynamicCaller::CallCtor(::TClass *clsInfo, array<System::Object^> ^args)
		{
			//
			// Allocate space for this object, and set everything up.
			//

			System::Object ^result = nullptr;
			Call(clsInfo, args, result);

			//
			// If that fails, then see if we can do something else.
			//

			return result;
		}

		//
		// Do a call for a compiled in class.
		//
		bool DynamicCaller::Call(void *ptr, array<System::Object^> ^args, System::Object^% result)
		{
			///
			/// Get the calling function
			///
			if (_methodCall == nullptr)
			{
				G__ClassInfo *gcl = static_cast<G__ClassInfo*>(_method->GetClass()->GetClassInfo());
				if (gcl == nullptr)
					throw gcnew System::InvalidOperationException("ROOT Class we already know about we can't get the info for!");

				auto gmi = gcl->GetMethod(_method->GetName(), ArgList().c_str(), &_offset, G__ClassInfo::ExactMatch);
				if (!gmi.IsValid())
					throw gcnew System::InvalidOperationException("ROOT Method we already know about we can't get the method info for!");

				_methodCall = new G__CallFunc();
				_methodCall->Init();
				_methodCall->SetFunc(gmi);
			}

			//
			// Setup the arguments
			//

			_methodCall->ResetArg();
			for (int i_arg = 0; i_arg < args->Length; i_arg++)
			{
				_arg_converters[i_arg]->SetArg(args[i_arg], _methodCall);
			}

			//
			// Now execute the thing.
			//

			return _rtn_converter->Call (_methodCall, (void*)((long)ptr + _offset), result);
		}

		//
		// Get a prototype list for everyone...
		//

		string DynamicCaller::ArgList() const
		{
			ostringstream args;
			bool isFirst = true;
			for each (ROOTTypeConverter *c in _arg_converters)
			{
				if (!isFirst)
					args << ",";
				isFirst = false;

				args << c->GetArgType();
			}

			return args.str();
		}
	}
}
