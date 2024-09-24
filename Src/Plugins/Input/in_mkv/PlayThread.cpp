#include "MKVPlayer.h"
#include "api__in_mkv.h"
#include "main.h"


#include "../Winamp/wa_ipc.h"
#include "../nsmkv/nsmkv.h"
#include "../nu/AutoLock.h"
#include "../nu/ns_wc.h"
#include "../nu/AutoWide.h"
#include "../nu/AudioOutput.h"
#include "ifc_mkvaudiodecoder.h"
#include "svc_mkvdecoder.h"
#include "ifc_mkvvideodecoder.h"
#include "../nsmkv/file_mkv_reader.h"
#include <api/service/waservicefactory.h>
#include <api/service/services.h>
#include <windows.h>
#include <strsafe.h>

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
{
	0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf }
};


IVideoOutput *videoOutput = 0;
/* benski> 
TODO: keep track of "fully parsed position" we don't have to always start over at segment_position
TODO: if we have multiple audio or video tracks, do that weird winamp interface for it
*/


void MKVPlayer::MKVWait::Wait_SetEvents(HANDLE killswitch, HANDLE seek_event)
{
	handles[0]=killswitch;
	handles[1]=seek_event;
}

int MKVPlayer::MKVWait::WaitOrAbort(int time_in_ms)
{
	switch(WaitForMultipleObjects(2, handles, FALSE, 55))
	{
	case WAIT_TIMEOUT: // all good, wait successful
		return 0; 
	case WAIT_OBJECT_0: // killswitch
		return MKVPlayer::MKV_STOP;
	case WAIT_OBJECT_0+1: // seek event
		return MKVPlayer::MKV_ABORT;
	default: // some OS error?
		return MKVPlayer::MKV_ERROR;
	}
}

MKVPlayer::MKVPlayer(const wchar_t *_filename) : audio_output(&plugin)
{
	first_cluster_found = false;
	cues_searched = false;
	segment_position=0;
	segment_size = 0;

	filename = _wcsdup(_filename);
	m_needseek = -1;

	audio_decoder=0;
	audio_track_num=0;
	audio_output_len = 65536;
	audio_opened=false;
	audio_flushing=FLUSH_START;
	audio_first_timestamp=0;
	audio_buffered=0;
	audio_bitrate=0;

	video_decoder=0;
	video_track_num=0;
	video_opened = false;
	video_stream = 0;
	video_thread = 0;
	video_timecode_scale = 0;
	video_cluster_position = 0;
	video_track_entry = 0;
	video_bitrate = 0;
	consecutive_early_frames = 0;

	if (!videoOutput)
		videoOutput = (IVideoOutput *)SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_GET_IVIDEOOUTPUT);

	killswitch = CreateEvent(NULL, TRUE, FALSE, NULL);
	seek_event = CreateEvent(NULL, TRUE, FALSE, NULL);

	/* video events */
	video_break = CreateEvent(NULL, TRUE, FALSE, NULL);
	video_flush_done = CreateEvent(NULL, FALSE, FALSE, NULL);
	video_flush = CreateEvent(NULL, TRUE, FALSE, NULL);
	video_resume = CreateEvent(NULL, TRUE, FALSE, NULL);	
	video_ready = CreateEvent(NULL, TRUE, FALSE, NULL);	

	audio_output.Wait_SetEvents(killswitch, seek_event);
}

MKVPlayer::~MKVPlayer() {
	free(filename);
	CloseHandle(killswitch);
	CloseHandle(seek_event);

	CloseHandle(video_break);
	CloseHandle(video_flush_done);
	CloseHandle(video_flush);
	CloseHandle(video_resume);
	CloseHandle(video_ready);

	if (audio_decoder) {
		audio_decoder->Close();
	}
	delete main_reader;		
}

void MKVPlayer::Kill()
{
	SetEvent(killswitch);
	if (video_thread)
		WaitForSingleObject(video_thread, INFINITE);
	video_thread = 0;
	if (video_decoder)
		video_decoder->Close();
	video_decoder=0;
}

void MKVPlayer::Seek(int seek_pos)
{
	m_needseek = seek_pos;
	SetEvent(seek_event);
}

