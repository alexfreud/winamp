#pragma once
#include "ifc_playlistloadercallback.h"

template <class T>
class ifc_playlistloadercallbackT : public ifc_playlistloadercallback
{
protected:
	ifc_playlistloadercallbackT() {}
	~ifc_playlistloadercallbackT() {}
protected:
	// return 0 to continue enumeration, or 1 to quit

	// title will be NULL if no title found, length will be -1
	int OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info) {  return LOAD_ABORT; }
	// numEntries is just a hint, there is no gaurantee.  0 means "don't know"
	int OnPlaylistInfo(const wchar_t *playlistName, size_t numEntries, ifc_plentryinfo *info) { return LOAD_ABORT; }
	 // return 0 to use playlist file path as base (or just don't implement)
	const wchar_t *GetBasePath() { return 0; }

#define CBCLASS T
#define CBCLASST ifc_playlistloadercallbackT<T>
	START_DISPATCH_INLINE;
	CBT(IFC_PLAYLISTLOADERCALLBACK_ONFILE_RET, OnFile);
	CBT(IFC_PLAYLISTLOADERCALLBACK_ONPLAYLISTINFO_RET, OnPlaylistInfo);
	CBT(IFC_PLAYLISTLOADERCALLBACK_GETBASEPATH, GetBasePath);
	END_DISPATCH;
#undef CBCLASS
#undef CBCLASST
};
