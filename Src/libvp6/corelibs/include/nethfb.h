#ifndef _nethfb_h
#define _nethfb_h

#include "duck_hfb.h"

#ifndef NETHFB

#define lHFB_GetDataSize HFB_GetDataSize
#define lHFB_GetStreamingData HFB_GetStreamingData
#define lHFB_ReleaseStreamingData HFB_ReleaseStreamingData
#define lHFB_ReadData HFB_ReadData
#define lHFB_WhatsAhead HFB_WhatsAhead
#define lHFB_GetAudioInfo HFB_GetAudioInfo
#define lHFB_GetInitialFrames HFB_GetInitialFrames
#define lHFB_GetSamplesPerFrame HFB_GetSamplesPerFrame
#define lHFB_GetFrameRates HFB_GetFrameRates
#define lHFB_GetDRMData HFB_GetDRMData
#define lHFB_LoadIndex HFB_LoadIndex
#define lHFB_ParseFile HFB_ParseFile
#define lHFB_Init HFB_Init
#define lHFB_Exit HFB_Exit
#define lHFB_FindFile HFB_FindFile
#define lHFB_OpenFile HFB_OpenFile
#define lHFB_SeekToIndex HFB_SeekToIndex
#define lHFB_BeginLoadIndex HFB_BeginLoadIndex
#define lHFB_FinishLoadIndex HFB_FinishLoadIndex
#define lHFB_ReadMoreIndex HFB_ReadMoreIndex
#define lHFB_ParseBigIndex HFB_ParseBigIndex
#define lHFB_CloseFile HFB_CloseFile
#define lHFB_GetFileInfo HFB_GetFileInfo
#define lHFB_ResetStreams HFB_ResetStreams
#define lHFB_GetStream HFB_GetStream
#define lHFB_ReleaseStream HFB_ReleaseStream
#define lHFB_GetStreamInfo HFB_GetStreamInfo
#define lHFB_CreateBuffer HFB_CreateBuffer
#define lHFB_DestroyBuffer HFB_DestroyBuffer
#define lHFB_ResetBuffer HFB_ResetBuffer
#define lHFB_SetBufferMode HFB_SetBufferMode
#define lHFB_QueueOpenFile HFB_QueueOpenFile
#define lHFB_GetBufferPerCentFull HFB_GetBufferPerCentFull
#define lHFB_GetmovieSize HFB_GetmovieSize
#define lHFB_InitBuffer HFB_InitBuffer
#define lHFB_GetBufferSpace HFB_GetBufferSpace
#define lHFB_FillBuffer HFB_FillBuffer
#define lHFB_GetBufferStatus HFB_GetBufferStatus
#define lHFB_FramestoNextKeyFrame HFB_FramestoNextKeyFrame
#define lHFB_FrameToChunk HFB_FrameToChunk
#define lHFB_PreviousKeyFrame HFB_PreviousKeyFrame
#define lHFB_GetIndexFlags HFB_GetIndexFlags
#define lHFB_AddIndexFlags HFB_AddIndexFlags
#define lHFB_GetDataPosition HFB_GetDataPosition
#define lHFB_ConditionBuffer HFB_ConditionBuffer
#define lHFB_WalkFlags HFB_WalkFlags
#define lHFB_isVideoKeyFrame HFB_isVideoKeyFrame

#define lHFB_GetStreamParentBuffer HFB_GetStreamParentBuffer
#define lHFB_GetStreamParentFile HFB_GetStreamParentFile

#define lHFB_GetStreamRateAndScale HFB_GetStreamRateAndScale
#define lHFB_GetStreamFCCs HFB_GetStreamFCCs
#define lHFB_GetStreamSampleSize HFB_GetStreamSampleSize
#define lHFB_GetLastError HFB_GetLastError

