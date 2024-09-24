#ifndef NULLSOFT_API_EXPLORERFINDFILE_H
#define NULLSOFT_API_EXPLORERFINDFILE_H

#include <bfc/dispatch.h>

class api_explorerfindfile : public Dispatchable
{
protected:
	api_explorerfindfile() {}
	~api_explorerfindfile() {}
public:
	BOOL AddFile(wchar_t* file);
	BOOL ShowFiles(void);
	void Reset(void);
public:
	DISPATCH_CODES
	{
		API_EXPLORERFINDFILE_ADDFILE = 10,
		API_EXPLORERFINDFILE_SHOWFILES = 11,
		API_EXPLORERFINDFILE_RESET = 12,
	};
};

inline BOOL api_explorerfindfile::AddFile(wchar_t* file)
{
	return _call(API_EXPLORERFINDFILE_ADDFILE, (BOOL)0, file);
}

inline BOOL api_explorerfindfile::ShowFiles(void)
{
	return _call(API_EXPLORERFINDFILE_SHOWFILES, (BOOL)0);
}

inline void api_explorerfindfile::Reset(void)
{
	_voidcall(API_EXPLORERFINDFILE_RESET);
}

extern api_explorerfindfile *ExplorerFindFileManager;
#define WASABI_API_EXPLORERFINDFILE ExplorerFindFileManager

// {83D6CD21-D67A-4326-A5B2-E1EFD664ADB5}
static const GUID ExplorerFindFileApiGUID = 
{ 0x83d6cd21, 0xd67a, 0x4326, { 0xa5, 0xb2, 0xe1, 0xef, 0xd6, 0x64, 0xad, 0xb5 } };

#endif