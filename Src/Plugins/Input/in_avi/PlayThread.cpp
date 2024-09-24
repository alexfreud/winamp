#include "main.h"
#include "api__in_avi.h"
#include "../nsavi/nsavi.h"
#include "interfaces.h"
#include "../nu/AudioOutput.h"
#include "../Winamp/wa_ipc.h"
#include <api/service/waservicefactory.h>
#include "VideoThread.h"
#include "win32_avi_reader.h"
#include "http_avi_reader.h"
#include "StreamSelector.h"
#include <shlwapi.h>
#include <strsafe.h>
#include <map>

nsavi::HeaderList header_list;
int video_stream_num, audio_stream_num;
ifc_avivideodecoder *video_decoder=0;
IVideoOutput *video_output=0;
HANDLE audio_break=0, audio_resume=0, audio_break_done=0;
static Streams streams;
static bool checked_in_dshow=false;
extern int GetOutputTime();

class StatsFOURCC
{
public:
	uint32_t GetAudioStat()
	{
		uint32_t fourcc=0;
		uint32_t max=0;
		for (Stats::iterator itr = audio_types.begin();itr!=audio_types.end();itr++)
		{
			if (itr->second > max)
			{
				max = itr->second;
				fourcc = itr->first;
			}
		}
		return fourcc;
	}

	uint32_t GetVideoStat()
	{
		uint32_t fourcc=0;
		uint32_t max=0;
		for (Stats::iterator itr = video_fourccs.begin();itr!=video_fourccs.end();itr++)
		{
			if (itr->second > max)
			{
				max = itr->second;
				fourcc = itr->first;
			}
		}
		return fourcc;
	}

	typedef std::map<uint32_t, uint32_t> Stats;
	Stats audio_types;
	Stats video_fourccs;
};

static StatsFOURCC stats;
class AVIWait
{
public:
	int WaitOrAbort(int time_in_ms)
	{
		HANDLE events[] = {killswitch, seek_event};
		int ret = WaitForMultipleObjects(2, events, FALSE, time_in_ms);
		if (ret == WAIT_TIMEOUT)
			return 0;
		else if (ret == WAIT_OBJECT_0)
			return 1;
		else if (ret == WAIT_OBJECT_0+1)
			return 2;

		return -1;
	}
};

static bool audio_opened=false;
static ifc_aviaudiodecoder *audio_decoder=0;
static char audio_output[65536];
static nu::AudioOutput<AVIWait> out(&plugin);

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
{
	0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf }
};

static int GetStreamNumber( uint32_t id )
{
	char *stream_data = (char *)( &id );
	if ( !isxdigit( stream_data[ 0 ] ) || !isxdigit( stream_data[ 1 ] ) )
		return -1;

	stream_data[ 2 ] = 0;
	int stream_number = strtoul( stream_data, 0, 16 );

	return stream_number;
}

static ifc_aviaudiodecoder *FindAudioDecoder( const nsavi::AVIH *avi_header, const nsavi::STRL &stream )
{
	unsigned int bits_per_sample = (unsigned int)AGAVE_API_CONFIG->GetUnsigned( playbackConfigGroupGUID, L"bits", 16 );
	if ( bits_per_sample >= 24 )	bits_per_sample = 24;
	else	bits_per_sample = 16;

	unsigned int max_channels;
	// get max channels
	if ( AGAVE_API_CONFIG->GetBool( playbackConfigGroupGUID, L"surround", true ) )
		max_channels = 6;
	else if ( AGAVE_API_CONFIG->GetBool( playbackConfigGroupGUID, L"mono", false ) )
		max_channels = 1;
	else
		max_channels = 2;

	size_t n = 0;
	waServiceFactory *sf = 0;
	while ( sf = plugin.service->service_enumService( WaSvc::AVIDECODER, n++ ) )
	{
		svc_avidecoder *dec = static_cast<svc_avidecoder *>( sf->getInterface() );
		if ( dec )
		{
			ifc_aviaudiodecoder *decoder = 0;
			if ( dec->CreateAudioDecoder( avi_header, stream.stream_header, stream.stream_format, stream.stream_data,
				 bits_per_sample, max_channels, false,
				 &decoder ) == svc_avidecoder::CREATEDECODER_SUCCESS )
			{
				sf->releaseInterface( dec );
				return decoder;
			}

			sf->releaseInterface( dec );
		}
	}

	return 0;
}


