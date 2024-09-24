#pragma once

#include <bfc/dispatch.h>
#include <time.h>

class ifc_authcallback;

enum 
{
	AUTHPARAM_EMPTY = 0,
	AUTHPARAM_TOOSHORT = 1,
	AUTHPARAM_TOOLONG = 2,
	AUTHPARAM_BADFORMAT = 3,
};

enum
{
	AUTH_SUCCESS = 0,
	AUTH_404 = 1,
	AUTH_TIMEOUT = 2,
	AUTH_NOHTTP = 3,
	AUTH_NOPARSER = 4,
	AUTH_CONNECTIONRESET = 5,
	AUTH_ERROR_PARSING_XML = 6,
	AUTH_NOT_AUTHORIZED = 7,
	AUTH_SECURID = 8,
	AUTH_ABORT = 10,
	AUTH_INVALIDCRED = 11,
	AUTH_UNCONFIRMED = 12,
	AUTH_UNEXPECTED = 13,  // unexpected catastrophic failure
	AUTH_INVALIDPASSCODE = 14,

	AUTH_USERNAME = 20,
	AUTH_USERNAME_EMPTY = (AUTH_USERNAME + AUTHPARAM_EMPTY),
	AUTH_USERNAME_TOOSHORT = (AUTH_USERNAME + AUTHPARAM_TOOSHORT),
	AUTH_USERNAME_TOOLONG = (AUTH_USERNAME + AUTHPARAM_TOOLONG),
	AUTH_USERNAME_BADFORMAT = (AUTH_USERNAME + AUTHPARAM_BADFORMAT),

	AUTH_PASSWORD = 30,
	AUTH_PASSWORD_EMPTY = (AUTH_PASSWORD + AUTHPARAM_EMPTY),
	AUTH_PASSWORD_TOOSHORT = (AUTH_PASSWORD + AUTHPARAM_TOOSHORT),
	AUTH_PASSWORD_TOOLONG = (AUTH_PASSWORD + AUTHPARAM_TOOLONG),
	AUTH_PASSWORD_BADFORMAT = (AUTH_PASSWORD + AUTHPARAM_BADFORMAT),

	AUTH_PASSCODE = 40,
	AUTH_PASSCODE_EMPTY = (AUTH_PASSCODE + AUTHPARAM_EMPTY),
	AUTH_PASSCODE_TOOSHORT = (AUTH_PASSCODE + AUTHPARAM_TOOSHORT),
	AUTH_PASSCODE_TOOLONG = (AUTH_PASSCODE + AUTHPARAM_TOOLONG),
	AUTH_PASSCODE_BADFORMAT = (AUTH_PASSCODE + AUTHPARAM_BADFORMAT),
	
	
};
// {392839D2-640D-4961-8E2A-1B5315C2F970}
static const GUID AuthApiGUID = 
{ 0x392839d2, 0x640d, 0x4961, { 0x8e, 0x2a, 0x1b, 0x53, 0x15, 0xc2, 0xf9, 0x70 } };

class api_auth : public Dispatchable
{
protected:
	api_auth() {}
	~api_auth() {}
public:
	
	struct AuthResults
	{
		// plx2b using SecureZeroMemory when you are done :)
		char session_key[256]; // seems to be strlen = ~200, but let's just be safe
		char token[256]; // always seems to be strlen=44, but let's just be safe
		__time64_t expire; // expiration time in absolute time [AOL's Auth API returns duration, but api_auth adds time()]
		char context[512]; // dunno exact length yet
		int statusCode;
		int statusDetailCode;
	};
	static const GUID getServiceGuid() { return AuthApiGUID; }
	int Login(const wchar_t *username, const wchar_t *password, AuthResults *results, ifc_authcallback *callback);
	int LoginSecurID(const wchar_t *username, const wchar_t *password, const char *context, const wchar_t *securid, AuthResults *results, ifc_authcallback *callback);
	// realm is just a "named account" kinda thing.  pass realm = GUID_NULL for "default" account, or your own unique name if you need to login with a separate ID from the rest of Winamp
	int SetCredentials(GUID realm, const char *session_key, const char *token, const wchar_t *username, __time64_t expire);
	int GetCredentials(GUID realm, char *session_key, size_t session_key_len, char *token, size_t token_len, wchar_t *username, size_t username_len, __time64_t *expire);
	const char *GetDevID();
	int ClientToWeb(GUID realm, const wchar_t *destination_url, wchar_t *url, size_t urlcch);

	HWND CreateLoginWindow(GUID realm, HWND owner, UINT style);
	INT_PTR LoginBox(GUID realm, HWND owner, UINT style);
	int GetUserName(GUID realm, wchar_t *username, size_t username_len);

	enum
	{
		API_AUTH_LOGIN = 0,
		API_AUTH_SETCREDENTIALS = 1,
		API_AUTH_GETCREDENTIALS = 2,
		API_AUTH_GETDEVID = 3,
		API_AUTH_LOGIN_SECURID = 4,
		API_AUTH_CLIENT_TO_WEB = 5,
		API_AUTH_CREATELOGINWINDOW = 6,
		API_AUTH_LOGINBOX = 7,
		API_AUTH_GETUSERNAME=8,
	};
};

inline int api_auth::Login(const wchar_t *username, const wchar_t *password, AuthResults *results, ifc_authcallback *callback)
{
	return _call(API_AUTH_LOGIN, (int)AUTH_NOHTTP, username, password, results, callback);
}

inline int api_auth::LoginSecurID(const wchar_t *username, const wchar_t *password, const char *context, const wchar_t *securid, AuthResults *results, ifc_authcallback *callback)
{
	return _call(API_AUTH_LOGIN_SECURID, (int)AUTH_NOHTTP, username, password, context, securid, results, callback);
}

inline int api_auth::SetCredentials(GUID realm, const char *session_key, const char *token, const wchar_t *username, __time64_t expire)
{
	return _call(API_AUTH_SETCREDENTIALS, (int)AUTH_NOT_AUTHORIZED, realm, session_key, token, username, expire);	
}

inline int api_auth::GetCredentials(GUID realm, char *session_key, size_t session_key_len, char *token, size_t token_len, wchar_t *username, size_t username_len, __time64_t *expire)
{
	return _call(API_AUTH_GETCREDENTIALS, (int)AUTH_NOT_AUTHORIZED, realm, session_key, session_key_len, token, token_len, username, username_len, expire);	
}

inline const char *api_auth::GetDevID()
{
	return _call(API_AUTH_GETDEVID, (const char *)0);
}

inline int api_auth::ClientToWeb(GUID realm, const wchar_t *destination_url, wchar_t *url, size_t urlcch)
{
	return _call(API_AUTH_CLIENT_TO_WEB, (int)1, realm, destination_url, url, urlcch);
}

inline HWND api_auth::CreateLoginWindow(GUID realm, HWND owner, UINT style)
{
	return _call(API_AUTH_CREATELOGINWINDOW, (HWND)NULL, realm, owner, style);
}

inline INT_PTR api_auth::LoginBox(GUID realm, HWND owner, UINT style)
{
	return _call(API_AUTH_LOGINBOX, (INT_PTR)-1, realm, owner, style);
}

inline int api_auth::GetUserName(GUID realm, wchar_t *username, size_t username_len)
{
	return _call(API_AUTH_GETUSERNAME, (int)AUTH_UNEXPECTED, realm, username, username_len);	
}