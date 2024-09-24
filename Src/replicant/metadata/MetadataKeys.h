#pragma once

namespace MetadataKeys
{
	const int EXTENDED_KEYS_OFFSET = 1000;		// The offset to where the extended id's start

	enum
	{
		UNKNOWN=-1,
		ARTIST=0,
		ALBUM_ARTIST=1,
		ALBUM=2,
		TITLE=3,
		URI=4,
		GENRE=5,
		YEAR=6,
		TRACK=7, // in ifc_metadata::GetField, this might return something like "2/12" for track 2 out of 12.  in ifc_metadata::GetInteger, you will just get the track number (use TRACKS for total)
		DISC=8, // in ifc_metadata::GetField, this might return something like "1/2" for disc 1 out of 2.  in ifc_metadata::GetInteger, you will just get the disc number (use DISCS for total)
		BITRATE=9,
		COMPOSER=10,
		PUBLISHER=11,
		BPM=12,
		COMMENT=13,
		DISCS=14, // only valid for use in ifc_metadata::GetInteger
		FILE_SIZE=15,
		FILE_TIME=16,
		LENGTH=17,
		PLAY_COUNT=18,
		RATING=19,
		SERVER=20,
		MIME_TYPE=21,
		TRACK_GAIN=22,
		TRACK_PEAK=23,
		ALBUM_GAIN=24,
		ALBUM_PEAK=25,
		TRACKS=26, // only valid for use in ifc_metadata::GetInteger
		PREGAP=27,
		POSTGAP=28,
		STAT=29,
		CATEGORY=30,
		DIRECTOR=31,
		PRODUCER=32,
		LAST_PLAY=33,
		LAST_UPDATE=34,
		ADDED=35, // date added
		CLOUD=36, // used by pmp_cloud for the 'all sources' view
		METAHASH=37, // used by pmp_cloud for the 'all sources' view
		NUM_OF_METADATA_KEYS,
	};
}