static ifc_avivideodecoder *FindVideoDecoder(const nsavi::AVIH *avi_header, const nsavi::STRL &stream)
{
	size_t n = 0;
	waServiceFactory *sf = 0;
	while (sf = plugin.service->service_enumService(WaSvc::AVIDECODER, n++))
	{
		svc_avidecoder *dec = static_cast<svc_avidecoder *>(sf->getInterface());
		if (dec)
		{
			ifc_avivideodecoder *decoder=0;
			if (dec->CreateVideoDecoder(avi_header, stream.stream_header, stream.stream_format, stream.stream_data, &decoder) == svc_avidecoder::CREATEDECODER_SUCCESS)
			{
				sf->releaseInterface(dec);
				return decoder;
			}

			sf->releaseInterface(dec);
		}
	}

	return 0;
}


static bool OnAudio( uint16_t type, const void **input_buffer, uint32_t *input_buffer_bytes )
{
	uint32_t output_len = sizeof( audio_output );
	int ret = audio_decoder->DecodeChunk( type, input_buffer, input_buffer_bytes, audio_output, &output_len );
	//if (*input_buffer_bytes != 0)
	//DebugBreak();
	if ( ( ret == ifc_aviaudiodecoder::AVI_SUCCESS || ret == ifc_aviaudiodecoder::AVI_NEED_MORE_INPUT ) && output_len )
	{
		if ( !audio_opened )
		{
			unsigned int sample_rate, channels, bps;
			bool is_float;
			if ( audio_decoder->GetOutputProperties( &sample_rate, &channels, &bps, &is_float ) == ifc_aviaudiodecoder::AVI_SUCCESS )
			{
				audio_opened = out.Open( 0, channels, sample_rate, bps );
				if ( !audio_opened )
					return false;
			}
			else
			{
				// TODO: buffer audio.  can nu::AudioOutput handle this for us?
			}
		}

		if ( audio_opened )
			out.Write( audio_output, output_len );
	}

	return true;
}

static bool CheckDSHOW()
{
	if (!checked_in_dshow)
	{
		LPCWSTR pluginsDir = (LPCWSTR)SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_GETPLUGINDIRECTORYW);
		wchar_t in_dshow_path[MAX_PATH] = {0};
		PathCombine(in_dshow_path, pluginsDir, L"in_dshow.dll");
		in_dshow = LoadLibrary(in_dshow_path);
		checked_in_dshow = true;
	}

	return !!in_dshow;
}

static void CALLBACK DSHOWAPC( ULONG_PTR param )
{
	In_Module *dshow_mod_local = 0;
	wchar_t *playFile = (wchar_t *)param;

	if ( in_dshow )
	{
		typedef In_Module *( *MODULEGETTER )( );

		MODULEGETTER moduleGetter = (MODULEGETTER)GetProcAddress( in_dshow, "winampGetInModule2" );
		if ( moduleGetter )
			dshow_mod_local = moduleGetter();
	}

	if ( dshow_mod_local )
	{
		dshow_mod_local->outMod = plugin.outMod;
		if ( dshow_mod_local->Play( playFile ) )
			dshow_mod_local = 0;
	}

	free( playFile );

	if ( !dshow_mod_local )
	{
		if ( WaitForSingleObject( killswitch, 200 ) != WAIT_OBJECT_0 )
			PostMessage( plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0 );
	}
	else
		dshow_mod = dshow_mod_local;
}

