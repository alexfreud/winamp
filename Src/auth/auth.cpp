#include "api.h"

#include "../nu/ns_wc.h"
#include <api/service/waservicefactory.h>
#include "OAuthKey.h"
#include "../nu/AutoCharFn.h"
#include "../nu/AutoChar.h"
#include "auth.h"
#include "./loginbox/loginbox.h"

#include "ifc_authcallback.h"
#include "main.h"
#include "../nu/AutoUrl.h"
#include <api/syscb/callbacks/authcb.h>

#include "../Winamp/buildType.h"
#include <strsafe.h>

//#ifdef INTERNAL
//// QA
////const char *openauth_url ="https://authapi.qa.aol.com:8439/auth/clientLogin";
//// QH
//const char *openauth_url ="https://authapi.qh.aol.com:6443/auth/clientLogin";
//#else
const char *openauth_url ="https://api.screenname.aol.com:443/auth/clientLogin";
//#endif

static void CleanPassword(char *password)
{
	char *src = password;
	char *dest = password;

	while (src && *src)
	{
		char c = *src++;
		if (c >= 'a' && c <= 'z')
			*dest++ = c;
		else if (c >= 'A' && c <= 'Z')
			*dest++ = c;
		else if (c >= '0' && c <= '9')
			*dest++ = c;
	}
	*dest=0;
}

static void ParsePassword(const wchar_t *password, char **password_url, char **securid=0)
{
	const wchar_t *find_slash = wcschr(password, L'/');
	if (find_slash)
	{
		*password_url = AutoUrlDupN(password, find_slash-password);
		if (securid)
			*securid = AutoUrlDup(find_slash+1);
	}
	else
	{
		*password_url = AutoUrlDup(password);
		if (securid)
			*securid=0;
	}
}

// TODO: benski> use &forceRateLimit=true to force captcha request
int PostXML(const char *url, const char *post_data, obj_xml *parser, ifc_authcallback *callback);
static int Authorize(obj_xml *parser, const wchar_t *username, const wchar_t *password, ifc_authcallback *callback)
{
	char *password_url, *securid;
	ParsePassword(password, &password_url, &securid);

	char post_data[2048] = {0};
	StringCbPrintfA(post_data, sizeof(post_data), 
		"devId=%s"
		"&f=xml"
		"&pwd=%s"
		"&s=%s"
		"%s%s"
		"&tokenType=longterm",
		OPENAUTH_DEVID,
		password_url,
		AutoUrl(username),
		(securid?"&securid=":""),
		(securid?securid:"")
		);

	free(password_url);
	free(securid);
	return PostXML(openauth_url, post_data, parser, callback);
}

static int AuthorizeSecurID(obj_xml *parser, const wchar_t *username, const char *context, const wchar_t *securid, ifc_authcallback *callback)
{
	char post_data[2048] = {0};
	StringCbPrintfA(post_data, sizeof(post_data), 
		"devId=%s"
		"&f=xml"		
		"&s=%s"
		"&context=%s"
		"&securid=%s"
		"&tokenType=longterm",
		OPENAUTH_DEVID,		
		AutoUrl(username),
		AutoUrl(context),
		AutoUrl(securid)
		);

	return PostXML(openauth_url, post_data, parser, callback);
}


void OpenAuthParser::RegisterCallbacks(obj_xml *parser)
{
	parser->xmlreader_registerCallback(L"response\fstatusCode", &statusCode);
	parser->xmlreader_registerCallback(L"response\fstatusText", &statusText);
	parser->xmlreader_registerCallback(L"response\fstatusDetailCode", &statusDetailCode);
	parser->xmlreader_registerCallback(L"response\fdata\ftoken\fa", &token);
	parser->xmlreader_registerCallback(L"response\fdata\ftoken\fexpiresIn", &expire);
	parser->xmlreader_registerCallback(L"response\fdata\fsessionSecret", &session_secret);
	parser->xmlreader_registerCallback(L"response\fdata\fchallenge\fcontext", &context);

}
void OpenAuthParser::UnregisterCallbacks(obj_xml *parser)
{
	parser->xmlreader_unregisterCallback(&statusCode);
	parser->xmlreader_unregisterCallback(&statusText);
	parser->xmlreader_unregisterCallback(&statusDetailCode);		
	parser->xmlreader_unregisterCallback(&token);
	parser->xmlreader_unregisterCallback(&expire);
	parser->xmlreader_unregisterCallback(&session_secret);
	parser->xmlreader_unregisterCallback(&context);
}


