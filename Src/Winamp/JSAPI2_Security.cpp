#include "api.h"
#include "JSAPI2_Security.h"
#include "JSAPI2_svc_apicreator.h"
#include <bfc/platform/types.h>
#include "main.h"
#include "resource.h"
#include "language.h"
#include <api/service/waservicefactory.h>
#include <shlwapi.h>
#include <strsafe.h>

JSAPI2::Security::~Security()
{
	for(NameMap::iterator iter = names.begin(); iter != names.end(); iter++)
	{
		if (NULL != iter->second)
			free(iter->second);
	}
}

int JSAPI2::Security::GetActionAuthorization( const wchar_t *group, const wchar_t *action, const wchar_t *authorization_key, JSAPI::ifc_info *info, int default_authorization, AuthorizationData *data )
{
	// TODO: benski> we should build a cache table, as we may hit this function repeatedly and it incurs a file lock
	// but for now it will get the ball rolling
	if ( action )
	{
		wchar_t group_action[ 256 ] = { 0 };
		StringCbPrintfW( group_action, sizeof( group_action ), L"%s@%s", action, group );
		int authorization = GetPrivateProfileIntW( authorization_key, group_action, api_security::ACTION_UNDEFINED, JSAPI2_INIFILE );
		if ( authorization != api_security::ACTION_UNDEFINED )
			return authorization;
	}
	if ( group )
	{
		int authorization = GetPrivateProfileIntW( authorization_key, group, api_security::ACTION_UNDEFINED, JSAPI2_INIFILE );
		if ( authorization != api_security::ACTION_UNDEFINED )
			return authorization;
	}

	int authorization = GetPrivateProfileIntW( authorization_key, authorization_key, default_authorization, JSAPI2_INIFILE );

	if ( ( ACTION_UNDEFINED == authorization || ACTION_PROMPT == authorization ) &&
		 false != IsAuthorizationBypassed( authorization_key ) )
	{
		authorization = ACTION_ALLOWED;
	}

	if ( ( authorization == ACTION_UNDEFINED || authorization == ACTION_PROMPT )/* if we have to prompt */
		 && default_authorization != ACTION_UNDEFINED  /* clients pass ACTION_UNDEFINED, API's don't */
		 && group )
	{
		waServiceFactory *sf = 0;
		int n = 0;
		do
		{
			sf = WASABI_API_SVC->service_enumService( JSAPI2::svc_apicreator::getServiceType(), n++ );
			if ( !sf )
				break;

			if ( sf )
			{
				JSAPI2::svc_apicreator *creator = (JSAPI2::svc_apicreator *)sf->getInterface();
				if ( creator )
				{
					HWND parent = 0;
					if ( info )
						parent = info->GetHWND();
					if ( !parent )
						parent = this->GetAssociation( authorization_key );

					int prompt = creator->PromptForAuthorization( parent, group, action, authorization_key, data );
					if ( ( prompt & JSAPI2::svc_apicreator::AUTHORIZATION_MASK ) != JSAPI2::svc_apicreator::AUTHORIZATION_UNDEFINED )
					{
						sf->releaseInterface( creator );
						int new_authorization = 0;
						switch ( prompt & JSAPI2::svc_apicreator::AUTHORIZATION_MASK )
						{
							case JSAPI2::svc_apicreator::AUTHORIZATION_ALLOW:
								new_authorization = ACTION_ALLOWED;
								break;
							case JSAPI2::svc_apicreator::AUTHORIZATION_DENY:
								new_authorization = ACTION_DISALLOWED;
								break;
							default:
								return default_authorization;
						}
						if ( prompt & JSAPI2::svc_apicreator::AUTHORIZATION_FLAG_GROUP_ONLY )
							action = 0;
						if ( prompt & JSAPI2::svc_apicreator::AUTHORIZATION_FLAG_ALWAYS )
							SetActionAuthorization( group, action, authorization_key, new_authorization );
						if ( prompt & JSAPI2::svc_apicreator::AUTHORIZATION_FLAG_ALWAYS_FOR_SERVICE )
							SetActionAuthorization( 0, 0, authorization_key, new_authorization );

						return new_authorization;
					}
				}

				sf->releaseInterface( creator );
			}
		} while ( sf );
	}

	return authorization;
}

int JSAPI2::Security::SetActionAuthorization(const wchar_t *group, const wchar_t *action, const wchar_t *authorization_key, int authorization)
{
	// TODO: benski> we should build a cache table, as we may hit this function repeatedly and it incurs a file lock
	// but for now it will get the ball rolling
	wchar_t intval[64] = {0};
	_itow(authorization, intval, 10);
	if (action)
	{
		wchar_t group_action[256] = {0};
		StringCbPrintfW(group_action, sizeof(group_action), L"%s@%s", action, group);
		WritePrivateProfileStringW(authorization_key, group_action, intval, JSAPI2_INIFILE);
	}
	else if (group)
		WritePrivateProfileStringW(authorization_key, group, intval, JSAPI2_INIFILE);
	else
		WritePrivateProfileStringW(authorization_key, authorization_key, intval, JSAPI2_INIFILE);

	return JSAPI2::api_security::SUCCESS;
}