int MKVPlayer::GetOutputTime() const
{
	if (m_needseek != -1)
		return m_needseek;
	else
		return plugin.outMod->GetOutputTime() + audio_first_timestamp;
}

DWORD CALLBACK MKVThread(LPVOID param)
{
	MKVPlayer *player = (MKVPlayer *)param;
	DWORD ret = player->ThreadFunction();
	return ret;
}

bool MKVPlayer::FindCues()
{
	if (cues_searched)
		return true;

	uint64_t original_position = main_reader->Tell();

	// first, let's try the seek table
	uint64_t cues_position = 0;
	if (seek_table.GetEntry(mkv_segment_cues, &cues_position))
	{
		main_reader->Seek(cues_position+segment_position);
		ebml_node node;
		if (read_ebml_node(main_reader, &node) == 0)
			return false;

		if (node.id == mkv_segment_cues) // great success!
		{
			if (nsmkv::ReadCues(main_reader, node.size, cues) == 0)
				return false;
			cues_searched=true;
			main_reader->Seek(original_position);
			return true;
		}
	}

	main_reader->Seek(segment_position); // TODO: keep track of how far Step() has gotten so we don't have to start from scratch
	/* --- TODO: make this block into a function in nsmkv --- */
	while (1) // TODO: key off segment size to make sure we don't overread
	{
		uint64_t this_position = main_reader->Tell();
		ebml_node node;
		if (read_ebml_node(main_reader, &node) == 0)
			return false;

		if (node.id != mkv_void)
		{
			nsmkv::SeekEntry seek_entry(node.id, this_position-segment_position);
			seek_table.AddEntry(seek_entry, nsmkv::SeekTable::ADDENTRY_FOUND);
		}

		if (node.id == mkv_segment_cues)
		{
			if (nsmkv::ReadCues(main_reader, node.size, cues) == 0)
				return false;
			break;
		}
		else
		{
			main_reader->Skip(node.size);
		}
	}
	/* ------ */
	cues_searched=true;
	main_reader->Seek(original_position);
	return true;
}

bool MKVPlayer::ParseHeader()
{
	ebml_node node;
	if (read_ebml_node(main_reader, &node) == 0)
		return false;

	if (node.id != mkv_header)
		return false;

	if (nsmkv::ReadHeader(main_reader, node.size, header) == 0)
		return false;

	if (OnHeader(header) != MKV_CONTINUE)
		return false;

	return true;
}

bool MKVPlayer::FindSegment()
{
	ebml_node node;
	while (segment_position == 0)
	{
		if (read_ebml_node(main_reader, &node) == 0)
			return false;

		if (node.id == mkv_segment)
		{
			segment_position = main_reader->Tell();
			segment_size = node.size;
		}
		else
		{
			if (nsmkv::SkipNode(main_reader, node.id, node.size) == 0)
				return false;
		}
	}
	return true;
}

static ifc_mkvvideodecoder *FindVideoDecoder(const nsmkv::TrackEntry *track_entry)
{
	size_t n = 0;
	waServiceFactory *sf = 0;
	while (sf = plugin.service->service_enumService(WaSvc::MKVDECODER, n++))
	{
		svc_mkvdecoder *dec = static_cast<svc_mkvdecoder *>(sf->getInterface());
		if (dec)
		{
			ifc_mkvvideodecoder *decoder=0;
			if (dec->CreateVideoDecoder(track_entry->codec_id, track_entry, &track_entry->video, &decoder) == svc_mkvdecoder::CREATEDECODER_SUCCESS)
			{
				sf->releaseInterface(dec);
				return decoder;
			}
			sf->releaseInterface(dec);
		}
	}
	return 0;
}