/* --- Video Window text info --- */
void SetVideoInfoText()
{
	wchar_t audio_name[128] = {0}, audio_properties[256] = {0};
	wchar_t video_name[128] = {0}, video_properties[256] = {0};
	wchar_t video_info[512] = {0};
	if (audio_decoder && video_decoder)
	{
		GetAudioCodecName(audio_name, sizeof(audio_name)/sizeof(*audio_name), header_list.stream_list[audio_stream_num].stream_format);
		GetAudioCodecDescription(audio_properties, sizeof(audio_properties)/sizeof(*audio_properties), header_list.stream_list[audio_stream_num].stream_format); 
		GetVideoCodecName(video_name, sizeof(video_name)/sizeof(*video_name), header_list.stream_list[video_stream_num].stream_format); 
		GetVideoCodecDescription(video_properties, sizeof(video_properties)/sizeof(*video_properties), header_list.stream_list[video_stream_num].stream_format); 
		StringCbPrintf(video_info, sizeof(video_info), L"AVI: %s (%s), %s (%s)", audio_name, audio_properties, video_name, video_properties);
		video_output->extended(VIDUSER_SET_INFOSTRINGW,(INT_PTR)video_info,0);
	}
	else if (audio_decoder)
	{
		GetAudioCodecName(audio_name, sizeof(audio_name)/sizeof(*audio_name), header_list.stream_list[audio_stream_num].stream_format);
		GetAudioCodecDescription(audio_properties, sizeof(audio_properties)/sizeof(*audio_properties), header_list.stream_list[audio_stream_num].stream_format); 
		StringCbPrintf(video_info, sizeof(video_info), L"AVI: %s (%s)", audio_name, audio_properties);
		video_output->extended(VIDUSER_SET_INFOSTRINGW,(INT_PTR)video_info,0);
	}
	else if (video_decoder)
	{
		GetVideoCodecName(video_name, sizeof(video_name)/sizeof(*video_name), header_list.stream_list[video_stream_num].stream_format);
		GetVideoCodecDescription(video_properties, sizeof(video_properties)/sizeof(*video_properties), header_list.stream_list[video_stream_num].stream_format); 
		StringCbPrintf(video_info, sizeof(video_info), L"AVI: %s (%s)", video_name, video_properties);
		video_output->extended(VIDUSER_SET_INFOSTRINGW,(INT_PTR)video_info,0);
	}
}
void Streams::Reset()
{
	num_audio_streams    = 0;
	num_video_streams    = 0;
	
	current_audio_stream = 0;
}

void Streams::AddAudioStream(int stream_num)
{
	audio_streams[num_audio_streams++]=stream_num;
}

void Streams::AddVideoStream(int stream_num)
{
	video_streams[num_video_streams++]=stream_num;
}

void Streams::SetAudioStream(int stream_num)
{
	for (int i=0;i<num_audio_streams;i++)
	{
		if (audio_streams[i] == stream_num)
			current_audio_stream=i;
	}
}

void Streams::SetVideoStream(int stream_num)
{
	for (int i=0;i<num_video_streams;i++)
	{
		if (video_streams[i] == stream_num)
			current_video_stream=i;
	}
}

int Streams::getNumAudioTracks()
{
	return num_audio_streams;
}

void Streams::enumAudioTrackName(int n, char *buf, int size)
{
	StringCchPrintfA(buf, size, "Audio Stream %d", n);
}

int Streams::getCurAudioTrack()
{
	return current_audio_stream;
}

int Streams::getNumVideoTracks()
{
	return num_video_streams;
}

void Streams::enumVideoTrackName(int n, char *buf, int size)
{
	StringCchPrintfA(buf, size, "Video Stream %d", n);
}

int Streams::getCurVideoTrack()
{
	return current_video_stream;
}

