#pragma once
#include "JSAPI2_api_security.h"
#include <bfc/platform/types.h>
#include <map>
#include <vector>
#include "JSAPI_Info.h"

namespace JSAPI2
{
	class Security : public JSAPI2::api_security
	{
	public:
		~Security();
		static const char *getServiceName() { return "JSAPI2 Security API"; }
		static const GUID getServiceGuid() { return api_securityGUID; }	
		int GetActionAuthorization(const wchar_t *group, const wchar_t *action, const wchar_t *authorization_key, JSAPI::ifc_info *info, int default_authorization = ACTION_UNDEFINED, AuthorizationData *data = 0);
		int SetActionAuthorization(const wchar_t *group, const wchar_t *action, const wchar_t *authorization_key, int authorization);	
		void Associate(const wchar_t *authorization_key, HWND hwnd);
		HWND GetAssociation(const wchar_t *authorization_key);
		int SecurityPrompt(HWND hwnd, const wchar_t *title_string, const wchar_t *display_string, int flags);
		void AssociateName(const wchar_t *authorization_key, const wchar_t *name);
		const wchar_t *GetAssociatedName(const wchar_t *authorization_key);
		void ResetAuthorization(const wchar_t *authorization_key);
		void SetBypass(const wchar_t *authorization_key, bool enable_bypass);

	protected:
		bool IsAuthorizationBypassed(const wchar_t *authorization_key);

	protected:
		RECVS_DISPATCH;


	private:
		typedef std::map<uint32_t, wchar_t*> NameMap;
		typedef std::map<uint32_t, void*> AssociationMap;
		typedef std::vector<uint32_t> BypassList;
		AssociationMap associations;
		NameMap names;
		BypassList bypassList;
	};
    
	extern Security security;
}