static ifc_mkvaudiodecoder *FindAudioDecoder(const nsmkv::TrackEntry *track_entry)
{
	unsigned int bits_per_sample = (unsigned int)AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"bits", 16);
	if (bits_per_sample >= 24)	bits_per_sample = 24;
	else	bits_per_sample = 16;

	unsigned int max_channels;
	// get max channels
	if (AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"surround", true))
		max_channels = 6;
	else if (AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"mono", false))
		max_channels = 1;
	else
		max_channels = 2;

	size_t n=0;
	waServiceFactory *sf = 0;
	while (sf = plugin.service->service_enumService(WaSvc::MKVDECODER, n++))
	{
		svc_mkvdecoder *dec = static_cast<svc_mkvdecoder *>(sf->getInterface());
		if (dec)
		{
			ifc_mkvaudiodecoder *decoder=0;
			// TODO: read from api_config!!
			if (dec->CreateAudioDecoder(track_entry->codec_id, track_entry, &track_entry->audio, bits_per_sample, max_channels, false, &decoder) == svc_mkvdecoder::CREATEDECODER_SUCCESS)
			{
				sf->releaseInterface(dec);
				return decoder;
			}
			sf->releaseInterface(dec);
		}
	}
	return 0;
}

int MKVPlayer::OnAudio(const nsmkv::Cluster &cluster, const nsmkv::BlockBinary &binary)
{
	if (video_decoder)
	{
		HANDLE handles[3] = {killswitch, seek_event, video_ready};
		if (WaitForMultipleObjects(3, handles, FALSE, INFINITE) != WAIT_OBJECT_0+2)
			return MKV_ABORT;
	}

	if (audio_flushing != FLUSH_NONE)
	{					
		uint64_t timestamp=cluster.time_code + binary.time_code;
		uint64_t timestamp_ms = segment_info.time_code_scale * timestamp / 1000000ULL;
		if (audio_flushing == FLUSH_START || !audio_opened)
		{
			audio_first_timestamp = (int)timestamp_ms;
		}
		else
		{			
			audio_first_timestamp=0;
			audio_output.Flush((int)timestamp_ms);
			m_needseek = -1;
		}
		audio_flushing=FLUSH_NONE;
	}

	nsmkv::LacingState lacing_state;
	uint32_t i = 0;
	if (nsmkv::Lacing::GetState(binary.flags, (const uint8_t *)binary.data, binary.data_size, &lacing_state))
	{
		const uint8_t *frame = 0;
		size_t frame_len = 0;
		while (nsmkv::Lacing::GetFrame(i++, (const uint8_t *)binary.data, binary.data_size, &frame, &frame_len, &lacing_state))
		{
			size_t decoded_size = audio_output_len-audio_buffered;
			if (audio_decoder->DecodeBlock((void *)frame, frame_len, audio_buffer+audio_buffered, &decoded_size) == ifc_mkvaudiodecoder::MKV_SUCCESS)
			{
				decoded_size+=audio_buffered;
				audio_buffered=0;
				if (!audio_opened)
				{
					unsigned int sample_rate, channels, bps;
					bool is_float;
					if (audio_decoder->GetOutputProperties(&sample_rate, &channels, &bps, &is_float) == ifc_mkvaudiodecoder::MKV_SUCCESS)
					{
						// TODO: pass -666 for 5th param if video
						if (!audio_output.Open(audio_first_timestamp, channels, sample_rate, bps, -1, -1))
						{
							return MKV_STOP;
						}
						audio_opened=true;
					}
				}

				if (audio_opened && decoded_size)
				{
					int ret = audio_output.Write((char *)audio_buffer, decoded_size);
					if (ret != 0)
						return ret;
				}
				else
				{
					audio_buffered=decoded_size;
				}
			}
		}
	}
	return MKV_CONTINUE;
}