void Streams::setAudioTrack(int n)
{
	SetEvent(audio_break);
	WaitForSingleObject(audio_break_done, INFINITE);

	int i = audio_streams[n];
	const nsavi::STRL &stream = header_list.stream_list[i];
	if (audio_decoder)
	{
		audio_decoder->Close();
		audio_decoder=0;
	}

	audio_decoder = FindAudioDecoder(header_list.avi_header, stream);
	if (audio_decoder)
	{
		current_audio_stream = n;
		audio_stream_num = i;
		video_only=0;  // TODO! need to do more to get this to work if we are switching FROM video_only
	}
	else
	{
		video_only; // TODO! need to do more to get this to work here
	}

	SetEvent(audio_resume);
	WaitForSingleObject(audio_break_done, INFINITE);

	SetVideoInfoText();
}

void Streams::setVideoTrack(int n)
{
	// TODO: need to VideoBreak, destroy decoder, create new one and update video_stream_num
}


bool SingleReaderLoop(nsavi::Demuxer &demuxer, nsavi::avi_reader *reader, nsavi::SeekTable *&audio_seek_table, nsavi::SeekTable *&video_seek_table)
{
	const void *input_buffer = 0;
	uint16_t type = 0;
	uint32_t input_buffer_bytes = 0;
	bool idx1_searched=false;

	HANDLE events[] = { killswitch, seek_event, audio_break, audio_resume };
	void *data;
	uint32_t data_size;
	uint32_t data_type;
	int waitTime = 0;
	for (;;)
	{
		int ret = WaitForMultipleObjects(4, events, FALSE, waitTime);
		if (ret == WAIT_OBJECT_0)
		{
			break;
		}
		else if (ret == WAIT_OBJECT_0+1)
		{
			volatile LONG _this_seek_position;
			do
			{
				InterlockedExchange(&_this_seek_position, seek_position);
				if (_this_seek_position != -1)
				{
					int this_seek_position = _this_seek_position;
					ResetEvent(seek_event); // reset this first so nothing aborts on it
					if (!idx1_searched) 
					{
						nsavi::IDX1 *index;
						ret = demuxer.GetSeekTable(&index);
						if (ret == nsavi::READ_OK)
						{
							if (video_seek_table)
								video_seek_table->AddIndex(index);
							if (audio_seek_table)
								audio_seek_table->AddIndex(index);
						}
						idx1_searched=true;							
					}

					uint64_t index_position, start_time;

					while (video_seek_table && video_seek_table->GetIndexLocation(this_seek_position, &index_position, &start_time))
					{
						nsavi::INDX *next_index=0;
						if (demuxer.GetIndexChunk(&next_index, index_position) == 0)
						{
							video_seek_table->AddIndex(next_index, start_time); // seek table takes ownership
							free(next_index);
						}
					}

					while (audio_seek_table && audio_seek_table->GetIndexLocation(this_seek_position, &index_position, &start_time))
					{
						nsavi::INDX *next_index=0;
						if (demuxer.GetIndexChunk(&next_index, index_position) == 0)
						{
							audio_seek_table->AddIndex(next_index, start_time); // seek table takes ownership
							free(next_index);
						}
					}

					if (video_seek_table)
					{
						int curr_time = GetOutputTime();
						int direction = (curr_time < this_seek_position)?nsavi::SeekTable::SEEK_FORWARD:nsavi::SeekTable::SEEK_BACKWARD;
						const nsavi::SeekEntry *video_seek_entry=video_seek_table->GetSeekPoint(this_seek_position, curr_time, direction);
						if (video_seek_entry)
						{
							Video_Break();
							if (video_only)
							{
								demuxer.Seek(video_seek_entry->file_position, video_seek_entry->absolute, reader);
								video_clock.Seek(this_seek_position);
							}
							else if (audio_seek_table)
							{
								const nsavi::SeekEntry *audio_seek_entry=audio_seek_table->GetSeekPoint(this_seek_position);
								if (audio_seek_entry)
								{
									if (audio_seek_entry->file_position < video_seek_entry->file_position)
										demuxer.Seek(audio_seek_entry->file_position, audio_seek_entry->absolute, reader);
									else
										demuxer.Seek(video_seek_entry->file_position, video_seek_entry->absolute, reader);
									audio_decoder->Flush();
									out.Flush(this_seek_position);
								}
							}
							video_total_time = video_seek_entry->stream_time;
							Video_Flush();
						}
					}
					else if (audio_seek_table)
					{
						int curr_time = GetOutputTime();
						int direction = (curr_time < this_seek_position)?nsavi::SeekTable::SEEK_FORWARD:nsavi::SeekTable::SEEK_BACKWARD;
						const nsavi::SeekEntry *audio_seek_entry=audio_seek_table->GetSeekPoint(this_seek_position, curr_time, direction);
						if (audio_seek_entry)
						{
							demuxer.Seek(audio_seek_entry->file_position, audio_seek_entry->absolute, reader);
							audio_decoder->Flush();
							out.Flush(this_seek_position);
						}
					}
				}
			} while (InterlockedCompareExchange(&seek_position, -1, _this_seek_position) != _this_seek_position); // loop again if seek point changed
		}
		else if (ret == WAIT_OBJECT_0+2)
		{ // audio break
			ResetEvent(audio_break);
			SetEvent(audio_break_done);
			waitTime = INFINITE;
			continue;
		}
		else if (ret == WAIT_OBJECT_0+3)
		{ // audio resume
			ResetEvent(audio_resume);
			SetEvent(audio_break_done);
			waitTime = 0;
			continue;
		}
		else if (ret != WAIT_TIMEOUT)
		{
			break;
		}

		if (input_buffer_bytes) // TODO: read ahead in situation where there is one giant audio chunk for the entire movie
		{
			if (!OnAudio(type, &input_buffer, &input_buffer_bytes))
			{
				return false;
			}
			if (input_buffer_bytes == 0)
			{
				free(data);
				data = NULL;
			}
		}
		else
		{
			ret = demuxer.GetNextMovieChunk(reader, &data, &data_size, &data_type);
			if (ret != nsavi::READ_OK)
			{
				break;
			}

			int stream_number = GetStreamNumber(data_type);
			type = (data_type>>16);
			if (stream_number == audio_stream_num)
			{
				input_buffer = (const void *)data;
				input_buffer_bytes = data_size;
				if (!OnAudio(type, &input_buffer, &input_buffer_bytes))
				{
					return false;
				}
				if (input_buffer_bytes == 0)
				{
					free(data);
					data = NULL;
				}
			}
			else if (stream_number == video_stream_num)
			{
				OnVideo(type, data, data_size);
				data = NULL;
			}
			else
			{
				free(data);
				data = NULL;
			}
		}
	}
	return true;
}

