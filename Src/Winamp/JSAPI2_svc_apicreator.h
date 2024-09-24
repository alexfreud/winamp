#pragma once

#include <bfc/dispatch.h>
#include <bfc/std_mkncc.h>
#include <ocidl.h>
#include "jsapi2_api_security.h"
#include "JSAPI_Info.h"

namespace JSAPI2
{
	class svc_apicreator : public Dispatchable
	{
	public:
		enum
		{
			AUTHORIZATION_UNDEFINED = 0x0,
			AUTHORIZATION_ALLOW = 0x1,
			AUTHORIZATION_DENY = 0x2,

			AUTHORIZATION_MASK = 0xFFF,

			AUTHORIZATION_FLAG_ALWAYS = 0x1000,
			AUTHORIZATION_FLAG_ALWAYS_FOR_SERVICE = 0x2000,
			AUTHORIZATION_FLAG_GROUP_ONLY = 0x4000,
	};
	public:
		static FOURCC getServiceType() { return MK4CC('j','s','a','c'); }
		// this key is guaranteed to out-live your object, so you can just save the pointer
		// also, DO NOT add a reference to the info object or else you will create a circular reference
		IDispatch *CreateAPI(const wchar_t *name, const wchar_t *key, JSAPI::ifc_info *info);

		// returns new AUTHORIZATION_* enum to save to settings, or AUTHORIZATION_UNDEFINED if this isn't our API
		int PromptForAuthorization(HWND parent, const wchar_t *group, const wchar_t *action, const wchar_t *authorization_key, JSAPI2::api_security::AuthorizationData *data);
		enum
		{
			JSAPI2_SVC_APICREATOR_CREATEAPI = 0,
			JSAPI2_SVC_APICREATOR_PROMPTFORAUTHORIZATION = 1,
		};
	};

}

inline IDispatch *JSAPI2::svc_apicreator::CreateAPI(const wchar_t *name, const wchar_t *key, JSAPI::ifc_info *info)
{
	return _call(JSAPI2_SVC_APICREATOR_CREATEAPI , (IDispatch *)0, name, key, info);
}

inline int JSAPI2::svc_apicreator::PromptForAuthorization(HWND parent, const wchar_t *group, const wchar_t *action, const wchar_t *authorization_key, JSAPI2::api_security::AuthorizationData *data)
{
	return _call(JSAPI2_SVC_APICREATOR_PROMPTFORAUTHORIZATION, (int)AUTHORIZATION_UNDEFINED, parent, group, action, authorization_key, data);
}
