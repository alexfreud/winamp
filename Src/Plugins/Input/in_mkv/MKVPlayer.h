#pragma once
#include <windows.h>
#include <bfc/platform/types.h>

// nsmkv stuff
#include "../nsmkv/Cluster.h"
#include "../nsmkv/header.h"
#include "../nsmkv/SeekTable.h"
#include "../nsmkv/SegmentInfo.h"
#include "../nsmkv/Tracks.h"
#include "../nsmkv/Cues.h"
#include "../nsmkv/Attachments.h"
#include "../nsmkv/mkv_reader.h"

#include "ifc_mkvvideodecoder.h"
#include "ifc_mkvaudiodecoder.h"

#include "../nu/AutoLock.h"
#include "../nu/AudioOutput.h"


class MKVPlayer
{
public:
	MKVPlayer(const wchar_t *_filename);
	~MKVPlayer();
	DWORD CALLBACK ThreadFunction();
	DWORD CALLBACK VideoThreadFunction();

	void Kill();
	void Seek(int seek_pos);
	int GetOutputTime() const;

private:
	// subfunctions to make the code cleaner
	// they all have "side effects", which is a coding style I don't like
	// but it makes it easier to read & modify
	// TODO: move these to step, and have a state variable to know where we are
	bool ParseHeader(); // on completion, header will be filled, file pointer will be at next level 0 node
	bool FindSegment(); // on completion, segment_position will be calculated, file pointer will be at first level 1 node under segment

	// file position is restored at end of function
	bool FindCues();

	int ParseCluster(nsmkv::MKVReader *stream, uint64_t size, uint64_t *track_numbers, size_t track_numbers_len);

	enum
	{
		// values that you can return from OnXXXX()
		MKV_CONTINUE = 0, // continue processing
		MKV_ABORT = 1, // abort parsing gracefully (maybe 'stop' was pressed)
		MKV_STOP = 2, // stop parsing completely - usually returned when mkv version is too new or codecs not supported

		// values returned from errors within the Step() function itself
		MKV_EOF = 3, // end of file
		MKV_ERROR = 4, // parsing error
	};
	int OnHeader(const nsmkv::Header &header);
	void OnSegmentInfo(const nsmkv::SegmentInfo &segment_info);
	int OnTracks(const nsmkv::Tracks &tracks);
	int OnBlock(const nsmkv::Cluster &cluster, const nsmkv::Block &block);
	int OnFirstCluster(uint64_t position);
	int OnAudio(const nsmkv::Cluster &cluster, const nsmkv::BlockBinary &binary);
	int OnVideo(const nsmkv::Cluster &cluster, const nsmkv::BlockBinary &binary);

	int OutputPictures(uint64_t default_timestamp);
	/* start calling with cluster_number = 0 and block_number = 0 (or whatever appropriate based on CuePoints when seeking
	will return 0 on success, 1 on EOF and -1 on failure
	*/
	int GetBlock(nsmkv::MKVReader *stream, uint64_t track_number, nsmkv::BlockBinary &binary, const nsmkv::Cluster **cluster, size_t &cluster_number, size_t &block_number);
	static void CALLBACK SeekAPC(ULONG_PTR data);

	int Step(nsmkv::MKVReader *stream, uint64_t *track_numbers, size_t track_numbers_len); // only gives you block data for the passed track number

private:
	/* nsmkv internal implementation */
	nsmkv::Header header;
	uint64_t segment_position; // position of the start of the first level 1 element in the segment(for SeekHead relative positions)
	uint64_t segment_size; // size of that segment
	nsmkv::SeekTable seek_table;
	nsmkv::SegmentInfo segment_info;
	nsmkv::Tracks tracks;
	nsmkv::Clusters clusters;
	nsmkv::Cues cues;
	nsmkv::Attachments attachments;
	bool cues_searched;
	bool first_cluster_found;

	/* player implementation */
	nsmkv::MKVReader *main_reader; // also gets used as audio_stream
	Nullsoft::Utility::LockGuard cluster_guard;	
	HANDLE killswitch, seek_event;
	wchar_t *filename;
	volatile int m_needseek;

	/* Audio */
	ifc_mkvaudiodecoder *audio_decoder;
	bool audio_opened;
	uint64_t audio_track_num;
	uint8_t audio_buffer[65536]; // TODO: dynamically allocate from OutputFrameSize
	size_t audio_output_len;
	size_t audio_buffered;
	int audio_first_timestamp; 
	enum FlushState
	{
		FLUSH_NONE=0,
		FLUSH_START=1,
		FLUSH_SEEK=2,
	};
	FlushState audio_flushing;
	unsigned int audio_bitrate;

	/* Video */
	ifc_mkvvideodecoder *video_decoder;
	const nsmkv::TrackEntry *video_track_entry;
	bool video_opened;
	HANDLE video_thread;
	double video_timecode_scale;
	uint64_t video_track_num;
	uint64_t video_cluster_position;
	nsmkv::MKVReader *video_stream;
	HANDLE video_break, video_flush, video_flush_done, video_resume, video_ready;
	unsigned int video_bitrate;
	int consecutive_early_frames;

	/* AudioOutput implementation */
	class MKVWait
	{
	public:
		void Wait_SetEvents(HANDLE killswitch, HANDLE seek_event);
	protected:
		int WaitOrAbort(int time_in_ms);
	private:
		HANDLE handles[2];
	};
	nu::AudioOutput<MKVWait> audio_output;

};