int Auth::SetupLogin(OpenAuthParser &authParser, waServiceFactory *&parserFactory, obj_xml *&parser)
{
	parserFactory = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
		parser = (obj_xml *)parserFactory->getInterface();

	if (parser)
	{
		parser->xmlreader_setCaseSensitive();
		authParser.RegisterCallbacks(parser);
		parser->xmlreader_open();
		return AUTH_SUCCESS;
	}
	else
		return AUTH_NOPARSER;
}

int Auth::ParseAuth(const wchar_t *password, OpenAuthParser &authParser, AuthResults *results)
{
	switch(authParser.statusCode.GetUInt32())
	{
	case 200:
		{
			char *password_url;
			ParsePassword(password, &password_url);
			UrlDecode(password_url);
			OAuthKey key(password_url, strlen(password_url));
			AutoChar session_utf8(authParser.session_secret.GetString(), CP_UTF8);
			key.FeedMessage((char *)session_utf8, strlen(session_utf8));
			key.EndMessage();
			key.GetBase64(results->session_key, sizeof(results->session_key));
			WideCharToMultiByteSZ(CP_UTF8, 0, authParser.token.GetString(), -1, results->token, sizeof(results->token), 0, 0);
			results->expire = authParser.expire.GetUInt32() + _time64(0);
			free(password_url);
			return AUTH_SUCCESS;
		}
	case 330:
		switch(authParser.statusDetailCode.GetUInt32())
		{
		case 3011: // Password-LoginId Required/Invalid
			return AUTH_INVALIDCRED;
		case 3012: // SecurId Required/Invalid 
		case 3013: // SecurId Next Token Required
			WideCharToMultiByteSZ(CP_UTF8, 0, authParser.context.GetString(), -1, results->context, sizeof(results->context), 0, 0);
			return AUTH_SECURID;
		}
		break;
	case 401:
		switch(authParser.statusDetailCode.GetUInt32())
		{
		case 3020:
			return AUTH_UNCONFIRMED;
		}
		break;
	}

	return AUTH_NOT_AUTHORIZED;
}

int Auth::Login(const wchar_t *username, const wchar_t *password, AuthResults *results, ifc_authcallback *callback)
{
	SecureZeroMemory(results, sizeof(AuthResults));
	OpenAuthParser authParser;
	obj_xml *parser = 0;
	waServiceFactory *parserFactory = 0;

	int err = SetupLogin(authParser, parserFactory, parser);
	if (err == AUTH_SUCCESS)
	{
		err = Authorize(parser, username, password, callback);
		authParser.UnregisterCallbacks(parser);
		parser->xmlreader_close();
		parserFactory->releaseInterface(parser);
		if (err != AUTH_SUCCESS)
			return err;

		return ParseAuth(password, authParser, results);
	}
	else
		return err;

	return AUTH_NOT_AUTHORIZED;
}

int Auth::LoginSecurID(const wchar_t *username, const wchar_t *password, const char *context, const wchar_t *securid, AuthResults *results, ifc_authcallback *callback)
{
	SecureZeroMemory(results, sizeof(AuthResults));
	OpenAuthParser authParser;
	obj_xml *parser = 0;
	waServiceFactory *parserFactory = 0;

	int err = SetupLogin(authParser, parserFactory, parser);
	if (err == AUTH_SUCCESS)
	{
		err = AuthorizeSecurID(parser, username, context, securid, callback);
		authParser.UnregisterCallbacks(parser);
		parser->xmlreader_close();
		parserFactory->releaseInterface(parser);
		if (err != AUTH_SUCCESS)
			return err;

		return ParseAuth(password, authParser, results);
	}
	else
		return err;

	return AUTH_NOT_AUTHORIZED;
}

const char *Auth::GetDevID()
{
	return OPENAUTH_DEVID;
}

