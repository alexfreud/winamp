#pragma once

#include "api_urlmanager.h"
#include <vector>

class URLManager : public api_urlmanager
{
public:
	static const char *getServiceName() { return "URL Manager API"; }
	static const GUID getServiceGuid() { return urlManagerGUID; }
public:
	const wchar_t *GetURL(const wchar_t *urlid);
//	int Parse(const wchar_t *filename);

private:
	void AddURL(const wchar_t *urlid, const wchar_t *url);
	struct URLS
	{
		wchar_t *urlid;
		wchar_t *url;
	};
	typedef std::vector<URLS> URLList;
	URLList urls;

protected:
	RECVS_DISPATCH;
};

