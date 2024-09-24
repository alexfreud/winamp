#include "api_auth.h"
#include <api/service/services.h>
#include "../nu/PtrMap.h"
#include "XMLString.h"
#include "../xml/obj_xml.h"

class OpenAuthParser
{
public:
	void RegisterCallbacks(obj_xml *parser);
	void UnregisterCallbacks(obj_xml *parser);
	XMLString statusCode, statusText, statusDetailCode;
	XMLString token;
	XMLString expire;
	XMLString session_secret;
	XMLString context;
};


class Auth : public api_auth
{
public:
	static const char *getServiceName() { return "Authorization API"; }
	static FOURCC getServiceType() { return WaSvc::UNIQUE; } 
	void Init();
	void Quit();
	Auth();
	int Login(const wchar_t *username, const wchar_t *password, AuthResults *results, ifc_authcallback *callback);
	int LoginSecurID(const wchar_t *username, const wchar_t *password, const char *context, const wchar_t *securid, AuthResults *results, ifc_authcallback *callback);
	const char *GetDevID();
	int SetCredentials(GUID realm, const char *session_key, const char *token, const wchar_t *username, __time64_t expire);
	int GetCredentials(GUID realm, char *session_key, size_t session_key_len, char *token, size_t token_len, wchar_t *username, size_t username_len, __time64_t *expire);
	int ClientToWeb(GUID realm, const wchar_t *destination_url, wchar_t *url, size_t urlcch);

	HWND CreateLoginWindow(GUID realm, HWND owner, UINT style);
	INT_PTR LoginBox(GUID realm, HWND owner, UINT style);
	int GetUserName(GUID realm, wchar_t *username, size_t username_len);

private:
	static int SetupLogin(OpenAuthParser &authParser, waServiceFactory *&parserFactory, obj_xml *&parser);
	static int ParseAuth(const wchar_t *password, OpenAuthParser &authParser, AuthResults *results);

	char *inifile;
protected:
	RECVS_DISPATCH;
};