int Auth::SetCredentials(GUID realm, const char *session_key, const char *token, const wchar_t *username, __time64_t expire)
{
	if (NULL != WASABI_API_SYSCB)
		WASABI_API_SYSCB->syscb_issueCallback(SysCallback::AUTH, AuthCallback::CREDENTIALS_ABOUTTOCHANGE, (intptr_t)this, (intptr_t)&realm);

	char guid_str[40] = {0};
	if (realm != GUID_NULL)
	{
	  StringCbPrintfA( guid_str, sizeof(guid_str), "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
    (int)realm.Data1, (int)realm.Data2, (int)realm.Data3,
    (int)realm.Data4[0], (int)realm.Data4[1],
    (int)realm.Data4[2], (int)realm.Data4[3],
    (int)realm.Data4[4], (int)realm.Data4[5],
    (int)realm.Data4[6], (int)realm.Data4[7] );
	}
	else
	{
		StringCbCopyA(guid_str, sizeof(guid_str), "default");
	}

	if (session_key && session_key[0])
	{
		WritePrivateProfileStringA(guid_str, "session_key", session_key, inifile);
		WritePrivateProfileStringA(guid_str, "token", token, inifile);
		WritePrivateProfileStringA(guid_str, "username", AutoChar(username, CP_UTF8), inifile);
		char temp[128] = {0};
		StringCbPrintfA(temp, sizeof(temp), "%I64d", expire);
		WritePrivateProfileStringA(guid_str, "expiration", temp, inifile);
	}
	else
	{
		char empty[2] = {0,0};
		WritePrivateProfileSectionA(guid_str, empty, inifile);
		if (username && username[0]) // they might want to save their username tho
			WritePrivateProfileStringA(guid_str, "username", AutoChar(username, CP_UTF8), inifile);
	}

	if (NULL != WASABI_API_SYSCB)
		WASABI_API_SYSCB->syscb_issueCallback(SysCallback::AUTH, AuthCallback::CREDENTIALS_CHANGED, (intptr_t)this, (intptr_t)&realm);

	return AUTH_SUCCESS;
}

int Auth::GetCredentials(GUID realm, char *session_key, size_t session_key_len, char *token, size_t token_len, wchar_t *username, size_t username_len, __time64_t *expire)
{
	char guid_str[40] = {0};
	if (realm != GUID_NULL)
	{
	  StringCbPrintfA( guid_str, sizeof(guid_str), "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
    (int)realm.Data1, (int)realm.Data2, (int)realm.Data3,
    (int)realm.Data4[0], (int)realm.Data4[1],
    (int)realm.Data4[2], (int)realm.Data4[3],
    (int)realm.Data4[4], (int)realm.Data4[5],
    (int)realm.Data4[6], (int)realm.Data4[7] );
	}
	else
	{
		StringCbCopyA(guid_str, sizeof(guid_str), "default");
	}

	GetPrivateProfileStringA(guid_str, "session_key", "", session_key, (DWORD)session_key_len, inifile);
	GetPrivateProfileStringA(guid_str, "token", "", token, (DWORD)token_len, inifile);
		char temp[1024] = {0};
	GetPrivateProfileStringA(guid_str, "username", "", temp, sizeof(temp), inifile);
	MultiByteToWideCharSZ(CP_UTF8, 0, temp, -1, username, (DWORD)username_len);
	
	GetPrivateProfileStringA(guid_str, "expiration", "", temp, sizeof(temp), inifile);
	*expire = _atoi64(temp);
	return AUTH_SUCCESS;
}

int Auth::GetUserName(GUID realm, wchar_t *username, size_t username_len)
{
	char guid_str[40] = {0};
	if (realm != GUID_NULL)
	{
	  StringCbPrintfA( guid_str, sizeof(guid_str), "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
    (int)realm.Data1, (int)realm.Data2, (int)realm.Data3,
    (int)realm.Data4[0], (int)realm.Data4[1],
    (int)realm.Data4[2], (int)realm.Data4[3],
    (int)realm.Data4[4], (int)realm.Data4[5],
    (int)realm.Data4[6], (int)realm.Data4[7] );
	}
	else
	{
		StringCbCopyA(guid_str, sizeof(guid_str), "default");
	}

	char temp[1024] = {0};
	GetPrivateProfileStringA(guid_str, "username", "", temp, sizeof(temp), inifile);
	MultiByteToWideCharSZ(CP_UTF8, 0, temp, -1, username, (DWORD)username_len);
	return AUTH_SUCCESS;
}


Auth::Auth()
{
	inifile=0;
}
void Auth::Init()
{
	wchar_t inifileW[MAX_PATH] = {0};
	const wchar_t *settings_path = WASABI_API_APP->path_getUserSettingsPath();
	PathCombineW(inifileW, settings_path, L"auth.ini");
	inifile = _strdup(AutoCharFn(inifileW));
}

void Auth::Quit()
{
	free(inifile);
}

static void AddParameter(char *&position, size_t &len, const char *param, const wchar_t *val)
{
	AutoUrl encoded_val(val);
	StringCchPrintfExA(position, len, &position, &len, 0, "&%s=%s", param, encoded_val);
}

static void AddParameter(char *&position, size_t &len, const char *param, const char *val)
{
	AutoUrl encoded_val(val);
	StringCchPrintfExA(position, len, &position, &len, 0, "&%s=%s", param, encoded_val);
}