void JSAPI2::Security::Associate(const wchar_t *authorization_key, HWND hwnd)
{
	unsigned long key;
	if (FALSE == StrToIntExW(authorization_key, STIF_DEFAULT,  (int*)&key))
		return;

	associations[key] = (void *)hwnd;
}

HWND JSAPI2::Security::GetAssociation(const wchar_t *authorization_key)
{
	unsigned long key;
	if (FALSE == StrToIntExW(authorization_key, STIF_DEFAULT,  (int*)&key))
		return NULL;

	AssociationMap::iterator iter = associations.find(key);
	return (HWND)((iter != associations.end()) ? iter->second : NULL);
}


INT_PTR JSAPI2_SecurityPrompt(HWND hParent, LPCWSTR pszCaption, LPCWSTR pszTitle, LPCWSTR pszMessage, UINT flags);

int JSAPI2::Security::SecurityPrompt(HWND parent, const wchar_t *title_string, const wchar_t *display_string, int flags)
{
	return (INT_PTR)JSAPI2_SecurityPrompt(parent, NULL, title_string, display_string, flags);
}

void JSAPI2::Security::AssociateName(const wchar_t *authorization_key, const wchar_t *name)
{
	unsigned long key;
	if (FALSE == StrToIntExW(authorization_key, STIF_DEFAULT,  (int*)&key))
		return;

	NameMap::iterator iter = names.find(key);
	if (NULL != name)
	{
		if (iter != names.end())
		{
			if (NULL != iter->second) free(iter->second);
			iter->second = _wcsdup(name);
		}
		else
		{
			names.insert({key, _wcsdup(name)});
		}
	}
	else
	{
		if (iter != names.end())
		{
			if (NULL != iter->second) 
			{
				free(iter->second);
				iter->second = NULL;
			}
			names.erase(iter);
		}
	}
}

const wchar_t *JSAPI2::Security::GetAssociatedName(const wchar_t *authorization_key)
{
	unsigned long key;
	if (FALSE == StrToIntExW(authorization_key, STIF_DEFAULT,  (int*)&key))
		return NULL;

	NameMap::iterator iter = names.find(key);
	return (iter != names.end()) ? iter->second : NULL;
}

void JSAPI2::Security::ResetAuthorization(const wchar_t *authorization_key)
{
	const wchar_t empty[2] = {0, 0};
	WritePrivateProfileSectionW(authorization_key, empty, JSAPI2_INIFILE);
}

void JSAPI2::Security::SetBypass(const wchar_t *authorization_key, bool enable_bypass)
{
	size_t index = bypassList.size();
	
	unsigned long key;
	if (FALSE == StrToIntExW(authorization_key, STIF_DEFAULT,  (int*)&key))
		return;

	while(index--)
	{
		if (bypassList[index] == key)
		{
			if (false == enable_bypass)
				bypassList.erase(bypassList.begin() + index);
			return;
		}
	}

	if (false != enable_bypass)
		bypassList.push_back(key);
}

bool JSAPI2::Security::IsAuthorizationBypassed(const wchar_t *authorization_key)
{
	size_t index = bypassList.size();
	if (0 == index) return false;

	unsigned long key;
	if (FALSE == StrToIntExW(authorization_key, STIF_DEFAULT,  (int*)&key))
		return false;

	while(index--)
	{
		if (bypassList[index] == key)
			return true;
	}
	return false;
}

JSAPI2::Security JSAPI2::security;

#define CBCLASS JSAPI2::Security
START_DISPATCH;
CB(JSAPI2_API_SECURITY_GETACTIONAUTHORIZATION, GetActionAuthorization);
CB(JSAPI2_API_SECURITY_SETACTIONAUTHORIZATION, SetActionAuthorization);
VCB(JSAPI2_API_SECURITY_ASSOCIATE, Associate);
CB(JSAPI2_API_SECURITY_GETASSOCIATION, GetAssociation);
CB(JSAPI2_API_SECURITY_SECURITYPROMPT, SecurityPrompt);
VCB(JSAPI2_API_SECURITY_ASSOCIATENAME, AssociateName);
CB(JSAPI2_API_SECURITY_GETASSOCIATEDNAME, GetAssociatedName);
VCB(JSAPI2_API_SECURITY_RESETAUTHORIZATION, ResetAuthorization);
VCB(JSAPI2_API_SECURITY_SETBYPASS, SetBypass);
END_DISPATCH;
#undef CBCLASS