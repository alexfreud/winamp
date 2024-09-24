#pragma once

#include <bfc/dispatch.h>
#include <bfc/platform/guid.h>

class api_urlmanager : public Dispatchable
{
protected:
	api_urlmanager(){}
	~api_urlmanager(){}
public:
	const wchar_t *GetURL(const wchar_t *urlid);

	enum
	{
		API_URLMANAGER_GETURL=0,
	};
};

inline const wchar_t *api_urlmanager::GetURL(const wchar_t *urlid)
{
	return _call(API_URLMANAGER_GETURL, (const wchar_t *)0, urlid);
}

// {B5E9E32E-4C4A-49d6-804F-8858B396F27E}
static const GUID urlManagerGUID = 
{ 0xb5e9e32e, 0x4c4a, 0x49d6, { 0x80, 0x4f, 0x88, 0x58, 0xb3, 0x96, 0xf2, 0x7e } };
