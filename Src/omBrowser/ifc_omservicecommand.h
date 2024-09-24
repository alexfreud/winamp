#ifndef NULLSOFT_WINAMP_OMSERVICE_COMMAND_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSERVICE_COMMAND_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {CA77030B-DF0F-4610-B01E-4A406A2FCD67}
static const GUID IFC_OmServiceCommand = 
{ 0xca77030b, 0xdf0f, 0x4610, { 0xb0, 0x1e, 0x4a, 0x40, 0x6a, 0x2f, 0xcd, 0x67 } };

// {8E4F3281-37CA-4cbe-8753-BA7531690D07}
static const GUID CMDGROUP_SERVICE = 
{ 0x8e4f3281, 0x37ca, 0x4cbe, { 0x87, 0x53, 0xba, 0x75, 0x31, 0x69, 0xd, 0x7 } };
#define SVCCOMMAND_SHOWINFO			1
#define SVCCOMMAND_REPORT			2
#define SVCCOMMAND_UNSUBSCRIBE		3
#define SVCCOMMAND_RATE				4		// commandArg = (UINT)ratingValue
#define SVCCOMMAND_BLOCKNAV			5		//used with query state 

// {FA32FE7D-848D-4813-8BDE-27E19CC835B5}
static const GUID CMDGROUP_NAVIGATION = 
{ 0xfa32fe7d, 0x848d, 0x4813, { 0x8b, 0xde, 0x27, 0xe1, 0x9c, 0xc8, 0x35, 0xb5 } };
#define NAVCOMMAND_BACKFORWARD		1		// arg = FALSE - backward, arg = TRUE - rofward
#define NAVCOMMAND_HISTORY			2
#define NAVCOMMAND_HOME				3
#define NAVCOMMAND_REFRESH			4		// arg = TRUE - full refresh
#define NAVCOMMAND_STOP				5


// {C04A2400-8D0D-4b3c-9271-0E042AF8C363}
static const GUID CMDGROUP_WINDOW = 
{ 0xc04a2400, 0x8d0d, 0x4b3c, { 0x92, 0x71, 0xe, 0x4, 0x2a, 0xf8, 0xc3, 0x63 } };
#define WNDCOMMAND_FULLSCREEN		1		//
#define WNDCOMMAND_CLOSE				2		// 


// {6A730612-A7CE-4a45-9898-2F0A807A3BB4}
static const GUID CMDGROUP_ADDRESSBAR = 
{ 0x6a730612, 0xa7ce, 0x4a45, { 0x98, 0x98, 0x2f, 0xa, 0x80, 0x7a, 0x3b, 0xb4 } };
#define ADDRESSCOMMAND_EXECUTE		1	// arg = address string
#define ADDRESSCOMMAND_VISIBLE		2	// used to check state
#define ADDRESSCOMMAND_READONLY		3	// used to check state

// QueryStatus() return values
#define CMDSTATE_ENABLED		S_OK
#define CMDSTATE_DISABLED		S_FALSE
#define CMDSTATE_UNKNOWN		E_NOINTERFACE

class __declspec(novtable) ifc_omservicecommand : public Dispatchable
{

protected:
	ifc_omservicecommand() {}
	~ifc_omservicecommand() {}

public:
	HRESULT QueryState(HWND hBrowser, const GUID *commandGroup, unsigned int  commandId); // return CMDSTATE_XXX or E_NOTIMPL if you ok with default
	HRESULT Exec(HWND hBrowser, const GUID *commandGroup, unsigned int  commandId, ULONG_PTR commandArg); // return SUCCEEDED() if command handled

public:
	DISPATCH_CODES
	{
		API_QUERYSTATE		= 10,
		API_EXEC			= 20,
	};
};

inline HRESULT ifc_omservicecommand::QueryState(HWND hBrowser, const GUID *commandGroup, unsigned int  commandId)
{
	return _call(API_QUERYSTATE, (HRESULT)E_NOTIMPL, hBrowser, commandGroup, commandId);
}

inline HRESULT ifc_omservicecommand::Exec(HWND hBrowser, const GUID *commandGroup, unsigned int  commandId, ULONG_PTR commandArg)
{
	return _call(API_EXEC, (HRESULT)E_NOTIMPL, hBrowser, commandGroup, commandId, commandArg);
}

#endif //NULLSOFT_WINAMP_OMSERVICE_COMMAND_INTERFACE_HEADER