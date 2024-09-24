#include "iPodSD.h"
#include <math.h>
#include <assert.h>
// get 3 bytes from data (used in iTunesSD1)
static __forceinline unsigned long get3(const uint8_t * data)
{
	unsigned long ret = 0;
	ret += ((unsigned long) data[0]) << 16;
	ret += ((unsigned long) data[1]) << 8;
	ret += ((unsigned long) data[2]);
	return ret;
}
//write 3 bytes normal (used in iTunesSD1)
static __forceinline void put3(const unsigned long number, uint8_t * data)
{
	data[0] = (uint8_t)(number >> 16) & 0xff;
	data[1] = (uint8_t)(number >>  8) & 0xff;
	data[2] = (uint8_t)number & 0xff;
}

// pass data and ptr, updates ptr automatically (by reference)
static __forceinline void write_uint64_t(uint8_t *data, size_t &offset, uint64_t value)
{
	memcpy(&data[offset], &value, 8);
	offset+=8;
}

// pass data and ptr, updates ptr automatically (by reference)
static __forceinline void write_uint32_t(uint8_t *data, size_t &offset, uint32_t value)
{
	memcpy(&data[offset], &value, 4);
	offset+=4;
}

// pass data and ptr, updates ptr automatically (by reference)
static __forceinline void write_uint16_t(uint8_t *data, size_t &offset, uint16_t value)
{
	memcpy(&data[offset], &value, 2);
	offset+=2;
}


// pass data and ptr, updates ptr automatically (by reference)
static __forceinline void write_uint8_t(uint8_t *data, size_t &offset, uint8_t value)
{
	data[offset++] = value;
}

// pass data and ptr, updates ptr automatically (by reference)
static __forceinline void write_header(uint8_t *data, size_t &offset, const char *header)
{
	data[offset++] = header[0];
	data[offset++] = header[1];
	data[offset++] = header[2];
	data[offset++] = header[3];
}


// Case insensitive version of wcsstr
static wchar_t *wcsistr (const wchar_t *s1, const wchar_t *s2)
{
	wchar_t *cp = (wchar_t*) s1;
	wchar_t *s, *t, *endp;
	wchar_t l, r;

	endp = (wchar_t*)s1 + ( lstrlen(s1) - lstrlen(s2)) ;
	while (cp && *cp && (cp <= endp))
	{
		s = cp;
		t = (wchar_t*)s2;
		while (s && *s && t && *t)
		{
			l = towupper(*s);
			r = towupper(*t);
			if (l != r)
				break;
			s++, t++;
		}

		if (*t == 0)
			return cp;

		cp = CharNext(cp);
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////
// iTunesSD1 - Classes for dealing with the iPodShuffle
//////////////////////////////////////////////////////////////////////

iTunesSD1::iTunesSD1()
{
}

iTunesSD1::~iTunesSD1()
{
}


long iTunesSD1::write(const iPod_mhlt::mhit_map_t *songs, unsigned char * data, const unsigned long datasize)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iTunesSD_write);
#endif

	const unsigned int numsongs = songs->size();
	const unsigned int total_size = 18 + (numsongs * 558);
	ASSERT(datasize >= total_size);
	if(datasize < total_size)
		return(-1);

	long ptr=0;

	put3(numsongs, &data[ptr]);
	ptr+=3;

	put3(0x010600, &data[ptr]);
	ptr+=3;

	put3(0x12, &data[ptr]);
	ptr+=3;

	put3(0, &data[ptr]);
	ptr+=3;
	put3(0, &data[ptr]);
	ptr+=3;
	put3(0, &data[ptr]);
	ptr+=3;

	iPod_mhlt::mhit_map_t::const_iterator begin = songs->begin();
	iPod_mhlt::mhit_map_t::const_iterator end   = songs->end();
	for(iPod_mhlt::mhit_map_t::const_iterator it = begin; it != end; it++)
	{
		iPod_mhit *m = ((*it).second);
		iTunesSD_Song song(m);
		long ret = song.write(&data[ptr], datasize - ptr);
		if (ret < 0)
			return ret;
		ptr += ret;
	}

	return(ptr);
}


