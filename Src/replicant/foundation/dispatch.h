#pragma once

#include "guid.h"

#ifdef WIN32
#ifndef NOVTABLE
#define NOVTABLE __declspec(novtable)
#endif
#else
#define NOVTABLE
#endif


#if defined(__GNUC__) && (defined(__x86_32__) || defined(__i386__))
#define WASABICALL __attribute__((stdcall))
#elif defined (_WIN32)
#define WASABICALL __stdcall
#else
#define WASABICALL
#endif    

namespace Wasabi2
{
	class NOVTABLE Dispatchable
	{
	protected:
		Dispatchable(size_t _dispatchable_version) : dispatchable_version(_dispatchable_version) {}
		~Dispatchable() {}
	public:
#define dispatch_call(code, default_return, func) (DispatchValid(code))?(default_return):func
#define dispatch_voidcall(code, func) if (DispatchValid(code)) func

		bool DispatchValid(size_t code) const
		{
			return code < dispatchable_version;
		}

		size_t Retain()
		{
			return Dispatchable_Retain();
		}

		size_t Release()
		{
			return Dispatchable_Release();
		}

		int QueryInterface(GUID interface_guid, void **object)
		{
			return Dispatchable_QueryInterface(interface_guid, object);
		}

		template <class ifc_t>
		int QueryInterface(ifc_t **object)
		{
			return Dispatchable_QueryInterface(ifc_t::GetInterfaceGUID(), (void **)object);
		}

	protected:
		virtual size_t WASABICALL Dispatchable_Retain() { return 0; }
		virtual size_t WASABICALL Dispatchable_Release() { return 0; }
		virtual int WASABICALL Dispatchable_QueryInterface(GUID interface_guid, void **object) { return 1; }

		size_t dispatchable_version;
	};
}

#ifndef DECLARE_EXTERNAL_SERVICE
#define DECLARE_EXTERNAL_SERVICE(_type, _name) extern _type *_name
#endif

#ifndef DEFINE_EXTERNAL_SERVICE
#define DEFINE_EXTERNAL_SERVICE(_type, _name) _type *_name=0
#endif
