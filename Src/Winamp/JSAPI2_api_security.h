#pragma once
#include <bfc/dispatch.h>
#include "JSAPI_Info.h"

namespace JSAPI2
{
	class api_security : public Dispatchable
	{
	public:
		enum 
		{
			ACTION_UNDEFINED = -1,
			ACTION_DISALLOWED = 0,
			ACTION_ALLOWED = 1,
			ACTION_PROMPT = 2,
		};

		enum
		{
			SUCCESS = 0,
			FAILURE = 1,
		};


		struct AuthorizationData
		{
			GUID data_id;
			void *data;
			size_t data_len;
		};

		// returns one of JSAPI2::api_security::ACTION_DISALLOWED, etc.
		int GetActionAuthorization(const wchar_t *group, const wchar_t *action, const wchar_t *authorization_key, JSAPI::ifc_info *info, int default_authorization = ACTION_ALLOWED, AuthorizationData *data = 0);

		// returns JSAPI2_SUCCESS, etc.
		int SetActionAuthorization(const wchar_t *group, const wchar_t *action, const wchar_t *authorization_key, int authorization);		

		// associates an HWND with a key.. used for prompting
		void Associate(const wchar_t *authorization_key, HWND hwnd);
		HWND GetAssociation(const wchar_t *authorization_key);

		// if you just want a simple security prompt, you can call this in your svc_apicreator::PromptForAuthorization implementation
		int SecurityPrompt(HWND hwnd, const wchar_t *title_string, const wchar_t *display_string, int flags=0);

		// associates a name (e.g. an online service name) with a key.. used for prompting
		void AssociateName(const wchar_t *authorization_key, const wchar_t *name);
		const wchar_t *GetAssociatedName(const wchar_t *authorization_key);

		// clears out all settings for a given key
		void ResetAuthorization(const wchar_t *authorization_key);

		void SetBypass(const wchar_t *authorization_key, bool enable_bypass);

		enum
		{
			JSAPI2_API_SECURITY_GETACTIONAUTHORIZATION = 0,
			JSAPI2_API_SECURITY_SETACTIONAUTHORIZATION = 1,
			JSAPI2_API_SECURITY_ASSOCIATE = 2,
			JSAPI2_API_SECURITY_GETASSOCIATION = 3,
			JSAPI2_API_SECURITY_SECURITYPROMPT = 4,
			JSAPI2_API_SECURITY_ASSOCIATENAME = 5,
			JSAPI2_API_SECURITY_GETASSOCIATEDNAME = 6,
			JSAPI2_API_SECURITY_RESETAUTHORIZATION = 7,
			JSAPI2_API_SECURITY_SETBYPASS = 8,
		};

	};

	inline int api_security::GetActionAuthorization(const wchar_t *group, const wchar_t *action, const wchar_t *authorization_key, JSAPI::ifc_info *info, int default_authorization, AuthorizationData *data)
	{
		return _call(JSAPI2_API_SECURITY_GETACTIONAUTHORIZATION, (int)default_authorization, group, action, authorization_key, info, default_authorization, data);
	}

	inline int api_security::SetActionAuthorization(const wchar_t *group, const wchar_t *action, const wchar_t *authorization_key, int authorization)
	{
		return _call(JSAPI2_API_SECURITY_SETACTIONAUTHORIZATION, (int)FAILURE, group, action, authorization_key, authorization);
	}

	inline void api_security::Associate(const wchar_t *authorization_key, HWND hwnd)
	{
		_voidcall(JSAPI2_API_SECURITY_ASSOCIATE, authorization_key, hwnd);
	}

	inline HWND api_security::GetAssociation(const wchar_t *authorization_key)
	{
		return _call(JSAPI2_API_SECURITY_GETASSOCIATION, (HWND)0, authorization_key);
	}

	inline int api_security::SecurityPrompt(HWND hwnd, const wchar_t *title_string, const wchar_t *display_string, int flags)
	{
		return _call(JSAPI2_API_SECURITY_SECURITYPROMPT, (int)0, hwnd, title_string, display_string, flags);
	}

	inline void api_security::AssociateName(const wchar_t *authorization_key, const wchar_t *name)
	{
		_voidcall(JSAPI2_API_SECURITY_ASSOCIATENAME, authorization_key, name);
	}

	inline const wchar_t *api_security::GetAssociatedName(const wchar_t *authorization_key)
	{
		return _call(JSAPI2_API_SECURITY_GETASSOCIATEDNAME, (const wchar_t *)0, authorization_key);
	}

	inline void api_security::ResetAuthorization(const wchar_t *authorization_key)
	{
		_voidcall(JSAPI2_API_SECURITY_RESETAUTHORIZATION, authorization_key);
	}
	inline void api_security::SetBypass(const wchar_t *authorization_key, bool enable_bypass)
	{
		_voidcall(JSAPI2_API_SECURITY_SETBYPASS, authorization_key, enable_bypass);
	}

	// {D8BA8766-E489-4cfc-8527-9F3206257FFC}
	static const GUID api_securityGUID = 
	{ 0xd8ba8766, 0xe489, 0x4cfc, { 0x85, 0x27, 0x9f, 0x32, 0x6, 0x25, 0x7f, 0xfc } };

};