bool MultiReaderLoop(nsavi::Demuxer &demuxer, nsavi::avi_reader *reader, nsavi::avi_reader *video_reader, nsavi::SeekTable *&audio_seek_table, nsavi::SeekTable *&video_seek_table)
{
	demuxer.SeekToMovieChunk(video_reader);

	CreateVideoReaderThread(&demuxer, video_reader);

	const void *input_buffer = 0;
	uint16_t type = 0;
	uint32_t input_buffer_bytes = 0;
	bool idx1_searched=false;

	HANDLE events[] = { killswitch, seek_event, audio_break, audio_resume};
	void *data;
	uint32_t data_size;
	uint32_t data_type;
	int waitTime=0;
	for (;;)
	{
		int ret = WaitForMultipleObjects(4, events, FALSE, waitTime);
		if (ret == WAIT_OBJECT_0)
		{
			break;
		}
		else if (ret == WAIT_OBJECT_0+1)
		{
			volatile LONG _this_seek_position;
			do
			{
				InterlockedExchange(&_this_seek_position, seek_position);
				if (_this_seek_position != -1)
				{
					int this_seek_position = _this_seek_position;
					ResetEvent(seek_event); // reset this first so nothing aborts on it
					if (!idx1_searched) 
					{
						nsavi::IDX1 *index;
						ret = demuxer.GetSeekTable(&index);
						if (ret == nsavi::READ_OK)
						{
							video_seek_table->AddIndex(index);
							audio_seek_table->AddIndex(index);
						}
						idx1_searched=true;							
					}

					uint64_t index_position, start_time;
					while (video_seek_table->GetIndexLocation(this_seek_position, &index_position, &start_time))
					{
						nsavi::INDX *next_index=0;
						if (demuxer.GetIndexChunk(&next_index, index_position) == 0)
						{
							video_seek_table->AddIndex(next_index, start_time); // seek table takes ownership
							free(next_index);
						}
					}

					while (audio_seek_table->GetIndexLocation(this_seek_position, &index_position, &start_time))
					{
						nsavi::INDX *next_index=0;
						if (demuxer.GetIndexChunk(&next_index, index_position) == 0)
						{
							audio_seek_table->AddIndex(next_index, start_time); // seek table takes ownership
							free(next_index);
						}
					}

					int curr_time = GetOutputTime();
					int direction = (curr_time < this_seek_position)?nsavi::SeekTable::SEEK_FORWARD:nsavi::SeekTable::SEEK_BACKWARD;
					const nsavi::SeekEntry *video_seek_entry=video_seek_table->GetSeekPoint(this_seek_position, curr_time, direction);
					if (video_seek_entry)
					{
						Video_Break();
						demuxer.Seek(video_seek_entry->file_position, video_seek_entry->absolute, video_reader);
						const nsavi::SeekEntry *audio_seek_entry=audio_seek_table->GetSeekPoint(this_seek_position);
						if (audio_seek_entry)
						{
							demuxer.Seek(audio_seek_entry->file_position, audio_seek_entry->absolute, reader);
							audio_decoder->Flush();
							out.Flush(this_seek_position);
						}
						video_total_time = video_seek_entry->stream_time;
						Video_Flush();
					}
				}
			} while (InterlockedCompareExchange(&seek_position, -1, _this_seek_position) != _this_seek_position); // loop again if seek point changed
		}
		else if (ret == WAIT_OBJECT_0+2)
		{ // audio break
			ResetEvent(audio_break);
			SetEvent(audio_break_done);
			waitTime = INFINITE;
			continue;
		}
		else if (ret == WAIT_OBJECT_0+3)
		{ // audio resume
			ResetEvent(audio_resume);
			SetEvent(audio_break_done);
			waitTime = 0;
			continue;
		}
		else if (ret != WAIT_TIMEOUT)
		{
			break;
		}

		if (input_buffer_bytes) // TODO: read ahead in situation where there is one giant audio chunk for the entire movie
		{
			if (!OnAudio(type, &input_buffer, &input_buffer_bytes))
			{
				return false;
			}
			if (input_buffer_bytes == 0)
			{
				free(data);
				data = NULL;
			}
		}
		else
		{
			ret = demuxer.GetNextMovieChunk(reader, &data, &data_size, &data_type, audio_stream_num);
			if (ret != nsavi::READ_OK)
			{
				break;
			}

			int stream_number = GetStreamNumber(data_type);
			type = (data_type>>16);

			if (stream_number == audio_stream_num && type != 0x7869) // ignore 'ix'
			{
				input_buffer = (const void *)data;
				input_buffer_bytes = data_size;
				if (!OnAudio(type, &input_buffer, &input_buffer_bytes))
				{
					return false;
				}

				if (input_buffer_bytes == 0)
				{
					free(data);
					data = NULL;
				}
			}
			else
			{
				free(data);
				data = NULL;
			}
		}
	}

	return true;
}