iTunesSD_Song::iTunesSD_Song(const iPod_mhit *m) : size_total(0x22e), starttime(0), stoptime(0), volume(0x64), filetype(0), playflags(iTunesSD_Song::SHUFFLE)
{
	memset(filename, 0, (SDSONG_FILENAME_LEN + 1) * sizeof(wchar_t));


	iPod_mhod *mhod = m->FindString(MHOD_LOCATION);
	ASSERT(mhod);
	if(mhod)
	{
		// Convert from HFS format (:iPod_Control:Music:F00:filename) to quasi-FAT format (/iPod_Control/Music/F00/filename) filepaths
		SetFilename(mhod->str);
		wchar_t *w = filename;
		while(w && *w != '\0')
		{
			if(*w == ':')
				*w = '/';

			w = CharNext(w);
		}


		SetStartTime(m->starttime);
		SetStopTime(m->stoptime);

		int volume = (int)m->volume;

		// If Sound Check information is present, use that instead of volume
		if(m->soundcheck != 0)
		{
			// This code converts SoundCheck back into a gain value, then into a -255 to 255 mhit::volume value
			const double gain = -10.0 * log10(m->soundcheck / 1000.0);
			volume = (int)(gain * 12.75);	// XXX - this might not be the best way to convert the gain value...
		}

		if(volume < -255)
			volume = -255;
		else if(volume > 255)
			volume = 255;

		// Convert the volume value into a percentage for SetVolume
		SetVolume((int)((double)volume / 2.55));


		// To determine the filetype, first check the MHOD_FILETYPE type.  If that isn't available, fallback to file extension
		iPod_mhod *mtype = m->FindString(MHOD_FILETYPE);
		if(mtype != NULL)
		{
			if(wcsistr(mtype->str, L"MPEG") != NULL || wcsistr(mtype->str, L"MP3") != NULL)
				filetype = iTunesSD_Song::MP3;
			else if(wcsistr(mtype->str, L"AAC") != NULL)
				filetype = iTunesSD_Song::AAC;
			else if(wcsistr(mtype->str, L"WAV") != NULL)
				filetype = iTunesSD_Song::WAV;
		}
		if(filetype == 0)
		{
			if(wcsistr(mhod->str, L".mp3") != NULL)
				filetype = iTunesSD_Song::MP3;
			else if(wcsistr(mhod->str, L".m4a") != NULL || wcsistr(mhod->str, L".m4b") != NULL || wcsistr(mhod->str, L".m4p") != NULL)
				filetype = iTunesSD_Song::AAC;
			else if(wcsistr(mhod->str, L".wav") != NULL)
				filetype = iTunesSD_Song::WAV;
		}
		ASSERT(filetype != 0);
		if(filename == 0)
			filetype = iTunesSD_Song::MP3;		// Default to mp3


		if(wcsistr(mhod->str, L".m4b") != NULL)
			playflags = iTunesSD_Song::BOOKMARKABLE;		// Only playback in normal mode
		else
			playflags = iTunesSD_Song::SHUFFLE;			// Playable in normal/shuffle modes, but not bookmarkable
	}
}


long iTunesSD_Song::write(unsigned char * data, const unsigned long datasize)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iTunesSD_Song_write);
#endif
	long ptr=0;

	ASSERT(size_total == 0x22e);
	ASSERT(filetype != 0);

	put3(size_total, &data[ptr]);
	ptr+=3;

	put3(0x005aa501, &data[ptr]);

	ptr+=3;
	put3(starttime, &data[ptr]);
	ptr+=3;
	put3(0, &data[ptr]);
	ptr+=3;
	put3(0, &data[ptr]);
	ptr+=3;
	put3(stoptime, &data[ptr]);
	ptr+=3;
	put3(0, &data[ptr]);
	ptr+=3;
	put3(0, &data[ptr]);
	ptr+=3;
	put3(volume, &data[ptr]);
	ptr+=3;
	put3(filetype, &data[ptr]);
	ptr+=3;
	put3(0x200, &data[ptr]);
	ptr+=3;

	const unsigned int bufSize = (SDSONG_FILENAME_LEN + 1) * sizeof(wchar_t);
	memcpy(&data[ptr], filename, bufSize);
	ptr+=bufSize;

	put3(playflags, &data[ptr]);
	ptr+=3;

	ASSERT(size_total == ptr);
	return(ptr);
}

void iTunesSD_Song::SetFilename(const wchar_t *filename)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iTunesStats_SetFilename);
#endif

	ASSERT(filename != NULL);
	if(filename == NULL)
		return;

	if(filename)
	{
		lstrcpyn(this->filename, filename, SDSONG_FILENAME_LEN);
	}
	else
	{
		memset(this->filename, 0, SDSONG_FILENAME_LEN * sizeof(wchar_t));
	}
}

// Accepts values from -100 to 100, with 0 meaning no volume change
void iTunesSD_Song::SetVolume(const int percent)
{
	int p = percent;
	if(p > 100)
		p = 100;
	else if(p < -100)
		p = -100;

	// Volume ranges from 0 (-100%) to 100 (0%) to 200 (100%)
	volume = (unsigned int)(percent + 100);
}


