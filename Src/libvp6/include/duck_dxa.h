#ifndef _duck_dxa_h
#define _duck_dxa_h


#include "duck_wav.h"


#if defined(__cplusplus)
extern "C" {
#endif


typedef struct tXAudioSource*	DXL_XAUDIOSRC_HANDLE;  /* forward looking ! */


#define MAX_AUDIO_REGISTRATIONS 20

/* Structure used to register DXA plugins with dxa by formatTag.           */
/*-------------------------------------------------------------------------*/
typedef struct tXAudioRegistration {
	unsigned short formatTag;
	int 	(*audio_dxer_init)(DXL_XAUDIOSRC_HANDLE src);
	int 	(*audio_dxer_dx)(DXL_XAUDIOSRC_HANDLE src, void *left, void *right, int nSamples);
	int 	(*audio_dxer_exit)(DXL_XAUDIOSRC_HANDLE src);	
	int 	(*audio_dxer_clear)(DXL_XAUDIOSRC_HANDLE src);	     
} DXL_AUDIO_REGISTRATION;



/* If it's not documented here, if it's only defined here ... then it's probably not needed by most codec plugins */
/* It may only be used internally to dxa .                                                                        */
/*----------------------------------------------------------------------------------------------------------------*/
typedef struct tXAudioSource
{ 	
	unsigned char 		*addr;				/* address from which to read compressed audio bytes 	*/
	int 			totalPos;
	int 			length;				/* length of compressed audio bytes to read from buffer	*/	
	int 			interleave;
	short 			aiSamp[2],aiStepIndex[2];
	int 			blockFinished;			/* flags audio system that new data in buffer */
	int			samplesRead;			
	UINT64 			profileStartTime;
	UINT64 			profileEndTime;
	UINT64 			dxClocks;
        UINT64  	  	samplesDXed;
	short 			iFirstNibble;
	short 			iNextInput;
	short 			sum,diff,nudiff;
	DKWAVEFORM 		wv; 				/* details of the compressed audio data */
	DXL_AUDIO_REGISTRATION  registration;
	void* 			more;     			/* user data  ... plugin data  */

} DXL_XAUDIOSRC;



typedef struct tAudioBuff *DXL_AUDIODST_HANDLE;




/* audio function prototypes */

/*@
@Name  			DXL_InitAudio
@Description  		Initialize audio decompression services. This function allocates memory for requested object pools.
@Return value 		DXL_OK on success, or negative error code.
@*/
int DXL_InitAudio(
	int srcs,	/* max number of audio sources to be created. */
	int dsts	/* max number of audio destinations to be created. */
);



/*@
@Name DXL_ExitAudio
@Description		Shutdown audio decompression services, freeing allocated objects.
@Return value		none.
@*/
void DXL_ExitAudio(void);


typedef struct tDKWAVEFORM *DKWAVEFORMPTR; /* place holder pointer */



/*@
@Name 		DXL_CreateXAudioSrc
@Description 	Create a compressed audio source (decompressor) 			
@Return value  	returns an DXL_XAUDIOSRC_HANDLE or null unable to create audio source object.
@*/
DXL_XAUDIOSRC_HANDLE DXL_CreateXAudioSrc(
    
	DKWAVEFORMPTR wv, 			/* pointer to compressed waveform struct describing the audio input. */	
	unsigned char *addr,			/* address of compressed data */
	int length				/* length of compressed data in bytes. */	
);



/*@
@Name  		DXL_AlterXAudioData
@Description 	Link an audio decompressor to the next unit of compressed data. 
This function cannot change the type of xSource on the fly. That must remain consistent. 
Setting the address of the audio data to null causes the xSource to generate an infinate number of "zero" value samples.					
@Return value 	void
@*/
void DXL_AlterXAudioData(
	DXL_XAUDIOSRC_HANDLE xSource,		/* handle to compressed audio source */
	unsigned char *addr,			/* pointer to new compressed audio data */
	int length				/* length of compressed data in bytes. */
);




/*@
@Name 		DXL_DestroyXAudioSrc
@Description 	clears an audio decompressor and returns it to the pool.
@Return value 	void
@*/
void DXL_DestroyXAudioSrc(
	DXL_XAUDIOSRC_HANDLE xSource	/* compressed audio source */
);
        


/*@
@Name 			DXL_CreateAudioDst
@Description 		Create a audio destination description. When numChannel equals 2 but addrR is null, it 
is assumed that multi-channel samples should be interleaved within the dest buffer pointed to by addrL.
@Return value		returns an object of type DXL_AUDIODST_HANDLE, and audio destination.
@*/
DXL_AUDIODST_HANDLE DXL_CreateAudioDst(
    void *addrL,		/* pointer to left audio destination channel */
    void *addrR,		/* pointer to right audio destination channel */
    int length,			/* audio buffer size in bytes. */
    int bitDepth,		/* bits per sample */
    int numChannels,		/* number of audio channels */
    int sampleRate		/* samples per second */
);



/*@
@Name 			DXL_AlterAudioDst
@Description 		Change characteristics of audio destination. 
Specify 0 or null values for no change.
@Return value 		void
@*/
void DXL_AlterAudioDst(
    DXL_AUDIODST_HANDLE dst,	/* handle to audio destionation */
    void *addrL,		/* pointer to left audio destination channel */
    void *addrR,		/* pointer to right audio destination channel */
    int length,			/* audio buffer size in bytes. */
    int bitDepth,		/* bits per sample (8 or 16) */
    int numChannels,		/* number of audio channels (1 or 2) */
    int sampleRate		/* samples per second */
);



/*@
@Name 			DXL_DestroyAudioDst
@Description 		clears and audio destination object and returns it to the pool.	
@Return value 		none.
@*/
void DXL_DestroyAudioDst(
	DXL_AUDIODST_HANDLE dst		/* handle to audio destination */
);



/*@
@Name		DXL_dxAudio
@Description 	decompress up to maxSamplesToDecompress. The number of samples transferred is controlled by two factors. 
One factor is the limit parameter. The other factor is the  number of remaining samples in the src (internal buffer).
If the function returns less that the desired number of samples, get another audio record (via HFB_GetStreamingData()) for source data and try again.
@Return value	returns the actual number of samples decompressed 
@*/
int DXL_dxAudio(
	DXL_XAUDIOSRC_HANDLE src,	/* handle to compressed audio source.  */
	DXL_AUDIODST_HANDLE dst,	/* handle to uncompressed audio destination */
	int maxSamplesToDecompress	/* Try to decompress up to this many samples to the destination */
);


/*@
@Name		DXL_ClearAudio
@Description	Clears any internal audio buffers compressed and/or decompressed data so that playback may start from a new point. 
@Return value
@*/
int DXL_ClearAudio(DXL_XAUDIOSRC_HANDLE xSourcePublic);



int DXL_RegisteredAudioDXerGet(DXL_AUDIO_REGISTRATION *oneRegistration);

int DXL_RegisteredAudioDXerSet(
	unsigned short formatTag,
	int (*audio_dxer_init)		(DXL_XAUDIOSRC_HANDLE src),
	int (*audio_dxer_dx)		(DXL_XAUDIOSRC_HANDLE src, void *left, void *right, int nSamples),
	int (*audio_dxer_exit)		(DXL_XAUDIOSRC_HANDLE src),
	int (*audio_dxer_clear)		(DXL_XAUDIOSRC_HANDLE src)
	);
	
	
	
void DXL_AudioAccurateTime(UINT64* temp);


/* Register one of the On2 dxa plugins */
/*-------------------------------------*/
int DXL_RegisterAVC(void);
int DXL_RegisterAC3(void);
int DXL_RegisterQDesign(void);
int DXL_RegisterACM(unsigned short formatTag);
int DXL_RegisterDK4(void);
int DXL_RegisterMP3(void);





#if defined(__cplusplus)
}
#endif

#endif /* include guards */