void PlayLoop(nsavi::avi_reader *reader, bool multiple_readers)
{
	AVIReaderWin32 video_reader;
	uint32_t riff_type;

	audio_decoder=0;
	video_decoder=0;
	nsavi::SeekTable *video_seek_table = 0, *audio_seek_table = 0;
	nsavi::Demuxer demuxer(reader);
	audio_opened=false;
	int audio_bitrate=0;
	streams.Reset();

	out.Init(plugin.outMod);
	if (!video_output)
		video_output = (IVideoOutput *)SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_GET_IVIDEOOUTPUT);
	audio_stream_num = 65536;
	video_stream_num=65536; // purposefully too big value
	Video_Init();

	if (demuxer.GetRIFFType(&riff_type) == nsavi::READ_OK)
	{
		bool audio_no_decoder=false;
		bool video_no_decoder=false;
		if (demuxer.GetHeaderList(&header_list) == nsavi::READ_OK)
		{
			// find available codecs
			for (uint32_t i=0;i!=header_list.stream_list_size;i++)
			{
				const nsavi::STRL &stream = header_list.stream_list[i];
				if (stream.stream_header)
				{
					if (stream.stream_header->stream_type == nsavi::stream_type_audio)
					{
						nsavi::audio_format *f = (nsavi::audio_format *)stream.stream_format;
						if (f)
						{
							stats.audio_types[f->format]++;

							streams.AddAudioStream(i);
							if (!audio_decoder)
							{ // TODO: check priority
								audio_decoder = FindAudioDecoder(header_list.avi_header, stream);
								if (audio_decoder)
								{
									streams.SetAudioStream(i);
									audio_stream_num = i;
									video_only=0; 
								}
								else
									audio_no_decoder = true;

								if (stream.stream_header->length && !stream.stream_header->sample_size && stream.stream_header->rate)
									g_duration = (uint64_t)stream.stream_header->length * (uint64_t)stream.stream_header->scale * 1000ULL / (uint64_t)stream.stream_header->rate;
								audio_bitrate = MulDiv(f->average_bytes_per_second, 8, 1000);
								plugin.SetInfo(audio_bitrate, -1, -1, -1);
							}
						}
					}
					else if (stream.stream_header->stream_type == nsavi::stream_type_video)
					{
						nsavi::video_format *f = (nsavi::video_format *)stream.stream_format;
						if (f)
						{
							stats.video_fourccs[f->compression]++;

							streams.AddVideoStream(i);
							if (!video_decoder)
							{ // TODO: check priority
								video_decoder = FindVideoDecoder(header_list.avi_header, stream);
								if (video_decoder)
								{
									video_stream_num = i;
									streams.SetVideoStream(i);
								}
								else
									video_no_decoder = true;
								if (g_duration == -1 && stream.stream_header->rate)
									g_duration = (uint64_t)stream.stream_header->length * (uint64_t)stream.stream_header->scale * 1000ULL / (uint64_t)stream.stream_header->rate;
							}
						}
					}
				}
			}
		}

		if (AGAVE_API_STATS)
		{
			uint32_t audio_format = stats.GetAudioStat();
			uint32_t video_format = stats.GetVideoStat();
			AGAVE_API_STATS->SetStat(api_stats::AVI_AUDIO_FORMAT, audio_format);
			AGAVE_API_STATS->SetStat(api_stats::AVI_VIDEO_FOURCC, video_format);
		}

		if ((audio_no_decoder || video_no_decoder) && CheckDSHOW())
		{
			// use in_dshow to play this one
			HANDLE mainThread = WASABI_API_APP->main_getMainThreadHandle();
			if (mainThread)
			{
				Video_Stop();
				if (audio_decoder)
				{
					audio_decoder->Close();
					audio_decoder=0;
				}

				Video_Close();
				delete video_seek_table;
				delete audio_seek_table;
				wchar_t *fn = (wchar_t *)calloc(1024, sizeof(wchar_t *));
				reader->GetFilename(fn, 1024);
				QueueUserAPC(DSHOWAPC, mainThread, (ULONG_PTR)fn);
				CloseHandle(mainThread);
				return ;
			}
		}

		if (!audio_decoder && !video_decoder)
		{
			goto btfo;
		}

		if (!audio_decoder)
		{
			video_only=1;
			video_clock.Start();
		}
	}

	else
	{
		goto btfo;
	}
	SetVideoInfoText();


	video_output->extended(VIDUSER_SET_TRACKSELINTERFACE, (INT_PTR)&streams, 0);

	if (video_stream_num != 65536)
		video_seek_table = new nsavi::SeekTable(video_stream_num, !!video_decoder, &header_list);
	if (audio_stream_num != 65536)
		audio_seek_table = new nsavi::SeekTable(audio_stream_num, false, &header_list);

	uint64_t content_length = reader->GetContentLength();
	if (content_length && g_duration)
	{
		int total_bitrate = (int)(8ULL * content_length / (uint64_t)g_duration);
		plugin.SetInfo(total_bitrate, -1, -1, -1);
	}
	else if (header_list.avi_header->max_bytes_per_second)
	{
		int total_bitrate = MulDiv(header_list.avi_header->max_bytes_per_second, 8, 1000);
		plugin.SetInfo(total_bitrate, -1, -1, -1);
	}
	else
	{
		// use seek table for bitrate?
	}

	if (demuxer.FindMovieChunk() != nsavi::READ_OK)
	{
		goto btfo;
	}

	if (multiple_readers && video_decoder && !video_only)
	{
		wchar_t fn[MAX_PATH] = {0};
		reader->GetFilename(fn, MAX_PATH);
		if (video_reader.Open(fn) == nsavi::READ_OK)
		{
			MultiReaderLoop(demuxer, reader, &video_reader, audio_seek_table, video_seek_table);
		}
		else
			SingleReaderLoop(demuxer, reader, audio_seek_table, video_seek_table);		
	}
	else
		SingleReaderLoop(demuxer, reader, audio_seek_table, video_seek_table);

	if (audio_opened && WaitForSingleObject(killswitch, 0) == WAIT_TIMEOUT)
	{
		out.Write(0, 0);
		out.WaitWhilePlaying();
	}
