#ifndef _duck_hfb_h
#define _duck_hfb_h


/******************************************************************************\
<table BGCOLOR=#FFC0C0 border=1 WIDTH=100% ><tr><td><b>                                                                              
  duck_hfb.h  </b></td><td><b> 	TrueMotion include file for buffering/streaming libraries </b>
                                                                           
</td></tr><tr><td>&nbsp</td><td>	Version:      6.0.0  
</td></tr><tr><td>&nbsp</td><td>  	Updated:      $Date: 2010/07/23 19:10:44 $                                       
</td></tr><tr><td>&nbsp</td><td>  	Copyright (c) 1994-98, The Duck Corp. All rights reserved.
</td></tr>
</table>
******************************************************************************/



#define lHFB_ HFB_
#define libFunc


#if defined(__cplusplus)
extern "C" {
#endif

#include "duck_bmp.h"
#include "duck_wav.h"
#include <string.h> /* for size_t */

typedef enum dukDirect {     /* direction to move within stream */
    DUK_BACKWARD=-1,         /* move backward from current position */
    DUK_ABSOLUTE=0,          /* move to an absolute position */
    DUK_FORWARD=1,            /* move forward from current position */
    DUK_RESET=2              /* reset stream according to file position */
} hfbDirection;

typedef enum HFB_FileType {  /* type of file opened */
    DUK_NULFILE = 0,         /* not a TrueMotion "S" A/V file */
    DUK_DUKFILE = 1,         /* TrueMotion "S" duk file */
    DUK_AVIFILE = -1,         /* TrueMotion "S" Video for Windows AVI compatible file */
    DUK_QTFILE = 3           /* TrueMotion "S" Quicktime MOV compatible file */
} hfbFileType;

typedef enum HFB_StreamType {/* stream types */
    DUK_UNDEFINED = 0,       /* indeterminate or uninitialized stream */
    DUK_AUDSTREAM = 1,       /* audio stream */
    DUK_VIDSTREAM = 2,       /* video stream */
    DUK_TXTSTREAM = 3        /* text stream */
} hfbStreamType;

typedef enum HFB_Modes {              /* HFB buffering modes */
    HFBMODE_NORMAL =        0,        /* normal play once mode */
    HFBMODE_FORWARDLOOP =   1,        /* forward looping mode (loop back to start) */
    HFBMODE_REVERSE =       2,        /* reverse play once mode (not yet implemented)*/
    HFBMODE_REVERSELOOP =   4,        /* reverse looping mode (not yet implemented)*/
    HFBMODE_QUEUE =         8,        /* file queue mode (not yet implemented)*/
	HFBMODE_FLYINDEX =      16,	      /* HFB does not have to read the AVI index to play the movie */ /* FWG 6-23-99 */
	HFBMODE_ASYNC = 		32,       /* HFB is in asnyc mode. Needed for first read and index stuff */ /* FWG 7-7-99 */
	HFBMODE_ASYNC_FLYINDEX = 64,      /* Asnyc & Fly-Index mode. */ /* FWG 7-7-99 */
	HFBMODE_PSEUDO_ASYNC =  128       /* */
} hfbBufferMode;

typedef enum BUFFERSTATUS {
	HFB_BUSY = 0,
	HFB_COMPLETE
} hfbBufferStatus;

typedef enum PRELOADSPEC {
	HFB_FULL = -2,
	HFB_THREEQUARTERS = -1,
	HFB_NONE = 0
} hfbPreloadSpec;

#define HFB_USE_DEFAULT		0L
#define HFB_RESET_COUNT		-1L
#define HFB_IGNORE_COUNT	-2L

#define HFB_DATA_READ		0x01
#define HFB_DATA_RELEASED	0x02
#define HFB_DATA_KEYFRAME	0x08


typedef struct tHFB_STREAM_INFO{
	hfbStreamType streamType;
	int streamNum;
	int lSampleRate;
	int lStart;
	int lLength;
	char szName[24];
	union {
		DKWAVEFORM WaveformInfo;
        DKBITMAP BitmapInfo;
        DKBITMAP_old  depricatedBitmapInfo; /* please don't use this */
	} a;
	DK_BITMAPINFOHEADER bmih;  /* should be part of union or replace DKBITMAP ... but due to healthy paranoia add here ... */
} HFB_STREAM_INFO, MFP_STREAM_INFO;

typedef struct tHFB_FILE_INFO{
	int lFileSize;
	int lNumStreams;
	char sName[96];
	int lNumFrames;
} HFB_FILE_INFO, MFP_FILE_INFO;

typedef struct tHFBFile *HFB_FILE_HANDLE;
typedef struct tHFBStream *HFB_STREAM_HANDLE;
typedef struct tHFBBuffer *HFB_BUFFER_HANDLE;

typedef int HFB_DATA_HANDLE;

#define DCK_DEFAULT_READSIZE 0
#define DCK_DEFAULT_BUFFSIZE 0

/* main HFB initialization and exit routines */

/*@
@Name			HFB_Init
@Description		Allocate and initialize required data structures.If all three parameters are -1, 
HFB will dynamically allocate objects as needed.
@Return value		0 if success or negative return code.
@*/
libFunc int HFB_Init(
	int maxOpenFiles,		/* maximum of concurently open file objects. */
	int maxOpenStreams,		/* maximum of concurently open streams. */
	int maxOpenBuffers		/* maximum of concurently open buffer objects. */
);



/*@
@Name			HFB_Exit
@Description		free any allocated strcutures, close all files, etc.	
@Return value		none.
@*/
libFunc void HFB_Exit(void);




/*@
@Name 			HFB_SeekToIndex
@Description		Seek to the index in the AVI file
@Return Value		returns 0 on success.
@*/
libFunc int HFB_SeekToIndex(HFB_FILE_HANDLE FileHandle);


/*@
@!Name			HFB_BeginLoadIndex
@Description
@Return value
@*/
libFunc int HFB_BeginLoadIndex(
	HFB_BUFFER_HANDLE dckPtr, 		/* */
	int size				/* */
);


/*@
@!Name			HFB_FinishLoadIndex
@Description
@Return value
@*/
libFunc int HFB_FinishLoadIndex(
	HFB_BUFFER_HANDLE dckPtr, 		/* */
	void *data, 				/* */
	int size				/* */
);






/*@
@!Name			HFB_ParseBigIndex
@Description
@Return value
@*/
libFunc int HFB_ParseBigIndex(
	HFB_BUFFER_HANDLE dckPtr, 
	void *data, 
	int size
);



/*@
@Name			HFB_OpenFile
@Description		open specified file, parse its header(s) and load the index
@Return value		handle to HFB file object.
@*/
libFunc HFB_FILE_HANDLE HFB_OpenFile(
	const char *fileName,			/* file to be opened. */
	HFB_BUFFER_HANDLE bufferHandle,		/* handle to a pre-existing HFB buffer. */
	unsigned int userData			/* user data. */
	);



/* the following three functions, findfile, parsefile and loadindex,
	are encapsulated by openfile, they are provided as a convenience
    for use on systems with asynchronous i/o */

#if 0 // Changed to a static funtion MEW 11-6-03
/*@
@Name			HFB_FindFile
@Description		This function implements a portion of the functionality performed by HFB_OpenFile.	
@Return value		Handle to a HFB file object.
@*/
libFunc HFB_FILE_HANDLE HFB_FindFile(
	const char *fileName, 		/* name of file to open */
	unsigned int userData		/* user data */
	);
#endif


/*@
@Name			HFB_ParseFile
@Description		After a file has been found, and at least a single sector is buffered, it's header can be 
parsedfor the information necessary to describe what the file contains. The combination of loading functions must 
appear in this order  HFB_FindFile, HFB_ParseFile, HFBLoadIndex.
@Return value		none.
@*/
libFunc int HFB_ParseFile(
	HFB_FILE_HANDLE fileHandle,		/* handle to an HFB file object. */
	HFB_BUFFER_HANDLE bufferHandle		/* handle to an HFB buffer object. */
	);



/*@
@Name			HFB_LoadIndex
@Description		Load a TrueMotion file's index into the specified buffer object. 
Must be used in this order ... HFB_FindFile, HFB_ParseFile, HFB_LoadIndex. 
@Return value		0 if successful, -1 if error
@*/
libFunc int HFB_LoadIndex(
	HFB_FILE_HANDLE fileHandle,		/* handle to HFB file object. */
	HFB_BUFFER_HANDLE bufferHandle		/* handle to HFFB buffer object. */
	);


/*@
@Name			HFB_CloseFile
@Description		Close a TrueMotion file (AVI) and release file structures.
@Return value		None.
@*/
libFunc void HFB_CloseFile(
	HFB_FILE_HANDLE fHnd		/* handle to an HFB file object. */
);



/*@
@Name			HFB_GetFileInfo		
@Description		Used to read information about an opened TrueMotion File (AVI).	
@Return value		a pointer to an HFB_FILE_INFO structure describing the indicated file.
@*/
libFunc HFB_FILE_INFO *HFB_GetFileInfo(
	HFB_FILE_HANDLE fileHandle		/* handle to an HFB file object. */
);



/*@
@Name				HFB_CreateBuffer			
@Description			Create High-speed File Buffer.		
@Return value			Handle to an HFB Buffer object, or null if no buffer objects available.
@*/
libFunc HFB_BUFFER_HANDLE HFB_CreateBuffer(
	int sizeOfBuffer,		/* size in bytes of buffer to allocate. */
	int reserved			/* reserved - must bbe zero. */
);



/*@
@Name			HFB_InitBuffer	
@Description		After creating a buffer and opening a file, an application mst initialize the buffer with data.
@Return value		Zero if successful, non-zero if not successful.
@*/
libFunc int HFB_InitBuffer(
	HFB_BUFFER_HANDLE bufferHandle,		/* handle to HFB buffer object. */
	HFB_FILE_HANDLE fileToLoad,		/* handle to HFB file object */
	int startFrame,				/* frame at which to being playback , normally 0. */
	int initialReadSize			/* amount of buffer to preload with data (specified in bytes). -1 means 3/4 buffer. -2 fill entire buffer */
);



/*@
@Name			HFB_FillBuffer	
@Description		read additional data from a file into space invalidated by HFB_ReleaseStreamingData calls 
or any empty buffer space available. For best results call this function once per frame with the elapsedFrames set to DUCK_IGNORE_COUNT. 
The function will use the elapsedFrame parameter in conjunction with internal computed values to determine the amount to read from the file 
in order to avoid waiting for data to become availabble.
@Return value		Number of bytes actually read from the disk file into the buffer or a negative error code.
@*/
libFunc int HFB_FillBuffer(
	HFB_BUFFER_HANDLE bufferHandle,			/* handle to a buffer objects */
	int maxToRead,					/* maximum number of bytes to read during this call */
	int elapsedFrames				/* number of frames elapsed since start of play */
	);


/*@
@Name			HFB_DestroyBuffer
@Description		Free memory used by buffer and release buffer object.
@Return value		none.
@*/
libFunc void HFB_DestroyBuffer(
	HFB_BUFFER_HANDLE bufferHandle			/* handle to an HFB buffer object */
);


/*@
@!Name			HFB_ResetStreams	
@Description		
@Return value
@*/
libFunc void HFB_ResetStreams(
	HFB_BUFFER_HANDLE bufferHandle			/* */
);



/*@
@Name			HFB_SetBufferMode
@Description		Sets the mode for the specified bufffer. Buffer mode defaults to HFBMODE_NORMAL unless this function is called.	
@Return value
@*/
libFunc int HFB_SetBufferMode(
	HFB_BUFFER_HANDLE buffer,			/* handle to HFB buffer object. */
	hfbBufferMode mode				/* mode. */
	);


/*@
@!Name			HFB_GetBufferPerCentFull
@Description		
@Return value
@*/
libFunc int HFB_GetBufferPerCentFull(
	HFB_BUFFER_HANDLE buffer			/* */
);



/*@
@!Name			HFB_GetmovieSize
@Description		
@Return value
@*/
libFunc int HFB_GetmovieSize(
	HFB_BUFFER_HANDLE buffer			/* */
);



/*@
@!Name			HFB_GetBufferSpace
@Description		
@Return value
@*/
libFunc int HFB_GetBufferSpace(
	HFB_BUFFER_HANDLE buffer			/* */
);



/*@
@Name			HFB_GetBufferStatus
@Description		Use this to detemine if a buffer has reached an end of file.
@Return value		0 - buffer OK. 1 - Buffer reached end of file.
@*/
libFunc hfbBufferStatus HFB_GetBufferStatus(
	HFB_BUFFER_HANDLE buffer			/* handle to an HFB buffer object. */
);



/*@
@!Name			HFB_ConditionBuffer
@Description		
@Return value
@*/
libFunc int HFB_ConditionBuffer(
	HFB_BUFFER_HANDLE bufferHandle,			/* */
	int bufferSize,				/* */
	int reserved					/* */
);


#define HFB_ResetBuffer HFB_ConditionBuffer


/*@
@Name			HFB_GetStream
@Description		get a stream reference handle by name, number, or type.	
@Return value		handle to a stream object.
@*/
libFunc HFB_STREAM_HANDLE HFB_GetStream(
	HFB_FILE_HANDLE fileHandle,			/* handle to HFB file object. */
	const char *StreamNameOrNull,			/* C string containing the name of a stream within the specified file. Null to ignore. */
	int streamNumber,				/* an absolute stream number or the nth occuring stream of the specified file. */
	unsigned int streamType			/* the type of stream to be opened. */
);



/*@
@Name			HFB_ReleaseStream
@Description		relinquish reference to stream object so it may return to the pool.	
@Return value		none.
@*/
libFunc void HFB_ReleaseStream(
	HFB_STREAM_HANDLE streamHandle			/* handle to an HFB stream object. */
);



/*@
@Name			HFB_GetStreamInfo
@Description		get a pointer to stream info struct
@Return value		pointer to a struct containing the stream info.
@*/
libFunc HFB_STREAM_INFO* HFB_GetStreamInfo(
	HFB_STREAM_HANDLE streamHandle		/* handle to an HFB stream object */
);




#define HFB_GetStreamLength(strmh) HFB_GetStreamInfo(strmh)->lLength

#define HFB_GetStreamName(strmh) HFB_GetStreamInfo(strmh)->szName


/*@
@Name			HFB_GetStreamingData
@Description		Get pointer to buffered record and length. Normally this will be balanced by a call to HFB_ReleaseStreamingData, unless 
the entire file fits within the HFB buffer. The operation does not move compressed data.	
@Return value		>= 0 handle to compressed block; -1 request out of range ; -2 block exists but is not in the buffer ... 
usually caused by unrleased block of buffer starvation ; -4 block has been release from use.
@*/
libFunc HFB_DATA_HANDLE HFB_GetStreamingData(
	HFB_STREAM_HANDLE streamHandle,			/* handle to an HFB stream object. */
	void **ptrToPtr,				/* pointer to pointer to compressed data. */
	int *ptrToLength,				/* pointer to length of data in bytes. */
	hfbDirection directionToMove,			/* direction in which to read records. */
	int framesToMove				/* the number of reqested records. */
);




/*@
@Name			HFB_ReleaseStreamingData
@Description		release buffer space occupied by record	
@Return value		none.
@*/
libFunc void HFB_ReleaseStreamingData(
	HFB_BUFFER_HANDLE bufferHandle,			/* handle to HFB buffer object. */
	HFB_DATA_HANDLE recordToRelease			/* index of data record to release. */
);



/*@
@Name			HFB_ReadData
@Description		read data directly from a file into a supplied buffer, 
 limit is set by initial value of *ptrToLength 
@Return value		0 on success, or negative error code.
@*/
libFunc int HFB_ReadData(
	HFB_STREAM_HANDLE streamHandle,			/* handle to HFB stream object. */
	void *data,					/* pointer to where data should be copied. Used by duck_read. */
	int *maxLength,				/* pointer to max data size, will be over-written with actual count of bytes read. */
	hfbDirection directionToMove,			/* direction in which the seek should move. */
	int count					/* the number of datarecords to move before reading. Absolute referencse begin at 0. */
);


libFunc int HFB_ReadDataBlocking(
	HFB_STREAM_HANDLE streamHandle,			/* handle to HFB stream object. */
	void *data,					/* pointer to where data should be copied. Used by duck_read. */
	int *maxLength,				/* pointer to max data size, will be over-written with actual count of bytes read. */
	hfbDirection directionToMove,			/* direction in which the seek should move. */
	int count					/* the number of datarecords to move before reading. Absolute referencse begin at 0. */
);



/*@
@!Name			HFB_FramestoNextKeyFrame
@Description		
@Return value
@*/
libFunc int HFB_FramestoNextKeyFrame(
	HFB_STREAM_HANDLE streamHandle,			/* */
	int recordHandle,				/* */
	int *numberOfRecordsSpanned			/* */
	);



/*@
@!Name			HFB_FrameToChunk
@Description		
@Return value
@*/
libFunc int HFB_FrameToChunk(
	HFB_STREAM_HANDLE streamHandle,			/* */
	int frameNumber					/* */
	);


/*@
@Name			HFB_PreviousKeyFrame
@Description		Get the frameNumber of the keyframe at or prior to the specified frameNumber	
@Return value
@*/
libFunc int HFB_PreviousKeyFrame(
	HFB_STREAM_HANDLE streamHandle,			/* */
	int frameNumber					/* */
	);



typedef enum FTYPE {
	HFB_FRAMENUM = 0,
	HFB_INDEXNUM = 1
} hfbFrameNumber;



/*@
@Name			HFB_GetIndexFlags
@Description		get the HFB index flags for the specified record/frame		
@Return value		>= 0 data block flags for data block specifid; -1 frameNum is out of index range ; -2 frameNum is out of frame range.
@*/
libFunc int HFB_GetIndexFlags(
	HFB_STREAM_HANDLE ,			/* handle to HFB stream object. */
	hfbFrameNumber frameNumberType,		/* one of HFB_FRAMENUM, HFB_INDEXNUM */
	int recordHandleOrFrameNumber		/* record handle or frame number. */
);

/*@
@Name			HFB_AddIndexFlags
@Description		add the HFB index flags for the specified record/frame		
@Return value		0 if unsuccessful; 1 if successful;
@*/
libFunc int HFB_AddIndexFlags(
	HFB_STREAM_HANDLE ,			/* handle to HFB stream object. */
	hfbFrameNumber frameNumberType,		/* one of HFB_FRAMENUM, HFB_INDEXNUM */
	int recordHandleOrFrameNumber,		/* record handle or frame number. */
	int flags							/* flags to add */
);

/*@
@Name			HFB_GetDataPosition
@Description		get current data position. video - frameNumber, audio - sampleCount. 
This is useful for resolving differences between streams when starting from a position other than the beginning of the file. 
@Return value		Longword starting position of the data record within the stream, expressed in audio samples for audio streams and frames for video streams.
@*/
libFunc int HFB_GetDataPosition(
	HFB_STREAM_HANDLE streamHandle,		/* handle to HFB stream object. */
	HFB_DATA_HANDLE dataIndex		/* index to a data record within a stream. Use -1 to find position of first available record in the buffered stream */
);



/*@
@Name			HFB_GetAudioInfo
@Description		Get information about audio stream.
@Return value		pointer to a DKWAVEFORM structure describing the audio information contained in the stream.
@*/
libFunc DKWAVEFORM *HFB_GetAudioInfo(
	HFB_STREAM_HANDLE aStream,		/* handle to HFB stream object */
	int *NumChannels,			/* pointer to number of channels in the audio stream. */
	int *SamplesPerSec,			/* pointer to samples per second in the audio stream. */
	int *BytesPerSec,			/* pointer to bytes per second in the audio stream. */
	int *wFormat				/* pointer to the format tag value for the audio stream. */
);



/*@
@!Name			HFB_GetInitialFrames
@Description		/* get the amount of audio skew expressed in records 
@Return value
@*/
libFunc int HFB_GetInitialFrames(
	HFB_STREAM_HANDLE videoStream,		/* handle to video stream */
	HFB_STREAM_HANDLE audioStream		/* handle to audio stream */
);



/*@
@Name			HFB_GetSamplesPerFrame
@Description		get the number of audio frames elapsed during a single video frame
@Return value		The number of audio samples from the audio stream occurring within a single frame interval of video. 
@*/
libFunc int HFB_GetSamplesPerFrame(
	HFB_STREAM_HANDLE videoStream,		/* handle to an HFB video stream */
	HFB_STREAM_HANDLE audioStream		/* handle to an HFB audio stream */
);



/*@
@Name			HFB_GetFrameRates
@Description		get video frame rate and calculated audio skew (in audio samples) 
@Return value
@*/
libFunc void HFB_GetFrameRates(
	HFB_STREAM_HANDLE videoStream,		/* handle to an HFB video stream */
	HFB_STREAM_HANDLE audioStream,		/* handle to an HFB audio stream */
	int *ptrToIntegerFrameRate,		/* pointer to receive frame rate of dominant stream. */
	int *ptrToEstimatedAudioSampleSkew	/* pointer to number of audio samples appearing before the first video frame in file. */
);


/*@
@Name			HFB_GetDRMData
@Description		get the DRMX data chunk
@Return value
@*/
libFunc int HFB_GetDRMData(
	HFB_FILE_HANDLE dckPtr,		/*  */
	unsigned int* pOutEncFourCC,		/*  */
	int* pOutLength,		/*  */
	unsigned char** ppOutData	/*  */
);



/*@
@!Name			HFB_GetStreamParentBuffer
@Description		functions to retrieve parent buffer and file of a given stream
@Return value
@*/
libFunc HFB_BUFFER_HANDLE HFB_GetStreamParentBuffer(
	HFB_STREAM_HANDLE streamHandle		/* */
);


/*@
@!Name			HFB_GetStreamParentFile
@Description		
@Return value
@*/
libFunc HFB_FILE_HANDLE HFB_GetStreamParentFile(
	HFB_STREAM_HANDLE streamHandle		/* */
);



/*@
@!Name			HFB_GetStreamParentFileHandle
@Description		function to retrieve parent file handle of a given stream		
@Return value
@*/
libFunc int HFB_GetStreamParentFileHandle(
	HFB_STREAM_HANDLE streamhandle		/* */
); /* FWG 6-23-99 */




/*@
@Name			HFB_SetMaxFrameSize
@Description		Tell HFB maximum frame size in bytes.
****** DO NOT GIVE WRONG NUMBER****** This might cause 
HFB to crash. Only used in Fly-Index mode. 
@Return Value		T.B.D.
@*/
libFunc int HFB_SetMaxFrameSize(
	HFB_BUFFER_HANDLE Buffer,	/* handle to an HFB Buffer */
	int maxFrameSize		/* maximum frame size */
); /* FWG 7-7-99 */



libFunc hfbBufferMode HFB_GetBufferMode(HFB_BUFFER_HANDLE); /* FWG 7-8-99 */

/* used to precisely calculate rational frame rates
   for a specific stream */
libFunc void HFB_GetStreamRateAndScale(
	HFB_STREAM_HANDLE xStream,		/* stream handle */
	int *rate, 				/* rate - of rate and scale. */
	int *scale				/* scale - of rate and scale. */
);



/*@
@Name			HFB_GetStreamFCCs		
@Description		get stream type and handler fourCC codes,
@Return Value	   	returns type (not handler) 
@*/
libFunc unsigned int HFB_GetStreamFCCs(
	HFB_STREAM_HANDLE xStream,		/* handle to compressed stream */
	unsigned int *type,			/* pointer to stream type */
	unsigned int *handler			/* pointer to fourCC code */
	);



/*@
@Name			HFB_NextKeyFrame
@Description		returns the index of the next keyframe, looking forward from StartChunk. 
byteRate is the projected byterate up to that keyframe.
@Return Value		returns the index of the next keyframe
@*/
int HFB_NextKeyFrame(
	HFB_STREAM_HANDLE sPtr, 		/* stream handle */
	int StartChunk,				/* look forward from here. */
	int *byteRate				/* */
);

/*
@Name			HFB_isVideoKeyFrame
@Description	checks if a video frame is a key frame (in logarithmic time)
@Return Value	returns 1 if key frame, 0 if not, -1 on error
@Note:          only works for the first video stream*/

int HFB_isVideoKeyFrame(
    HFB_BUFFER_HANDLE dckPtr,   /* buffer handle */
    int frameNum                /* video frame to check */
);   

int HFB_ProjectBitRate(HFB_STREAM_HANDLE sPtr, int StartChunk, int *numFrames );
// returns the byterate for the next *numFrames , given a starting chunkIndex
// numFrames is a pointer since the actual amount looked ahead may be adjusted
// as you get to the end of the stream

/* Determine the stream to which we can switch that has the highest precedence       */
/* (defined by order in the array), but is below the ceiling specified by the caller */
/* Note: We can only switch to another stream if its next frame is a keyframe.       */
/* Note: If no streams are below the ceiling, we will attempt to switch to the       */
/*       lowest bitrate stream on a keyframe.                                        */
int HFB_SelectBestFitStream(
	HFB_STREAM_HANDLE* streamArray, // array of streams to consider
	int numberOfStreamsInArray,     // number of streams in the array
	int currentStream,              // array index matching currently used stream
	int desiredProjectionSpan,      // distance in frames ahead to project
	int bitRateCeiling);            // we're looking for a stream under this bitrate


/* get streamSampleSize, <= 0 means variable */
libFunc int HFB_GetStreamSampleSize(HFB_STREAM_HANDLE xStream);


/*@
@!Name			HFB_WhatsAhead
@Description
@Return value
@*/
libFunc int HFB_WhatsAhead(
	HFB_STREAM_HANDLE sPtr, 	/* */
	int StartChunk, 		/* */
	int *horizon			/* */
);



/* windows 95 dll system abstraction functions */

libFunc void HFB_Setmalloc(
	void *(*mallocFuncPtr)(unsigned int size)
	);

libFunc void HFB_Setcalloc(
	void *(*callocFuncPtr)(unsigned int size, unsigned int number)
	);

libFunc void HFB_Setfree(
	void (*freeFuncPtr)(void *)
	);

libFunc void HFB_Setopen(
	int (*openFuncPtr)(const char *, int,...)
	);

libFunc void HFB_Setclose(
	int (*closeFuncPtr)(int)
	);

libFunc void HFB_Setread(
	int (*readFuncPtr)(int,void *, unsigned int)
	);

libFunc void HFB_Setseek(
	int (*seekFuncPtr)(int,int,int)
	);

/*@
@Name			HFB_GetStreamArray
@Description		HFB_GetStreamArray 
will return an array of all streams in a file 
that are of the given type.  If the given type is DUK_UNDEFINED, 
all streams will be included in the array, regardless of type. 
This function will also report back the size of the array (ie: the number 
of matching streams) through the numberOfStreams pointer.
@Return value		The array of stream handles 
@*/
HFB_STREAM_HANDLE* HFB_GetStreamArray(
	HFB_FILE_HANDLE FileHandle,      /* the file we will be looking in for our streams */
	unsigned int StreamType,        /* the type of stream we are looking for */
	unsigned int* numberOfStreams    /* a pointer to the number of matching streams */
);



/*@
@Name			HFB_ReleaseStreamArray
@Description		HFB_ReleaseStreamArray will deallocate an array of streams which
was previously allocated by a call to HFB_GetStreamArray(). 	
@Return Value
@*/
void HFB_ReleaseStreamArray(
	HFB_STREAM_HANDLE* StreamArray,		/* the array of streams we want to release */
	unsigned int numberOfStreams		/* number of streams in the array */
);

/*@
@Name			HFB_GetLastError
@Description	    HFB_GetLastError will return the last error that occured
@Return Value   0 if successful, -1 if unsuccessful
@*/
int HFB_GetLastError(
        HFB_BUFFER_HANDLE bfHnd,        /* pointer to the buffer handle */
        int* lastErrorCode,             /* will return negative value for HFB error, 
                                           positive value for SAL error */
        char errorString[],             /* will return error string */
        size_t maxLen                   /* max size of error string */
);


typedef enum HFB_ERR_t {

    HFB_ERR_MIN = -7,
    HFB_ERROR_LOADING_INDEX = -6,
    HFB_ERROR_PARSING_FILE = -5,
	HFB_PARTIALREAD = -4,
	HFB_ENDOFSTREAM = -3,
	HFB_NOTREADY = -2,
	HFB_ERROR = -1,
	HFB_OK = 0
	
} HFB_ERR;


typedef struct HFB_ERR_DECODE_temp
{
	HFB_ERR      code;
	char*        decode;
	
}  HFB_ERR_DECODE_t;


#if defined(__cplusplus)
}
#endif

#undef libFunc

#endif