static void AddParameter(char *&position, size_t &len, const char *param, int64_t val)
{
	char temp[64] = {0};
	StringCchPrintfA(temp, 64, "%I64d", val);
	StringCchPrintfExA(position, len, &position, &len, 0, "&%s=%s", param, temp);
}

//#ifdef INTERNAL
// QA
//static const char *c2w_server="my.screenname.qa.aol.com";
//static const char *c2w_path="/_cqr/login/login.psp";
//static const char *c2w_path_encoded="%2F_cqr%2Flogin%2Flogin.psp";
// QH
//static const char *c2w_server="my.screenname.qh.aol.com";
//static const char *c2w_path="/_cqr/login/login.psp";
//static const char *c2w_path_encoded="%2F_cqr%2Flogin%2Flogin.psp";
//#else
static const char *c2w_server="my.screenname.aol.com";
static const char *c2w_path="/_cqr/login/login.psp";
static const char *c2w_path_encoded="%2F_cqr%2Flogin%2Flogin.psp";
//#endif


int Auth::ClientToWeb(GUID realm, const wchar_t *destination_url, wchar_t *url, size_t urlcch)
{
	char session_key[1024], token[1024] = {0};
	wchar_t username[1024] = {0};
	__time64_t expire;
	int ret = GetCredentials(realm, session_key, 1024, token, 1024, username, 1024, &expire);
	if (ret)
		return ret;

	if (!session_key[0] || !token[0] || !username[0])
		return 1;

	char post_data[2048]="";
	char *post_itr=post_data;
	size_t post_cch=sizeof(post_data)/sizeof(*post_data);

	OAuthKey key(session_key, strlen(session_key));

	key.FeedMessage("GET&", 4);
	key.FeedMessage("http%3A%2F%2F", 13);
	key.FeedMessage(c2w_server, strlen(c2w_server));
	key.FeedMessage(c2w_path_encoded, strlen(c2w_path_encoded));
	key.FeedMessage("&", 1);

	// parameters
	StringCbPrintfExA(post_itr, post_cch, &post_itr, &post_cch, 0, "a=%s", AutoUrl(token));
	char *start = post_itr;
	key.FeedMessage("a%3D", 4);
	AutoUrl token_a_url1(token);
	AutoUrl token_a_url((char *)token_a_url1);
	key.FeedMessage(token_a_url, strlen((char *)token_a_url));

	AddParameter(post_itr, post_cch, "destUrl", destination_url);
	AddParameter(post_itr, post_cch, "devId", GetDevID());
	AddParameter(post_itr, post_cch, "entryType", L"client2Web");
		__time64_t t = _time64(0);
	AddParameter(post_itr, post_cch, "ts", t);

	AutoUrl encoded_post(start);
	key.FeedMessage((char *)encoded_post, strlen(encoded_post));

	key.EndMessage();
	char hash[512] = {0};
	key.GetBase64(hash, 512);

	StringCchPrintfA(post_itr, post_cch, "&sig_sha256=%s", AutoUrl(hash));

	char urla[2048] = {0};
	StringCbPrintfA(urla, sizeof(urla), "http://%s%s?%s", c2w_server, c2w_path, post_data);

	MultiByteToWideCharSZ(CP_UTF8, 0, urla, -1, url, (int)urlcch);

	return 0;
}

HWND Auth::CreateLoginWindow(GUID realm, HWND owner, UINT style)
{
#ifndef USE_LOGINBOX
	return NULL;
#else
	return LoginBox_CreateWindow(this, &realm, owner, style);
#endif
}

INT_PTR Auth::LoginBox(GUID realm, HWND owner, UINT style)
{
#ifndef USE_LOGINBOX
	return -1;
#else
	return LoginBox_Show(this, &realm, owner, style);
#endif
}

#define CBCLASS Auth
START_DISPATCH;
CB(API_AUTH_LOGIN, Login)
CB(API_AUTH_GETDEVID, GetDevID)
CB(API_AUTH_LOGIN_SECURID, LoginSecurID)
CB(API_AUTH_SETCREDENTIALS, SetCredentials)
CB(API_AUTH_GETCREDENTIALS, GetCredentials)
CB(API_AUTH_CLIENT_TO_WEB, ClientToWeb)
CB(API_AUTH_CREATELOGINWINDOW, CreateLoginWindow)
CB(API_AUTH_LOGINBOX, LoginBox)
CB(API_AUTH_GETUSERNAME, GetUserName)
END_DISPATCH;
#undef CBCLASS