btfo:
	if (WaitForSingleObject(killswitch, 0) == WAIT_TIMEOUT)
		PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
	Video_Stop();
	if (audio_decoder)
	{
		audio_decoder->Close();
		audio_decoder=0;
		if (audio_opened)
			out.Close();
	}

	Video_Close();
	video_reader.Close();
	delete video_seek_table;
	delete audio_seek_table;
}

DWORD CALLBACK AVIPlayThread(LPVOID param)
{
	if (!audio_break)
		audio_break = CreateEvent(0, TRUE, FALSE, 0);

	if (!audio_resume)
		audio_resume = CreateEvent(0, TRUE, FALSE, 0);

	if (!audio_break_done)
		audio_break_done = CreateEvent(0, FALSE, FALSE, 0);

	wchar_t *filename = (wchar_t *)param;
	if (PathIsURLW(filename))
	{
		AVIReaderHTTP reader(killswitch, seek_event);
		if (reader.Open(filename) != nsavi::READ_OK || reader.Connect() != nsavi::READ_OK)
		{
			if (WaitForSingleObject(killswitch, 200) == WAIT_TIMEOUT)
				PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
		}
		else
		{
			PlayLoop(&reader, false);
			reader.Close();
		}
	}
	else
	{
		AVIReaderWin32 reader;
		if (reader.Open(filename) != nsavi::READ_OK)
		{
			if (WaitForSingleObject(killswitch, 200) == WAIT_TIMEOUT)
				PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);

		}
		else
		{
			wchar_t root[4] = {0};
			StringCchCopy(root, 4, filename);
			UINT drive_type = GetDriveType(root);
			if (drive_type == DRIVE_CDROM)
				PlayLoop(&reader, false);
			else
				PlayLoop(&reader, true);
			reader.Close();
		}

	}
	free(filename);
	return 0;
}