int MKVPlayer::OutputPictures(uint64_t default_timestamp)
{
	void *data=0, *decoder_data=0;
	uint64_t timestamp=default_timestamp;
	while (video_decoder->GetPicture(&data, &decoder_data, &timestamp) == ifc_mkvvideodecoder::MKV_SUCCESS)
	{
		if (!video_opened)
		{
			int color_format = 0;
			int width = 0, height = 0;
			double aspect_ratio = 1.0;
			if (video_decoder->GetOutputProperties(&width, &height, &color_format, &aspect_ratio) == ifc_mkvvideodecoder::MKV_SUCCESS)
			{
				if (video_track_entry && video_track_entry->video.display_height && video_track_entry->video.display_width && video_track_entry->video.pixel_height && video_track_entry->video.pixel_width)
				{
					aspect_ratio = (double)video_track_entry->video.pixel_width / (double)video_track_entry->video.pixel_height / ((double)video_track_entry->video.display_width / (double)video_track_entry->video.display_height);
				}
				else
				{
					// winamp wants an "aspect correction value" not the true aspect ratio itself
					aspect_ratio = 1.0/aspect_ratio; 
				}
				videoOutput->extended(VIDUSER_SET_THREAD_SAFE, 1, 0);
				videoOutput->open(width, height, 0, aspect_ratio, color_format);
				video_opened=true;
				SetEvent(video_ready);
			}
		}

		if (video_opened)
		{
			uint64_t timestamp_ms;
			if (video_timecode_scale == 0)
				timestamp_ms = segment_info.time_code_scale * timestamp / 1000000ULL;
			else
				timestamp_ms = (uint64_t) (video_timecode_scale * (double)segment_info.time_code_scale * (double)timestamp / 1000000.0);
again:
			int realTime = plugin.outMod->GetOutputTime() + audio_first_timestamp;
			int time_diff = (int)timestamp_ms - realTime;
			if (time_diff > 12 && consecutive_early_frames) // plenty of time, go ahead and turn off frame dropping
			{
				if (--consecutive_early_frames == 0)
					video_decoder->HurryUp(0);
			}
			else if (time_diff < -50) // shit we're way late, start dropping frames
			{
				video_decoder->HurryUp(1);
				consecutive_early_frames += 3;
			}
			if (time_diff > 3)
			{
				HANDLE handles[] = {killswitch, video_break};
				int ret= WaitForMultipleObjects(2, handles, FALSE, (DWORD)(timestamp_ms-realTime));
				if (ret != WAIT_TIMEOUT)
				{
					video_decoder->FreePicture(data, decoder_data);
					if (ret == WAIT_OBJECT_0+1) /* second event doesn't stop stream*/
						return MKV_ABORT;
					return MKV_STOP;
				}
				goto again; // TODO: handle paused state a little better than this
			}

			videoOutput->draw(data);
		}

		video_decoder->FreePicture(data, decoder_data);
	}
	return MKV_CONTINUE;
}

int MKVPlayer::OnVideo(const nsmkv::Cluster &cluster, const nsmkv::BlockBinary &binary)
{
	if (video_decoder)
	{
		nsmkv::LacingState lacing_state;
		uint32_t i = 0;
		if (nsmkv::Lacing::GetState(binary.flags, (const uint8_t *)binary.data, binary.data_size, &lacing_state))
		{
			const uint8_t *frame = 0;
			size_t frame_len = 0;
			while (nsmkv::Lacing::GetFrame(i++, (const uint8_t *)binary.data, binary.data_size, &frame, &frame_len, &lacing_state))
			{
				// matroska epic fail: laced frames don't have separate timestamps!
				if (video_decoder) video_decoder->DecodeBlock(frame, frame_len, cluster.time_code+binary.time_code);

				uint64_t timestamp=cluster.time_code + binary.time_code;
				int ret = OutputPictures(timestamp);
				if (ret != MKV_CONTINUE)
					return ret;
			}
		}
	}
	return MKV_CONTINUE;
}

DWORD CALLBACK MKVPlayer::VideoThreadFunction()
{
	//video_stream = _wfopen(filename, L"rb");
	if (!video_stream)
		return 1;

	video_stream->Seek(video_cluster_position);
	HANDLE handles[] = { killswitch, video_break, video_flush, video_resume };
	DWORD waitTime = 0;
	while (1)
	{
		int ret = WaitForMultipleObjects(4, handles, FALSE, waitTime);
		if (ret == WAIT_TIMEOUT)
		{
			int ret = Step(video_stream, &video_track_num, 1);
			if (ret == MKV_EOF)
			{
				video_decoder->EndOfStream();
				OutputPictures(0);
				// TODO: tell decoder about end-of-stream to flush buffers
				waitTime = INFINITE;
			}
			else if (ret == MKV_ERROR || ret == MKV_STOP)
			{
				waitTime = INFINITE; // wait for killswitch
			}
		}
		else if (ret == WAIT_OBJECT_0)
		{
			break;
		}
		else if (ret == WAIT_OBJECT_0 + 1) // video break
		{
			waitTime = INFINITE; // this will stop us from decoding samples for a while
			ResetEvent(video_break);
			SetEvent(video_flush_done);
		}
		else if (ret == WAIT_OBJECT_0 + 2) // video flush
		{
			if (video_decoder)
				video_decoder->Flush();
			ResetEvent(video_flush);
			waitTime = 0;
			SetEvent(video_flush_done);
		}
		else if (ret == WAIT_OBJECT_0 + 3) // resume video
		{
			ResetEvent(video_resume);
			waitTime = 0;
			SetEvent(video_flush_done);
		}
	}
	if (videoOutput)
		videoOutput->close();

	delete video_stream;
	video_stream=0;
	return 0;
}

