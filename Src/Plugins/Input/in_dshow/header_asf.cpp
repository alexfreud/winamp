#include <windows.h>
#include "header_asf.h"
#include <mmsystem.h>
#include <stdio.h>
#include <bfc/platform/types.h>
#pragma pack(1)

///////////////////////
// MS GUID definition
///////////////////////
#ifndef GUID_DEFINED
#define GUID_DEFINED
// Size of GUID is 16 bytes!
typedef struct {
  uint32_t	Data1;		// 4 bytes
  uint16_t	Data2;		// 2 bytes
  uint16_t	Data3;		// 2 bytes
  uint8_t		Data4[8];	// 8 bytes
} GUID_t;
#endif

///////////////////////
// ASF Object Header 
///////////////////////
typedef struct {
  uint8_t guid[16];
  uint64_t size;
} ASF_obj_header_t;

////////////////
// ASF Header 
////////////////
typedef struct {
  ASF_obj_header_t objh;
  uint32_t cno; // number of subchunks
  uint8_t v1; // unknown (0x01)
  uint8_t v2; // unknown (0x02)
} ASF_header_t;

/////////////////////
// ASF File Header 
/////////////////////
typedef struct {
  uint8_t client[16]; // Client GUID
  uint64_t file_size;
  uint64_t creat_time; //File creation time FILETIME 8
  uint64_t packets;    //Number of packets UINT64 8
  uint64_t end_timestamp; //Timestamp of the end position UINT64 8
  uint64_t duration;  //Duration of the playback UINT64 8
  uint32_t start_timestamp; //Timestamp of the start position UINT32 4
  uint32_t preroll; //Time to bufferize before playing UINT32 4
  uint32_t flags; //Unknown, maybe flags ( usually contains 2 ) UINT32 4
  uint32_t packetsize; //Size of packet, in bytes UINT32 4
  uint32_t packetsize2; //Size of packet ( confirm ) UINT32 4
  uint32_t frame_size; //Size of uncompressed video frame UINT32 4
} ASF_file_header_t;

///////////////////////
// ASF Stream Header
///////////////////////
typedef struct {
  uint8_t type[16]; // Stream type (audio/video) GUID 16
  uint8_t concealment[16]; // Audio error concealment type GUID 16
  uint64_t unk1; // Unknown, maybe reserved ( usually contains 0 ) UINT64 8
  uint32_t type_size; //Total size of type-specific data UINT32 4
  uint32_t stream_size; //Size of stream-specific data UINT32 4
  uint16_t stream_no; //Stream number UINT16 2
  uint32_t unk2; //Unknown UINT32 4
} ASF_stream_header_t;

///////////////////////////
// ASF Content Description
///////////////////////////
typedef struct {
  uint16_t title_size;
  uint16_t author_size;
  uint16_t copyright_size;
  uint16_t comment_size;
  uint16_t rating_size;
} ASF_content_description_t;

////////////////////////
// ASF Segment Header 
////////////////////////
typedef struct {
  uint8_t streamno;
  uint8_t seq;
  uint32_t x;
  uint8_t flag;
} ASF_segmhdr_t;

//////////////////////
// ASF Stream Chunck
//////////////////////
typedef struct {
  uint16_t	type;
  uint16_t	size;
  uint32_t	sequence_number;
  uint16_t	unknown;
  uint16_t	size_confirm;
} ASF_stream_chunck_t;

#pragma pack()

// Definition of the differents type of ASF streaming
typedef enum {
  ASF_Unknown_e,
  ASF_Live_e,
  ASF_Prerecorded_e,
  ASF_Redirector_e,
  ASF_PlainText_e
} ASF_StreamType_e;

#define	ASF_LOAD_GUID_PREFIX(guid)	(*(uint32_t *)(guid))

#define ASF_GUID_PREFIX_audio_stream	0xF8699E40
#define ASF_GUID_PREFIX_video_stream	0xBC19EFC0
#define ASF_GUID_PREFIX_audio_conceal_none 0x49f1a440
#define ASF_GUID_PREFIX_audio_conceal_interleave 0xbfc3cd50
#define ASF_GUID_PREFIX_header		0x75B22630
#define ASF_GUID_PREFIX_data_chunk	0x75b22636
#define ASF_GUID_PREFIX_index_chunk	0x33000890
#define ASF_GUID_PREFIX_stream_header	0xB7DC0791
#define ASF_GUID_PREFIX_header_2_0	0xD6E229D1
#define ASF_GUID_PREFIX_file_header	0x8CABDCA1
#define	ASF_GUID_PREFIX_content_desc	0x75b22633

HeaderAsf::HeaderAsf() 
{  
}

int HeaderAsf::getInfos(const wchar_t *filename, bool checkMetadata) 
{
  ASF_header_t asfh;
  DWORD bytesRead = 0;
  unsigned char asfhdrguid[16]={0x30,0x26,0xB2,0x75,0x8E,0x66,0xCF,0x11,0xA6,0xD9,0x00,0xAA,0x00,0x62,0xCE,0x6C};

  HANDLE fh=CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  if (fh==INVALID_HANDLE_VALUE)
    return 0;

  ReadFile(fh,&asfh, sizeof(ASF_header_t), &bytesRead, NULL);

  // check for ASF guid
  if(memcmp(asfhdrguid,asfh.objh.guid,16) || asfh.cno>256) 
  {
    CloseHandle(fh);
    return 0;
  }

  for(int i=0;i<(int)asfh.cno;i++) {
    int pos = SetFilePointer(fh, 0, 0, FILE_CURRENT);

    ASF_obj_header_t objh;
	bytesRead = 0;
    ReadFile(fh, &objh, sizeof(objh), &bytesRead, NULL);

    switch(ASF_LOAD_GUID_PREFIX(objh.guid)) {
    case ASF_GUID_PREFIX_file_header:
      {
        ASF_file_header_t fileh;
		bytesRead = 0;
        ReadFile(fh, &fileh, sizeof(fileh), &bytesRead, NULL);
        length=(int)(fileh.duration/10000);
      }
      break;
    case ASF_GUID_PREFIX_stream_header:
      {
        ASF_stream_header_t streamh;
		bytesRead = 0;
        ReadFile(fh, &streamh, sizeof(streamh), &bytesRead, NULL);
        switch(ASF_LOAD_GUID_PREFIX(streamh.type)) 
        {
        case ASF_GUID_PREFIX_audio_stream: 
          {
			WAVEFORMATEX wfe = {0};
			bytesRead = 0;
            ReadFile(fh, &wfe, sizeof(wfe), &bytesRead, NULL);
            audio_bps=wfe.wBitsPerSample;
            audio_srate=wfe.nSamplesPerSec;
            audio_nch=wfe.nChannels;
            has_audio=true;
          }
          break;
        case ASF_GUID_PREFIX_video_stream: 
          {
			bytesRead = 0;
            ReadFile(fh, &video_w, sizeof(video_w), &bytesRead, NULL);
			bytesRead = 0;
            ReadFile(fh, &video_h, sizeof(video_h), &bytesRead, NULL);
            has_video=true;
          }
          break;
        }
      }
      break;
    }
    SetFilePointer(fh, pos+(int)objh.size, NULL, FILE_BEGIN);
  }
  CloseHandle(fh);
  return 1;
}