/* Shadow DB version 2 */
long iTunesSD2::write(const iPod_mhlt *songs, const iPod_mhlp *playlists, unsigned char * data, const unsigned long datasize)
{
	uint32_t numsongs = songs->GetChildrenCount();
	uint32_t numplaylists = playlists->GetChildrenCount();
	size_t offset=0;
	size_t ptr=0;

	if (datasize < 64)
		return -1;

	write_header(data, ptr, "bdhs");
	write_uint32_t(data, ptr, 0x02000003); /* also have seen 0x02010001, perhaps a DB version number? */
	write_uint32_t(data, ptr, 64); /* length of header */
	write_uint32_t(data, ptr, numsongs);
	write_uint32_t(data, ptr, numplaylists); /* number of playlists */
	write_uint32_t(data, ptr, 0);
	write_uint32_t(data, ptr, 0);
	write_uint8_t(data, ptr, 0); /* volume limit */
	write_uint8_t(data, ptr, 1); /* voiceover */
	write_uint16_t(data, ptr, 0);
	write_uint32_t(data, ptr, numsongs); /* TODO number of tracks w/o podcasts and audiobooks*/
	write_uint32_t(data, ptr, 64); /* track header offset */
	write_uint32_t(data, ptr, 64+20 + numsongs*4+iTunesSD2_Song::header_size*numsongs); /* playlist header offset */

	write_uint32_t(data, ptr, 0);
	write_uint32_t(data, ptr, 0);
	write_uint32_t(data, ptr, 0);
	write_uint32_t(data, ptr, 0);
	write_uint32_t(data, ptr, 0);
	offset = 64;

	uint32_t hths_header_size = 20+numsongs*4;
	if (datasize - ptr < hths_header_size)
		return -1;
	write_header(data, ptr, "hths");
	write_uint32_t(data, ptr, hths_header_size); /* header length */
	write_uint32_t(data, ptr, numsongs);
	write_uint32_t(data, ptr, 0);
	write_uint32_t(data, ptr, 0);
	offset += hths_header_size;
	/* positions for each track */
	for (size_t i=0;i<numsongs;i++)
	{
		write_uint32_t(data, ptr, offset + iTunesSD2_Song::header_size * i);
	}
	
	/* write tracks */
	for (uint32_t i=0;i<numsongs;i++)
	{
		iPod_mhit *m = songs->GetTrack(i);
		long ret = iTunesSD2_Song::write(m, &data[ptr], datasize - ptr);
		if (ret < 0)
			return ret;
		ptr += ret;
		offset += ret;
	}

	uint32_t podcast_playlist_count=0;
	for (size_t i=0;i<numplaylists;i++)
	{
		iPod_mhyp *p = playlists->GetPlaylist(i);
		if (p->podcastflag)
			podcast_playlist_count++;
	}

	uint32_t hphs_header_size = 20 + numplaylists*4;
	if (datasize - ptr < hphs_header_size)
		return -1;

	write_header(data, ptr, "hphs");
	write_uint32_t(data, ptr, hphs_header_size); /* header length */
	write_uint32_t(data, ptr, numplaylists);
	write_uint16_t(data, ptr, 0);
	write_uint16_t(data, ptr, numplaylists-podcast_playlist_count); /* non-podcast playlists */
	write_uint16_t(data, ptr, 1); /* master playlists */
	write_uint16_t(data, ptr, numplaylists); /* non-audiobook playlists */
	offset += hphs_header_size;

	/* write offsets for each track */
	for (size_t i=0;i<numplaylists;i++)
	{
		iPod_mhyp *p = playlists->GetPlaylist(i);
		write_uint32_t(data, ptr, offset);
		offset += p->GetMhipChildrenCount()*4 + 44;
	}

	iPod_mhyp *master_playlist = playlists->GetPlaylist(0);
	/* write playlists */
	for (size_t i=0;i<numplaylists;i++)
	{
		iPod_mhyp *p = playlists->GetPlaylist(i);
		long ret = iTunesSD2_Playlist::write(master_playlist, p, &data[ptr], datasize - ptr);
		if (ret < 0)
			return ret;
		ptr += ret;
	}
	return ptr;
}
uint32_t iTunesSD2_Song::header_size = 372;