int MKVPlayer::OnHeader(const nsmkv::Header &header)
{
	// TODO: figure out if the file is really matroska, and if we can support the ebml version
	return MKV_CONTINUE;
}

void MKVPlayer::OnSegmentInfo(const nsmkv::SegmentInfo &segment_info)
{
	g_duration = segment_info.GetDurationMilliseconds();
	uint64_t content_length = main_reader->GetContentLength();
	if (content_length && g_duration)
	{
		int bitrate = (int)(8ULL * content_length / (uint64_t)g_duration);
		plugin.SetInfo(bitrate, -1, -1, -1);
	}
}

int MKVPlayer::OnTracks(const nsmkv::Tracks &tracks)
{
	wchar_t audio_info[256] = {0};
	wchar_t video_info[256] = {0};
	// ===== enumerate tracks and find decoders =====
	size_t i=0;
	const nsmkv::TrackEntry *track_entry;
	while (track_entry = tracks.EnumTrack(i++))
	{
		if (track_entry->track_type == mkv_track_type_audio && !audio_decoder)
		{
			audio_decoder = FindAudioDecoder(track_entry);
			if (audio_decoder)
			{
				MultiByteToWideCharSZ(CP_UTF8, 0, track_entry->codec_id, -1, audio_info, 256);
				audio_track_num = track_entry->track_number;
			}
		}
		else if (track_entry->track_type == mkv_track_type_video && !video_decoder)
		{
			video_decoder = FindVideoDecoder(track_entry);
			if (video_decoder)
			{
				StringCbPrintfW(video_info, sizeof(video_info), L"%s %I64ux%I64u", AutoWide(track_entry->codec_id, CP_UTF8), track_entry->video.pixel_width, track_entry->video.pixel_height);
				video_track_num = track_entry->track_number;
				video_stream = new MKVReaderFILE(filename);
				video_timecode_scale = track_entry->track_timecode_scale;
				video_track_entry = track_entry;
			}
		}
	}

	// TODO this prevents trying to play video only files
	//		which the plug-in is not at all happy playing
	/*if (!audio_decoder)// && !video_decoder)
		return MKV_STOP;*/

	wchar_t video_status[512] = {0};
	if (audio_decoder && video_decoder)
	{
		StringCbPrintf(video_status, sizeof(video_status), L"MKV: %s, %s", audio_info, video_info);
		videoOutput->extended(VIDUSER_SET_INFOSTRINGW,(INT_PTR)video_status,0);
	}
	else if (audio_decoder)
	{
		StringCbPrintf(video_status, sizeof(video_status), L"MKV: %s", audio_info);
		videoOutput->extended(VIDUSER_SET_INFOSTRINGW,(INT_PTR)video_status,0);
	}
	else if (video_decoder)
	{
		StringCbPrintf(video_status, sizeof(video_status), L"MKV: %s, %s", audio_info, video_info);
		videoOutput->extended(VIDUSER_SET_INFOSTRINGW,(INT_PTR)video_status,0);
	}

	return MKV_CONTINUE;
}

DWORD CALLBACK VideoThread(LPVOID param)
{
	MKVPlayer *player = (MKVPlayer *)param;
	return player->VideoThreadFunction();
}