#else
#if defined(__cplusplus)
extern "C" {
#endif

/* main HFB initialization and exit routines */

int lHFB_Init(int ,int ,int );
void lHFB_Exit(void);

/* FWG 9-13-2000 */
int lHFB_SeekToIndex(HFB_FILE_HANDLE FileHandle);
int lHFB_BeginLoadIndex(HFB_BUFFER_HANDLE dckPtr, int size);
int lHFB_FinishLoadIndex(HFB_BUFFER_HANDLE dckPtr, void *data, int size);

/* open specified file, parse its header(s) and load the index */
HFB_FILE_HANDLE lHFB_OpenFile(
	const char *fileName,
	HFB_BUFFER_HANDLE bufferHandle,
	unsigned int userData
	);

/* the following three functions, findfile, parsefile and loadindex,
	are encapsulated by openfile, they are provided as a convenience
    for use on systems with asynchronous i/o */

//HFB_FILE_HANDLE lHFB_FindFile(const char *fileName, unsigned int userData);

int lHFB_ParseFile(
	HFB_FILE_HANDLE fileHandle,
	HFB_BUFFER_HANDLE bufferHandle
	);

int lHFB_LoadIndex(
	HFB_FILE_HANDLE fileHandle,
	HFB_BUFFER_HANDLE bufferHandle
	);

void lHFB_CloseFile(HFB_FILE_HANDLE fHnd);

HFB_FILE_INFO *lHFB_GetFileInfo(HFB_FILE_HANDLE fileHandle);

HFB_BUFFER_HANDLE lHFB_CreateBuffer(
	int sizeOfBuffer,
	int reserved
	);

void lHFB_InitBuffer(
	HFB_BUFFER_HANDLE bufferHandle,
	HFB_FILE_HANDLE fileToLoad,
	int startFrame,
	int initialReadSize
	);

int lHFB_FillBuffer(
	HFB_BUFFER_HANDLE bufferHandle,
	int maxToRead,
	int frameSyncCounter
	);

void lHFB_DestroyBuffer(HFB_BUFFER_HANDLE bufferHandle);

void lHFB_ResetStreams(HFB_BUFFER_HANDLE bufferHandle);

int lHFB_SetBufferMode(
	HFB_BUFFER_HANDLE ,
	hfbBufferMode mode 
	);

int lHFB_GetBufferPerCentFull(HFB_BUFFER_HANDLE );
int lHFB_GetmovieSize(HFB_BUFFER_HANDLE );
int lHFB_GetBufferSpace(HFB_BUFFER_HANDLE );
hfbBufferStatus lHFB_GetBufferStatus(HFB_BUFFER_HANDLE );

int lHFB_ConditionBuffer(
	HFB_BUFFER_HANDLE bufferHandle,
	int bufferSize,
	int reserved);

#define lHFB_ResetBuffer lHFB_ConditionBuffer

/* get a stream reference handle */
HFB_STREAM_HANDLE lHFB_GetStream(
	HFB_FILE_HANDLE fileHandle,
	const char *StreamNameOrNull,
	int streamNumber,
	unsigned int streamType);

/* relinquish reference to stream */
void lHFB_ReleaseStream(HFB_STREAM_HANDLE streamHandle);

/* get a pointer to stream info struct */
HFB_STREAM_INFO *lHFB_GetStreamInfo(HFB_STREAM_HANDLE );

#define lHFB_GetStreamLength(strmh) \
	lHFB_GetStreamInfo(strmh)->lLength

#define lHFB_GetStreamName(strmh) \
	lHFB_GetStreamInfo(strmh)->szName

/* get pointer to buffered record and length */
HFB_DATA_HANDLE lHFB_GetStreamingData(
	HFB_STREAM_HANDLE streamHandle,
	void **ptrToPtr,
	int *ptrToLength,
	hfbDirection directionToMove,
	int framesToMove
	);

/* release buffer space occupied by record */
void lHFB_ReleaseStreamingData(
	HFB_BUFFER_HANDLE bufferHandle,
	HFB_DATA_HANDLE recordToRelease);

/* read data directly from a file into a 
	supplied buffer, limit is set by initial value
	of *ptrToLength */
int lHFB_ReadData(
	HFB_STREAM_HANDLE streamHandle,
	void *ptrToBuffer,
	int *ptrToLength,
	hfbDirection directionToMove,
	int framesToMove);

int lHFB_FramestoNextKeyFrame(
	HFB_STREAM_HANDLE streamHandle,
	int recordHandle,
	int *numberOfRecordsSpanned
	);

int lHFB_FrameToChunk(
	HFB_STREAM_HANDLE streamHandle,
	int frameNumber
	);

/* get the frameNumber of the keyframe 
	at or prior to the specified frameNumber */
int lHFB_PreviousKeyFrame(
	HFB_STREAM_HANDLE streamHandle,
	int frameNumber
	);

/* get the HFB index flags for the specified record/frame */

int lHFB_GetIndexFlags(
	HFB_STREAM_HANDLE ,
	hfbFrameNumber frameNumberType,
	int recordHandleOrFrameNumber);

/* add the HFB index flags for the specified record/frame */

int lHFB_AddIndexFlags(
	HFB_STREAM_HANDLE ,
	hfbFrameNumber frameNumberType,
	int recordHandleOrFrameNumber,
	int flagsToAdd);


/* get current data position
   video - frameNumber
   audio - sampleCount */
int lHFB_GetDataPosition(
	HFB_STREAM_HANDLE streamHandle,
	HFB_DATA_HANDLE dataRecordHandle
	);

/* get information about audio stream */
DKWAVEFORM *lHFB_GetAudioInfo(
	HFB_STREAM_HANDLE nStream, 
	int *NumChannels, 
	int *SamplesPerSec, 
	int *BytesPerSec, 
	int *wFormat);

/* get the amount of audio skew 
	expressed in records */
int lHFB_GetInitialFrames(
	HFB_STREAM_HANDLE videoStream,
	HFB_STREAM_HANDLE audioStream
	);

/* get the number of audio frames elapsed 
   during a single video frame */
int lHFB_GetSamplesPerFrame(
	HFB_STREAM_HANDLE videoStream,
	HFB_STREAM_HANDLE audioStream
	);

/* get video frame rate and 
   calculated audio skew (in audio samples) */
void lHFB_GetFrameRates(
	HFB_STREAM_HANDLE videoStream,
	HFB_STREAM_HANDLE audioStream,
	int *ptrToIntegerFrameRate,
	int *ptrToEstimatedAudioSampleSkew);

/* */
int lHFB_GetDRMData(
	HFB_FILE_HANDLE dckPtr, 
	unsigned int* pOutEncFourCC, 
	int* pOutLength, 
	unsigned char** ppOutData);


/*get pointer to stream information streuct */
HFB_STREAM_INFO *lHFB_GetStreamInfo(HFB_STREAM_HANDLE );

/* functions to retrieve parent buffer 
   and file of a given stream*/
HFB_BUFFER_HANDLE lHFB_GetStreamParentBuffer(HFB_STREAM_HANDLE );
HFB_FILE_HANDLE lHFB_GetStreamParentFile(HFB_STREAM_HANDLE);

/* used to precisely calculate rational frame rates 
   for a specific stream */
void lHFB_GetStreamRateAndScale(
	HFB_STREAM_HANDLE xStream,
	int *rate, int *scale
	);

/* get stream type and handler fourCC codes, 
   returns type (not handler) */
unsigned int lHFB_GetStreamFCCs(
	HFB_STREAM_HANDLE xStream,
	unsigned int *type, 
	unsigned int *handler
	);

/* get the last error that occured in HFB */
int lHFB_GetLastError(
    HFB_BUFFER_HANDLE bfHnd, 
    int* lastErrorCode, 
    char errorString[], 
    size_t maxLen
    );

/* get streamSampleSize, <= 0 means variable */
int lHFB_GetStreamSampleSize(HFB_STREAM_HANDLE xStream);

int lHFB_WhatsAhead(HFB_STREAM_HANDLE ,int ,int *);

/* windows 95 dll system abstraction functions */

void lHFB_Setmalloc(
	void *(*mallocFuncPtr)(unsigned int size)
	);

void lHFB_Setcalloc(
	void *(*callocFuncPtr)(unsigned int size, unsigned int number)
	);

void lHFB_Setfree(
	void (*freeFuncPtr)(void *)
	);

void lHFB_Setopen(
	int (*openFuncPtr)(const char *, int,...)
	);

void lHFB_Setclose(
	int (*closeFuncPtr)(int)
	);

void lHFB_Setread(
	int (*readFuncPtr)(int,void *, unsigned int)
	);

void lHFB_Setseek(
	int (*seekFuncPtr)(int,int,int)
	);

#if defined(__cplusplus)
}
#endif

#endif

#endif
