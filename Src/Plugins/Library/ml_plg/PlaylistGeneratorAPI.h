#include "api_playlist_generator.h"

class PlaylistGeneratorAPI : public api_playlist_generator
{
public:
	// Exposed API functions
	int GeneratePlaylist(HWND parent, const itemRecordListW *selectedSeedRecordList);
	
	// Helper functions
	int AddSeedTracks(const itemRecordListW *recordList);
	
protected:
	RECVS_DISPATCH;
};
