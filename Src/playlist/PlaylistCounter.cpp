#include "PlaylistCounter.h"

int PlaylistCounter::OnFile( const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info )
{
	// TODO: recursive load?
	++count;
	if ( lengthInMS > 0 )
		length += lengthInMS;

	return LOAD_CONTINUE;
}


#define CBCLASS PlaylistCounter
START_DISPATCH;
CB( IFC_PLAYLISTLOADERCALLBACK_ONFILE_RET, OnFile )
END_DISPATCH;
#undef CBCLASS