#ifndef _duck_wav_h
#define _duck_wav_h

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct tDKWAVEFORM
{
	unsigned short	wFormatTag;        /* format type */
	unsigned short	nChannels;         /* number of channels (i.e. mono, stereo...) */
	unsigned int   nSamplesPerSec;    /* sample rate */
	unsigned int	nAvgBytesPerSec;   /* for buffer estimation */
	unsigned short	nBlockAlign;       /* block size of data */
	unsigned short	wBitsPerSample;    /* Number of bits per sample of mono data */
	unsigned short	cbSize;            /* The count in bytes of the size of
									extra information (after cbSize) */
	unsigned short	wSamplesPerBlock;

	unsigned int   userData[16];
} DKWAVEFORM;

/* don't try to endian fix the userData ! */
static int DKWAVEFORM_REFLECT[ ] = { 2,2, 4,4, 2,2,2,2 };

#if defined(__cplusplus)
}
#endif
#endif