long iTunesSD2_Song::write(const iPod_mhit *mhit, unsigned char *data, const unsigned long datasize)
{
	if (datasize < header_size)
		return -1;

	size_t ptr=0;
	write_header(data, ptr, "rths");
	write_uint32_t(data, ptr, header_size); /* length of header */
	write_uint32_t(data, ptr, mhit->starttime); /* start time, in milliseconds */
	write_uint32_t(data, ptr, mhit->stoptime); /* stop time, in milliseconds */
	write_uint32_t(data, ptr, mhit->volume); /* volume */
	switch(mhit->filetype)
	{
	case FILETYPE_WAV:
		write_uint32_t(data, ptr, iTunesSD_Song::WAV); /* file type */
		break;
	case FILETYPE_M4A:
	case 0x4d344220:
	case 0x4d345020:
		write_uint32_t(data, ptr, iTunesSD_Song::AAC); /* file type */
		break;
	case FILETYPE_MP3:
	default:
		write_uint32_t(data, ptr, iTunesSD_Song::MP3); /* file type */
		break;
	}

	iPod_mhod *mhod = mhit->FindString(MHOD_LOCATION);
	
	char filename[256] = {0};
	int converted = WideCharToMultiByte(CP_UTF8, 0, mhod->str, -1, filename, 256, 0, 0);
	for (int i=0;i<converted;i++)
	{
		if (filename[i] == ':')
			filename[i] = '/';
	}
	memcpy(&data[ptr], filename, converted);
	ptr+=converted;
	memset(&data[ptr], 0, 256-converted);
	ptr+=256-converted;
		
	write_uint32_t(data, ptr, mhit->bookmarktime); /* bookmark time */
	write_uint8_t(data, ptr, 0); /* skip flag */
	write_uint8_t(data, ptr, mhit->rememberPosition); /* remember playback position */
	write_uint8_t(data, ptr, 0); /* part of gapless album */
	write_uint8_t(data, ptr, 0);
	write_uint32_t(data, ptr, mhit->pregap); /* pre-gap */
	write_uint32_t(data, ptr, mhit->postgap); /* post-gap */
	write_uint64_t(data, ptr, mhit->samplecount); /* number of samples */
	write_uint32_t(data, ptr, mhit->gaplessData); /* gapless data */
	write_uint32_t(data, ptr, 0);
	write_uint32_t(data, ptr, mhit->album_id); /* album ID */
	write_uint16_t(data, ptr, mhit->tracknum); /* track number */
	write_uint16_t(data, ptr, mhit->cdnum); /* disc number */
	write_uint32_t(data, ptr, 0); 
	write_uint32_t(data, ptr, 0); 
	write_uint64_t(data, ptr, mhit->dbid); /* dbid */ 
	write_uint32_t(data, ptr, 0); /* artist ID */
	write_uint32_t(data, ptr, 0); 
	write_uint32_t(data, ptr, 0); 
	write_uint32_t(data, ptr, 0); 
	write_uint32_t(data, ptr, 0); 
	write_uint32_t(data, ptr, 0); 
	write_uint32_t(data, ptr, 0); 
	write_uint32_t(data, ptr, 0); 
	write_uint32_t(data, ptr, 0); 

	return ptr;
}

long iTunesSD2_Playlist::write(const iPod_mhyp *master_playlist, const iPod_mhyp *playlist, unsigned char * data, const unsigned long datasize)
{
	size_t ptr=0;
	uint32_t tracks = playlist->GetMhipChildrenCount();
	uint32_t header_size = 44 + tracks*4;
	if (datasize < header_size)
		return -1;

	write_header(data, ptr, "lphs");	
	write_uint32_t(data, ptr, header_size); /* header length */
	write_uint32_t(data, ptr, tracks); /* number of tracks */

	/* number of music tracks TODO: special handling for master playlist */
	if (playlist->podcastflag)
		write_uint32_t(data, ptr, 0);
	else
		write_uint32_t(data, ptr, tracks); 

	write_uint64_t(data, ptr, playlist->playlistID); /* playlist ID */

	/* playlist type */
	if (playlist->podcastflag)
		write_uint32_t(data, ptr, 3); /* podcast */
	else if (playlist->hidden)
		write_uint32_t(data, ptr, 1); /* master playlist */
	else
		write_uint32_t(data, ptr, 2); /* normal */
	write_uint32_t(data, ptr, 0); 
	write_uint32_t(data, ptr, 0); 
	write_uint32_t(data, ptr, 0); 
	write_uint32_t(data, ptr, 0); 
	if (master_playlist == playlist)
	{
		for (uint32_t i=0;i<tracks;i++)
		{
			write_uint32_t(data, ptr, i);
		}
	}
	else
	{
		for (uint32_t i=0;i<tracks;i++)
		{
			iPod_mhip *item = playlist->GetPlaylistEntry(i);
			uint32_t master_index = master_playlist->FindPlaylistEntry(item->songindex);
			assert(master_index != -1);
			write_uint32_t(data, ptr, master_index);
		}
	}
	return ptr;
}
