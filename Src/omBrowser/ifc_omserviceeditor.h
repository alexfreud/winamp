#ifndef NULLSOFT_WINAMP_OMSERVICE_EDITOR_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSERVICE_EDITOR_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {D0EC78E6-3115-4171-BD99-C500CC66DDAC}
static const GUID IFC_OmServiceEditor = 
{ 0xd0ec78e6, 0x3115, 0x4171, { 0xbd, 0x99, 0xc5, 0x0, 0xcc, 0x66, 0xdd, 0xac } };

#include <bfc/dispatch.h>

class __declspec(novtable) ifc_omserviceeditor : public Dispatchable
{
public:
	typedef enum
	{
		modifiedName			= 0x00000001,
		modifiedUrl			= 0x00000002,
		modifiedIcon			= 0x00000004,
		modifiedRating		= 0x00000008,
		modifiedVersion		= 0x00000010,
		modifiedFlags		= 0x00000020,
		modifiedDescription = 0x00000040,
		modifiedAuthorFirst	= 0x00000080,
		modifiedAuthorLast	= 0x00000100,
		modifiedUpdated		= 0x00000200,
		modifiedPublished	= 0x00000400,
		modifiedThumbnail	= 0x00000800,
		modifiedScreenshot	= 0x00001000,
		modifiedGeneration	= 0x00002000,
	} ModifiedFlags;

protected:
	ifc_omserviceeditor() {}
	~ifc_omserviceeditor() {}

public:
	HRESULT SetName(const wchar_t *name, BOOL utf8);
	HRESULT SetUrl(const wchar_t *url, BOOL utf8);
	HRESULT SetIcon(const wchar_t *imagePath, BOOL utf8);
	HRESULT SetRating(unsigned int rating);
	HRESULT SetVersion(unsigned int version);
	HRESULT SetFlags(unsigned int flags, unsigned int flagMask);
	HRESULT SetDescription(const wchar_t *description, BOOL utf8);
	HRESULT SetAuthorFirst(const wchar_t *authorName, BOOL utf8);
	HRESULT SetAuthorLast(const wchar_t *authorName, BOOL utf8);
	HRESULT SetUpdated(const wchar_t *date, BOOL utf8);
	HRESULT SetPublished(const wchar_t *date, BOOL utf8);
	HRESULT SetThumbnail(const wchar_t *imagePath, BOOL utf8);
	HRESULT SetScreenshot(const wchar_t *imagePath, BOOL utf8);
	HRESULT SetGeneration(unsigned int generation);

	HRESULT SetModified(unsigned int modifiedFlag, unsigned int modifiedMask);
	HRESULT GetModified(unsigned int *modifiedFlags);

	HRESULT BeginUpdate();
	HRESULT EndUpdate();

public:
	DISPATCH_CODES
	{
		API_SETNAME			= 10,
		API_SETURL			= 20,
		API_SETICON			= 30,
		API_SETRATING		= 40,
		API_SETVERSION		= 50,
		API_SETFLAGS		= 60,
		API_SETDESCRIPTION	= 70,
		API_SETAUTHORFIRST	= 80,
		API_SETAUTHORLAST	= 90,
		API_SETUPDATED		= 100,
		API_SETPUBLISHED	= 110,
		API_SETTHUMBNAIL		= 120,
		API_SETSCREENSHOT	= 130,
		API_SETGENERATION	= 140,

		API_SETMODIFIED		= 400,
		API_GETMODIFIED		= 410,
		API_BEGINUPDATE		= 420,
		API_ENDUPDATE		= 430,
	};
};


inline HRESULT ifc_omserviceeditor::SetName(const wchar_t *name, BOOL utf8)
{
	return _call(API_SETNAME, (HRESULT)E_NOTIMPL, name, utf8);
}

inline HRESULT ifc_omserviceeditor::SetUrl(const wchar_t *url, BOOL utf8)
{
	return _call(API_SETURL, (HRESULT)E_NOTIMPL, url, utf8);
}

inline HRESULT ifc_omserviceeditor::SetIcon(const wchar_t *imagePath, BOOL utf8)
{
	return _call(API_SETICON, (HRESULT)E_NOTIMPL, imagePath, utf8);
}

inline HRESULT ifc_omserviceeditor::SetRating(unsigned int rating)
{
	return _call(API_SETRATING, (HRESULT)E_NOTIMPL, rating);
}

inline HRESULT ifc_omserviceeditor::SetVersion(unsigned int version)
{
	return _call(API_SETVERSION, (HRESULT)E_NOTIMPL, version);
}

inline HRESULT ifc_omserviceeditor::SetFlags(unsigned int flags, unsigned int flagMask)
{
	return _call(API_SETFLAGS, (HRESULT)E_NOTIMPL, flags, flagMask);
}

inline HRESULT ifc_omserviceeditor::SetDescription(const wchar_t *description, BOOL utf8)
{
	return _call(API_SETDESCRIPTION, (HRESULT)E_NOTIMPL, description, utf8);
}

inline HRESULT ifc_omserviceeditor::SetAuthorFirst(const wchar_t *authorName, BOOL utf8)
{
	return _call(API_SETAUTHORFIRST, (HRESULT)E_NOTIMPL, authorName, utf8);
}

inline HRESULT ifc_omserviceeditor::SetAuthorLast(const wchar_t *authorName, BOOL utf8)
{
	return _call(API_SETAUTHORLAST, (HRESULT)E_NOTIMPL, authorName, utf8);
}

inline HRESULT ifc_omserviceeditor::SetUpdated(const wchar_t *date, BOOL utf8)
{
	return _call(API_SETUPDATED, (HRESULT)E_NOTIMPL, date, utf8);
}

inline HRESULT ifc_omserviceeditor::SetPublished(const wchar_t *date, BOOL utf8)
{
	return _call(API_SETPUBLISHED, (HRESULT)E_NOTIMPL, date, utf8);
}

inline HRESULT ifc_omserviceeditor::SetThumbnail(const wchar_t *imagePath, BOOL utf8)
{
	return _call(API_SETTHUMBNAIL, (HRESULT)E_NOTIMPL, imagePath, utf8);
}

inline HRESULT ifc_omserviceeditor::SetScreenshot(const wchar_t *imagePath, BOOL utf8)
{
	return _call(API_SETSCREENSHOT, (HRESULT)E_NOTIMPL, imagePath, utf8);
}

inline HRESULT ifc_omserviceeditor::SetModified(unsigned int modifiedFlag, unsigned int modifiedMask)
{
	return _call(API_SETMODIFIED, (HRESULT)E_NOTIMPL, modifiedFlag, modifiedMask);
}

inline HRESULT ifc_omserviceeditor::GetModified(unsigned int *modifiedFlags)
{
	return _call(API_GETMODIFIED, (HRESULT)E_NOTIMPL, modifiedFlags);
}

inline HRESULT ifc_omserviceeditor::BeginUpdate()
{
	return _call(API_BEGINUPDATE, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_omserviceeditor::EndUpdate()
{
	return _call(API_ENDUPDATE, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_omserviceeditor::SetGeneration(unsigned int generation)
{
	return _call(API_SETGENERATION, (HRESULT)E_NOTIMPL, generation);
}

#endif //NULLSOFT_WINAMP_OMSERVICE_EDITOR_INTERFACE_HEADER