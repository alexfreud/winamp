#include "./androiddevice.h"
#include "./AndroidPlaylist.h"

#include <shlwapi.h>
#include <strsafe.h>

// dtor
// cleanup the memory allocated for the vector of songs
AndroidPlaylist::~AndroidPlaylist() 
{

}

// this is the constructor that gets called
AndroidPlaylist::AndroidPlaylist(AndroidDevice& d, LPCTSTR fileName, BOOL m) 
: device(d), master(m), dirty(false)
{
	StringCbCopyW(filename, sizeof(filename), fileName);
	StringCbCopyW(playlistName, sizeof(playlistName), PathFindFileName(fileName));
	StringCbCopyW(playlistPath, sizeof(playlistName), fileName);
	PathRemoveFileSpec(playlistPath);
}


void AndroidPlaylist::OnFile( const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info )
{
	if ( filename == NULL )
		return;

	AndroidSong *song = NULL;
	//Nullsoft::Utility::AutoLock songs_lock(songs_guard);
	song = device.findSongInMasterPlaylist( filename );
	songs.push_back( song );
}

size_t AndroidPlaylist::size()
{
	//Nullsoft::Utility::AutoLock songs_lock(songs_guard);
	return songs.size();
}

AndroidSong *&AndroidPlaylist::at(size_t index)
{
	//Nullsoft::Utility::AutoLock songs_lock(songs_guard);
	return songs.at(index);
}

void AndroidPlaylist::push_back(AndroidSong *callback)
{
	//Nullsoft::Utility::AutoLock songs_lock(songs_guard);
	songs.push_back(callback);
	dirty=TRUE;
}

void AndroidPlaylist::RemoveSong(AndroidSong *song)
{
	//Nullsoft::Utility::AutoLock songs_lock(songs_guard);
	size_t old_size = songs.size();

	//songs.eraseAll(song);
	auto it = songs.begin();
	while (it != songs.end())
	{
		if (*it != song)
		{
			it++;
			continue;
		}

		it = songs.erase(it);
	}
	
	if (old_size != songs.size())
		dirty=TRUE;
}

void AndroidPlaylist::swap(size_t index1, size_t index2)
{
	//Nullsoft::Utility::AutoLock songs_lock(songs_guard);
	AndroidSong *temp = songs[index1];
	songs[index1] = songs[index2];
	songs[index2] = temp;
	dirty = true;
}

void AndroidPlaylist::eraseAt(size_t index)
{
//	Nullsoft::Utility::AutoLock songs_lock(songs_guard);
	songs.erase(songs.begin() + index);
	dirty=true;
}

static int filenamecmp( const wchar_t *f1, const wchar_t *f2 )
{
	for ( ;;)
	{
		wchar_t c1 = *f1++;
		wchar_t c2 = *f2++;
		if ( !c1 && !c2 )
			return 0;
		if ( !c1 )
			return -1;
		if ( !c2 )
			return 1;
		c1 = towupper( c1 );
		c2 = towupper( c2 );
		if ( c1 == '\\' )
			c1 = '/';
		if ( c2 == '\\' )
			c2 = '/';
		if ( c1 < c2 )
			return -1;
		else if ( c1 > c2 )
			return 1;
	}
}

AndroidSong *AndroidPlaylist::FindSong(const wchar_t *filename)
{
	//Nullsoft::Utility::AutoLock songs_lock(songs_guard);
	for (SongList::iterator e = songs.begin(); e != songs.end(); e++)
	{
		if (filenamecmp(filename, (*e)->filename) == 0)
		{
			return (*e);
		}
	}
	return 0;
}

#define CBCLASS AndroidPlaylist
START_DISPATCH;
VCB(IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile)
END_DISPATCH;