int MKVPlayer::ParseCluster(nsmkv::MKVReader *stream, uint64_t size, uint64_t *track_numbers, size_t track_numbers_len)
{
	nsmkv::Cluster cluster;

	uint64_t total_bytes_read=0;
	while (size)
	{
		ebml_node node;
		uint64_t bytes_read = read_ebml_node(stream, &node);

		if (bytes_read == 0)
			return MKV_ERROR;

		// benski> checking bytes_read and node.size separately prevents possible integer overflow attack
		if (bytes_read > size)
			return MKV_ERROR;
		total_bytes_read+=bytes_read;
		size-=bytes_read;

		if (node.size > size)
			return MKV_ERROR;
		total_bytes_read+=node.size;
		size-=node.size;

		switch(node.id)
		{
		case mkv_cluster_timecode:
			{
				uint64_t val;
				if (read_unsigned(stream, node.size, &val) == 0)
					return MKV_ERROR;

				printf("Time Code: %I64u\n", val);
				cluster.time_code = val;
			}
			break;
		case mkv_cluster_blockgroup:
			{
				printf("Block Group\n");
				nsmkv::Block block;
				if (nsmkv::ReadBlockGroup(stream, node.size, block, track_numbers, track_numbers_len) == 0)
					return MKV_ERROR;

				if (block.binary.data)
				{
					int ret = OnBlock(cluster, block);
					if (ret != MKV_CONTINUE)
						return ret;
				}
			}
			break;
		case mkv_cluster_simpleblock:
			{
				printf("simple block, size: %I64u\n", node.size);
				nsmkv::Block block;
				if (ReadBlockBinary(stream, node.size, block.binary, track_numbers, track_numbers_len) == 0)
					return 0;

				if (block.binary.data)
				{
					int ret = OnBlock(cluster, block);
					if (ret != MKV_CONTINUE)
						return ret;
				}
			}
			break;
		default:
			nsmkv::ReadGlobal(stream, node.id, node.size);
		}
	}
	return MKV_CONTINUE;
}

int MKVPlayer::OnBlock(const nsmkv::Cluster &cluster, const nsmkv::Block &block)
{
	if (WaitForSingleObject(killswitch, 0) == WAIT_TIMEOUT)
	{
		if (block.binary.track_number == audio_track_num)
		{
			return OnAudio(cluster, block.binary);
		}
		else if (block.binary.track_number == video_track_num)
		{
			return OnVideo(cluster, block.binary);
		}
		return MKV_CONTINUE;
	}
	else
		return MKV_ABORT;
}

int MKVPlayer::OnFirstCluster(uint64_t position)
{
	if (video_decoder)
	{
		video_cluster_position = position;
		video_thread = CreateThread(0, 0, VideoThread, this, 0, 0);
		SetThreadPriority(video_thread, (int)AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));
	}
	return MKV_CONTINUE;
}

int MKVPlayer::Step(nsmkv::MKVReader *stream, uint64_t *track_numbers, size_t track_numbers_len)
{
	uint64_t this_position = stream->Tell();

	ebml_node node;
	if (read_ebml_node(stream, &node) == 0)
		return MKV_EOF;

	if (node.id != mkv_void)
	{
		nsmkv::SeekEntry seek_entry(node.id, this_position-segment_position);
		seek_table.AddEntry(seek_entry, nsmkv::SeekTable::ADDENTRY_FOUND);
	}

	switch(node.id)
	{
	case mkv_segment_segmentinfo:
		if (nsmkv::ReadSegmentInfo(stream, node.size, segment_info) == 0)
			return MKV_EOF;
		OnSegmentInfo(segment_info);
		break;

	case mkv_metaseek_seekhead:
		if (nsmkv::ReadSeekHead(stream, node.size, seek_table) == 0)
			return MKV_EOF;
		break;

	case mkv_segment_tracks:
		if (nsmkv::ReadTracks(stream, node.size, tracks) == 0)
			return MKV_EOF;
		return OnTracks(tracks);
		break;

	case mkv_segment_cues:
		if (!cues_searched)
		{
			if (nsmkv::ReadCues(stream, node.size, cues) == 0)
				return MKV_EOF;
			cues_searched=true;
		}
		else
		{
			stream->Skip(node.size);
		}
		break;

	case mkv_segment_cluster:
		if (!first_cluster_found)
		{
			first_cluster_found=true;
			OnFirstCluster(this_position);
		}
		return ParseCluster(stream, node.size, track_numbers, track_numbers_len);
		break;

	case mkv_segment_attachments:
		if (nsmkv::ReadAttachment(stream, node.size, attachments) == 0)
			return MKV_EOF;
		break;

	default:
		if (nsmkv::ReadGlobal(stream, node.id, node.size) == 0)
			return MKV_EOF;
		break;
	}
	return MKV_CONTINUE;
}

