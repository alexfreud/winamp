#ifndef NULLSOFT_IFC_PLAYLISTDIRECTORYCALLBACK_H
#define NULLSOFT_IFC_PLAYLISTDIRECTORYCALLBACK_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>

class ifc_playlistdirectorycallback : public Dispatchable
{
protected:
	ifc_playlistdirectorycallback()                                   {}
	~ifc_playlistdirectorycallback()                                  {}

public:
	bool ShouldRecurse(const wchar_t *path);
	bool ShouldLoad(const wchar_t *filename);

	DISPATCH_CODES
	{
		IFC_PLAYLISTDIRECTORYCALLBACK_SHOULDRECURSE = 10,
		IFC_PLAYLISTDIRECTORYCALLBACK_SHOULDLOAD    = 20,
	};
};

inline bool ifc_playlistdirectorycallback::ShouldRecurse(const wchar_t *path)
{
	return _call(IFC_PLAYLISTDIRECTORYCALLBACK_SHOULDRECURSE, (bool)false, path);
}

inline bool ifc_playlistdirectorycallback::ShouldLoad(const wchar_t *filename)
{
	return _call(IFC_PLAYLISTDIRECTORYCALLBACK_SHOULDLOAD, (bool)true, filename);
}
#endif