DWORD CALLBACK MKVPlayer::ThreadFunction()
{
	// ===== tell audio output helper object about the output plugin =====
	audio_output.Init(plugin.outMod);

	FILE *f = _wfopen(filename, L"rb");
	if (!f)
		goto btfo;

	main_reader = new MKVReaderFILE(f);

	// ===== read the header =====
	if (!ParseHeader())
		goto btfo;


	// ===== find segment start =====
	if (!FindSegment())
		goto btfo;

	// TODO: try to find more segments?

	HANDLE handles[] = {killswitch, seek_event};
	while(1)
	{
		int ret = WaitForMultipleObjects(2, handles, FALSE, 0);
		if (ret == WAIT_TIMEOUT)
		{
			int ret = Step(main_reader, &audio_track_num, 1);
			if (ret == MKV_EOF)
			{
				break;
			}
			else if (ret == MKV_ERROR || ret == MKV_STOP)
			{
				break;
			}
		}
		else if (ret == WAIT_OBJECT_0) // kill
		{
			break;
		}
		else if (ret == WAIT_OBJECT_0+1) // seek event
		{
			ResetEvent(seek_event);
			// pause video thread
			if (video_decoder)
			{
				SetEvent(video_break);
				WaitForSingleObject(video_flush_done, INFINITE);
			}
			FindCues();
			uint64_t seek_time = segment_info.ConvertMillisecondsToTime(m_needseek);
			uint64_t curr_time = segment_info.ConvertMillisecondsToTime(plugin.outMod->GetOutputTime() + audio_first_timestamp);
			int direction = (curr_time < seek_time)?nsmkv::Cues::SEEK_FORWARD:nsmkv::Cues::SEEK_BACKWARD;
			nsmkv::CuePoint *cue_point = cues.GetCuePoint(seek_time, curr_time, direction);
			if (cue_point)
			{
				nsmkv::CueTrackPosition *position = cue_point->GetPosition(audio_track_num);
				if (!position) // some files don't have the audio track.  we're going to assume the data is interleaved and just use the video track
					position = cue_point->GetPosition(video_track_num);
				if (position)
				{
					audio_flushing=FLUSH_SEEK;
					if (audio_decoder) audio_decoder->Flush();
					main_reader->Seek(position->cluster_position + segment_position);
				}
				if (video_stream)
				{
					position = cue_point->GetPosition(video_track_num);
					if (position)
					{
						video_stream->Seek(position->cluster_position + segment_position);
					}
				}
			}
			else
			{
				// TODO	enumerate clusters & blocks to find closest time (ugh)
			}

			if (video_decoder)
			{
				SetEvent(video_flush);
				WaitForSingleObject(video_flush_done, INFINITE);
			}
		}
	}

	delete main_reader;
	main_reader=0;
	if (WaitForSingleObject(killswitch, 0) != WAIT_OBJECT_0)
	{
		// TODO: tell audio decoder about end-of-stream and get remaining audio
		audio_output.Write(0,0);
		audio_output.WaitWhilePlaying();

		if (WaitForSingleObject(killswitch, 0) != WAIT_OBJECT_0)
			PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
	}
	if (audio_decoder)
	{
		audio_decoder->Close();
		audio_decoder=0;
		audio_output.Close();
	}

	return 0;

btfo: // bail the fuck out
	if (WaitForSingleObject(killswitch, 0) != WAIT_OBJECT_0)
		PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
	delete main_reader;
	main_reader=0;
	if (audio_decoder)
	{
		audio_decoder->Close();
		audio_decoder=0;
		audio_output.Close();
	}
	return 1;
}