/*****************************************************************************

    Module      ToolVox.H
                The defines and prototypes for the Voxware voice compression
                system.


        Version 1.1.8.192
        For MacOS, Windows 3.1, Windows95, and Solaris


		Voxware Inc.
		305 College Road East
		Princeton, New Jersey  08540
		http://www.voxware.com


		Voxware Proprietary Material
		Copyright 1996, Voxware, Inc.
		All Rights Resrved

		DISTRIBUTION PROHIBITED without written authorization from Voxware

*****************************************************************************/



/*
    !!! All reserved, padding, and optional fields should be set to zero if
    !!! you are not using them.  Please memset() all structures before using
    !!! them.
*/



/*
    You must "#define VOXWARE_xxx" for the environment you
    are generating code for.

    Valid types are:
        VOXWARE_WIN16       <-- Windows 3.x
        VOXWARE_WIN32       <-- Windows 95 or Windows NT
        VOXWARE_DOS         <-- MS-DOS
        VOXWARE_MAC         <-- MacOS
        VOXWARE_SUN         <-- SunOS/Solaris
		VOXWARE_HP          <-- HPUX (Version 9.xx)
		VOXWARE_AIX			<-- IBM's AIX
*/




#ifndef __TOOLVOX_H_
#define __TOOLVOX_H_

#ifdef __cplusplus
extern "C" {
#endif



#ifndef VOXWARE_SUN
    #ifdef VOXWARE_MAC
        #include <ConditionalMacros.h>
    #endif
    #if PRAGMA_ALIGN_SUPPORTED
        #pragma options align=mac68k
    #elif VOXWARE_HP
        #pragma HP_ALIGN HPUX_WORD
    #else
        #pragma pack(2)
    #endif
#endif



// DAVE:  THIS SHOULD PROBABLY BE IN THIS FILE INSTEAD OF ITS OWN HDR. -GEO
#include "voxchunk.h"

/* Value returned from a ToolVox function call.  See ToolVox manual for a   */
/* list of specific return codes for each function.                         */
typedef unsigned short    VOXWARE_RETCODE;



/* We support VOXWARE_WIN for backwards compatibility.  Use VOXWARE_WIN16.  */
#if defined (VOXWARE_WIN)
    #define VOXWARE_WIN16
#endif

#if defined (VOXWARE_WIN16)
    #define VOX_FAR             __far
    #define VOX_EXPORT          __export
    #define VOX_PASCAL          __pascal
    #define VOXAPI              VOX_FAR VOX_PASCAL

    #ifdef __cplusplus
        #define VOXAPI_CALLBACK VOX_FAR __pascal
    #else
        #define VOXAPI_CALLBACK VOX_FAR __pascal __loadds
    #endif


#elif defined(VOXWARE_WIN32)
    #define VOX_FAR
    #define VOX_EXPORT
    #define VOX_PASCAL      __stdcall
    #define VOXAPI          VOX_PASCAL
    #define VOXAPI_CALLBACK  __stdcall


#elif defined(VOXWARE_MAC)
    #define VOX_FAR
    #define VOX_EXPORT
    #define VOX_PASCAL          pascal
    #define VOXAPI              VOX_PASCAL
    #define VOXAPI_CALLBACK     VOX_PASCAL


#elif defined(VOXWARE_SUN)
#if defined(VOXWARE_SGI)  || defined(VOXWARE_DEC)
    #define VOX_FAR
    #define VOX_EXPORT
    #define VOX_PASCAL
    #define VOXAPI      
    #define VOXAPI_CALLBACK        short
#else
    #define VOX_FAR
    #define VOX_EXPORT
    #define VOX_PASCAL
    #define VOXAPI      
    #define VOXAPI_CALLBACK
#endif

#elif defined(VOXWARE_DOS)
    #define VOX_FAR             __far
    #define VOX_EXPORT
    #define VOX_PASCAL          pascal
    #define VOXAPI              VOX_PASCAL
    #define VOXAPI_CALLBACK     VOX_PASCAL

#else
    #pragma message ("TOOLVOX.H: Platform indicator #define not setup.")
    #pragma message ("TOOLVOX.H: One of the following must be initialized:")
    #pragma message ("TOOLVOX.H:      #define VOXWARE_WIN16")
    #pragma message ("TOOLVOX.H:      #define VOXWARE_WIN32")
    #pragma message ("TOOLVOX.H:      #define VOXWARE_MAC")
    #pragma message ("TOOLVOX.H:      #define VOXWARE_SUN")
    #pragma message ("TOOLVOX.H:      #define VOXWARE_DOS")
    #pragma message ("TOOLVOX.H: Check the Voxware manual for more information.")

#endif



/*****************************************************************************
**                                                                          **
**  The data structures that will be passed to the ToolVox functions.       **
**                                                                          **
*****************************************************************************/



typedef struct tagVOX_ENVIRONMENT
{
    unsigned short  wSizeofVoxEnvironment;  /* --> Fill this in for us      */
    unsigned short  wCompatibility;         /* <-- versioning               */
    unsigned long   dwAPIVersionNumber;     /* <-- Version of API           */
    unsigned long   dwAPIMaintanceNumber;   /* <-- Maint Rel of API         */
    unsigned short  wNumCodecs;             /* <-- Number of CODECS avail.  */
    unsigned short  wNumVoiceFonts;         /* <-- Number of VoiceFonts     */
    char            szAPIName[32];          /* <-- ToolVox For MacOS 1.1.5  */
} VOX_ENVIRONMENT;

typedef VOX_ENVIRONMENT  VOX_FAR * LPVOX_ENVIRONMENT;





typedef struct tagVOXWARE_DATA
{
    unsigned short    wSizeOfVoxwareData;   /* The size of this structure   */

    unsigned short    wUserData;            /* Application dependent data   */
                                            /* ...(16 bit).                 */
    unsigned long     dwUserData;           /* Application dependent data   */
                                            /* (32 bit).                    */

    /* Buffer information required for (de)compressing between buffers <->  */
    /* disk.                                                                */
    unsigned long     dwInputType;          /* What is stored in the        */
                                            /* ...'lpInputLocation' field.  */
    void VOX_FAR      *lpInputLocation;     /* Pointer to a filename or     */
                                            /* ...buffer.                   */

    unsigned long     dwInputSize;          /* Inform Voxware about the     */
                                            /* ...buffer size.              */
    unsigned long     dwInputPosition;      /* Input data offset info.      */

    unsigned long     dwInputSamplingRate;  /* Number of samples per second.*/
    unsigned short    wInputBytesPerSample; /* Number of bytes per sample.  */


    unsigned long     dwOutputType;         /* What is stored in the        */
                                            /* ...'lpOutputLocation' field. */
    void VOX_FAR      *lpOutputLocation;    /* Pointer to filename, buffer, */
                                            /* ...or empty for playback.    */
    
    unsigned long     dwOutputSize;         /* Inform Voxware about the     */
                                            /* ...buffer size.              */
    unsigned long     dwOutputPosition;     /* Output data offset info.     */

    unsigned long     dwOutputSamplingRate; /* Number of samples per second.*/
    unsigned short    wOutputBytesPerSample;/* Number of bytes per sample.  */


    /* The decompression routines will use the following for voice effects: */
    unsigned short    bUseAbsolutePitchFlag;/* TRUE if fPitchChange is to   */
                                            /* ...be used; FALSE is for     */
                                            /* ...fPitchShift usage.        */
    float             fRelativePitch;       /* Relative PitchShift factor.  */
    float             fAbsolutePitch;       /* Absolute PitchShift value.   */
    float             fWarpedLengthFactor;  /* Warping factor               */
                                            /* ...0.2<factor<5.0 (a value   */
                                            /* ...of 1.0 = normal time)     */
    void VOX_FAR      *lpVoiceFont;         /* VoiceFont used for playback. */

    unsigned long     dwVoiceFontEnum;      /* Data used for enumeration of */
                                            /* ...VoiceFonts.               */

    /* This is a more detailed value that can help track down errors.       */
    signed long       dwSecondaryReturnCode;

    unsigned long     dwDataLeft;           /* Amount of data to process.   */

    /* A callback function that the effects routines will call.             */
    void VOX_FAR      *lpCallbackFunc;

    void VOX_FAR      *lpvControlBlock;     /* Private control block used   */
                                            /*  ...by Voxware.              */

    /* New fields for ToolVox version 1.1.0.                                */ 

    unsigned long     dwCompressionSettings;/* Choose the compression codec.*/ 

    unsigned long     dwStatus;             /*  Status of ToolVox engine.   */

    unsigned short    wVoiceFontType;       /* Unused.  Set to zero.        */

    unsigned short    bUseAbsoluteGainFlag; /* true if fGainControl is used */
    float             fRelativeGain;        /* decompression only           */
                                            /* Multimedia Codecs Only       */
                                            /* 1.0 is normal                */
                                            /* greater than 1 is louder     */
                                            /* less than 1 is softer        */
    float             fAbsoluteGain;        /* decompression only           */
    signed short      wCurrentEnergyLevel;  /* 0..255; > 200 = clipping     */

    unsigned short    padding3;             /* Unused.  Set to zero.        */

    void VOX_FAR      *lpReservedFunction;  /* reserved 1                   */
    void VOX_FAR      *lpReservedVars;      /* reserved 2                   */

    void VOX_FAR      *lpMarkerRecord;      /* reserved                     */

//###GTT - Added support for communication of info chunks - 5/30/96
    VOX_CHUNK_INFO VOX_FAR  *lpInfoChunks;
    short               wNumInfoChunks;
    short               bV1Compatible;
    
    unsigned short      wVoiceFontID;       /* Used to activate a VoiceFont */

	void VOX_FAR *		lpPlaybackWindow;

    unsigned short      wNumBitsPerFrame;

}   VOXWARE_DATA;

typedef VOXWARE_DATA  VOX_FAR * LPVOXWARE_DATA;





typedef struct tagVOX_VERSION
{
    /* The version and internal name for the ToolVox Compression DLL.       */
    unsigned char   szCompressVersion[64];
    unsigned char   szCompressInternal[32];

    /* The version and maintenance for the ToolVox Compression DLL.         */
    /* For example: 1.30.46.01                                              */
    unsigned long   dwCompressVersion;       /* e.g. 0x00010030 = "01.30"   */
    unsigned long   dwCompressMaint;         /* e.g. 0x00460001 = "46.01"   */


    /* The version and internal name for the ToolVox Decompression DLL.     */
    unsigned char   szDecompressVersion[64];
    unsigned char   szDecompressInternal[32];

    /* The version and maintenance for the ToolVox Decompression DLL.       */
    /* For example: 1.30.46.01                                              */
    unsigned long   dwDecompressVersion;     /* e.g. 0x00010030 = "01.30"   */
    unsigned long   dwDecompressMaint;       /* e.g. 0x00460001 = "46.01"   */


    /* The version and internal name for the ToolVox VoiceFont DLL.         */
    unsigned char   szVoiceFontVersion[64];
    unsigned char   szVoiceFontInternal[32];

    /* The version and maintenance for the ToolVox VoiceFont DLL.           */
    /* For example: 1.30.46.01                                              */
    unsigned long   dwVoiceFontVersion;      /* e.g. 0x00010030 = "01.30"   */
    unsigned long   dwVoiceFontMaint;        /* e.g. 0x00460001 = "46.01"   */


    /* The version and internal name for the ToolVox Utility DLL.           */
    unsigned char   szUtilityVersion[64];
    unsigned char   szUtilityInternal[32];

    /* The version and maintenance for the ToolVox Utility DLL.             */
    /* For example: 1.30.46.01                                              */
    unsigned long   dwUtilityVersion;        /* e.g. 0x00010030 = "01.30"   */
    unsigned long   dwUtilityMaint;          /* e.g. 0x00460001 = "46.01"   */


    /* The version and internal name for the ToolVox RT24 Compression DLL.  */
    unsigned char   szRT24CompressVersion[64];
    unsigned char   szRT24CompressInternal[32];

    /* The version and maintenance for the ToolVox RT24 Compression DLL.    */
    /* For example: 1.30.46.01                                              */
    unsigned long   dwRT24CompressVersion;   /* e.g. 0x00010030 = "01.30"   */
    unsigned long   dwRT24CompressMaint;     /* e.g. 0x00460001 = "46.01"   */


    /* The version and internal name for the ToolVox RT24 Decompression DLL.*/
    unsigned char   szRT24DecompressVersion[64];
    unsigned char   szRT24DecompressInternal[32];

    /* The version and maintenance for the ToolVox RT24 Decompression DLL.  */
    /* For example: 1.30.46.01                                              */
    unsigned long   dwRT24DecompressVersion; /* e.g. 0x00010030 = "01.30"   */
    unsigned long   dwRT24DecompressMaint;   /* e.g. 0x00460001 = "46.01"   */

}   VOX_VERSION;

typedef VOX_VERSION VOX_FAR * LPVOX_VERSION;



typedef struct tagVOX_FILE_INFO
{
    /* Some standard WAV, AIFF, and VOX file data.                          */
    unsigned short  wFormatTag;         /* WAVE_FORMAT_VOXWARE for com-     */
                                        /* ...pressed or WAVE_FORMAT_PCM    */
                                        /* ...for standard files.           */
    unsigned short  nChannels;          /* Set to the value                 */
                                        /* ...VOXWARE_WAVE_FILE_MONO or     */
                                        /* ...VOXWARE_WAVE_FILE_STEREO.     */
    unsigned long   nAvgBitsPerSec;     /* The bit rate of this file.       */
    unsigned long   dwSamplingRate;
    unsigned short  wBytesPerSample;
    unsigned long   dwNumSamplesInFile; /* The total number of samples in   */
                                        /* ...the file.                     */

    float           fMeanPitch;         /* Weighted average pitch of the    */
                                        /* ...Vox file.                     */
    float           fMinPitch;          /* Lowest pitch value of the file.  */
    float           fMaxPitch;          /* Highest pitch value of the file. */

//###lee - split this out into a new structure:
    unsigned long   dwCompressedSize;   /* One is estimated, and the other  */
    unsigned long   dwDecompressedSize; /* ...is the current size.          */

    unsigned short  bVariableRate;      /* TRUE if codec is variable rate.  */

}   VOX_FILE_INFO;

typedef VOX_FILE_INFO  VOX_FAR * LPVOX_FILE_INFO;



typedef struct tagVFONT_IDENTIFIER
{
    /* Used for vfontEnumerate calls. */
    char szFontName[32];
    char szFontFile[128];

}   VFONT_IDENTIFIER;

typedef VFONT_IDENTIFIER  VOX_FAR * LPVFONT_IDENTIFIER;



typedef struct tagCODEC_DESCRIPTION
{
    unsigned long   dwCodecID;
    char            szCodecInternalName[44];
    char            szCodecListBoxName[44];
    char            szCodecDescription[256];
}   CODEC_DESCRIPTION;

typedef CODEC_DESCRIPTION  VOX_FAR * LPCODEC_DESCRIPTION;



typedef struct tagVOICE_FONT_DESCRIPTION
{
    unsigned short  wFontID;
    char            szFontFamily[44];
    char            szFontName[44];
    char            szFontDescription[256];
    unsigned short  bHasDecodeEffect;
    unsigned short  wSizeofDecodeChunk;
}   VOICE_FONT_DESCRIPTION;

typedef VOICE_FONT_DESCRIPTION  VOX_FAR * LPVOICE_FONT_DESCRIPTION;



typedef struct tagVOX_FUNCTION_VARS
{
    unsigned short    wSizeOfFunctionVars;  /* The size of this structure   */

    void VOX_FAR      *lpBuffer;            /* Pointer to a the data buffer.*/
    unsigned long     dwBufferSize;         /* The size of the buffer.      */

    unsigned long     dwUserData;           /* Application dependent data   */
                                            /* (32 bit).                    */

    LPVOXWARE_DATA    lpVoxwareData;        /* The original VoxawareData    */
                                            /* used to start processing.    */

}   VOX_FUNCTION_VARS;


typedef VOX_FUNCTION_VARS VOX_FAR * LPVOX_FUNCTION_VARS;


/*
    For use with BitStreams
*/

typedef struct tagVOX_STREAM_HEADER
{
    unsigned long       dwHeaderID;
    unsigned short      wSizeOfVoxStreamHeader;
    unsigned char       voxStreamHeaderData[1]; // variable length
} VOX_STREAM_HEADER;

typedef VOX_STREAM_HEADER VOX_FAR * LPVOX_STREAM_HEADER;



typedef struct tagFIRST_BITSTREAM_BUFFER
{
    unsigned short      wSizeOfFirstBitStreamStruct;
    unsigned short      wNumBitsPerFrame;
    unsigned short      wNumSamplesPerFrame;
    unsigned short      wPadding;
    VOX_STREAM_HEADER   voxStreamHeader;        /* Send this to the decoder */
} FIRST_BITSTREAM_BUFFER;


typedef FIRST_BITSTREAM_BUFFER VOX_FAR * LPFIRST_BITSTREAM_BUFFER;



/*
    VOX_CALLBACK is a typedef for the user function that is called by the
    ToolVox compression and decompression routines.

    Special Note for Macintosh Developers:
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Do >> NOT << give us a Universal Procedure Pointer.
        We make our own so you don't have to... (your welcome)
        Sooo just give us the functions name:
            yourVoxData->lpCallbackFunc = myCallbackProc;
*/
#ifdef VOXWARE_MAC
    typedef VOXAPI_CALLBACK VOXWARE_RETCODE (* VOX_CALLBACK)
                            (unsigned short wVox, unsigned short wMessage,
                            LPVOXWARE_DATA lpVoxData);
#else
#if defined(VOXWARE_SGI) || defined(VOXWARE_DEC)
    typedef VOXWARE_RETCODE ( * VOX_CALLBACK) (unsigned short , unsigned short , LPVOXWARE_DATA );

#else
    typedef VOXWARE_RETCODE (VOXAPI_CALLBACK * VOX_CALLBACK) 
                            (unsigned short wVox, unsigned short wMessage, 
                            LPVOXWARE_DATA lpVoxData);
#endif
#endif


typedef struct tagVOX_COMMAND
{
    unsigned long   dwAttribute;        /* Attribute to act on?             */
    float           fFloatParam;        /* Use this for float attributes.   */
    unsigned long   dwLongParam;        /* Use this for long attributes.    */

} VOX_COMMAND;

typedef  VOX_COMMAND VOX_FAR * LPVOX_COMMAND;


#define ATTRIBUTE_WARPED_LENGTH_FACTOR      1L          // get and set
#define ATTRIBUTE_RELATIVE_PITCH            2L          // get and set
#define ATTRIBUTE_ABSOLUTE_PITCH            3L          // get and set
#define ATTRIBUTE_PLAY_TIME                 5L          // not yet supported
#define ATTRIBUTE_VOICE_FONT                6L          // set only
#define ATTRIBUTE_GAIN_CONTROL              7L          // get and set
#define ATTRIBUTE_BITS_PER_SECOND           8L          // not yet supported
#define ATTRIBUTE_FIXED_RATE_CODEC          9L          // not yet supported



#define ATTRIBUTE_DUMP_BUFFERS          0x80000 + 1L    // RESERVED: bit bit offset is param
#define ATTRIBUTE_SILENT_WINDOW         0x80000 + 2L    // RESERVED: get set            dwLongParam = WORD ms
#define ATTRIBUTE_INDICATE_LOST_FRAME   0x80000 + 3L    // RESERVED:     set            dwLongParam = BOOL
#define ATTRIBUTE_VOICE_LEVEL           0x80000 + 4L    // RESERVED: get set            dwLongParam = WORD Value
#define ATTRIBUTE_VOICE_STATE           0x80000 + 5L    // RESERVED: get                dwLongParam = int Value
//#define ATTRIBUTE_VOICE_FONT_II       0x80000 + 6L    // RESERVED:     set            dwLongParam = short EffectSelection - fFloatParam = short SliderValue
//#define ATTRIBUTE_VOICE_EFFECT        0x80000 + 7L    // RESERVED:     set            dwLongParam = short EffectSelection - fFloatParam = short SliderValue
#define ATTRIBUTE_BIT_OFFSET          	0x80000 + 8L   	// RESERVED:	 set
#define ATTRIBUTE_GAIN_CONTROL_ADJUST   0x80000 + 6L    // RESERVED: get                dwLongParam = signed short Value
#define ATTRIBUTE_AGC_REFERENCE_LEVEL   0x80000 + 7L    // RESERVED: set                dwLongParam = unsigned short Value






/***************************************************************************** 
**
**  lpVoxwareData->dwCompressionSettings = VOXWARE_CODEC_xxx
**
**  Not filling in the dwCompressionSettings field, or setting it to default
**  tells ToolVox to compress what you gave it using the best codec for the
**  job.  If you give us a 11k-16k file, we will chose the MM_11k codec, if
**  you give us a 22k or better file, we will choose the MM_22k codec, etc...
**
**  If you assign one of the values below to the dwCompressionSettings field,
**  then we will send your file through the codec you asked us to.
**
**  Refer to page ###Maija: of the manual for more.
**
*****************************************************************************/ 
 
#define VOXWARE_CODEC_RT_8K                 0x01000001
#define VOXWARE_CODEC_RT_8K_HQ29            0x01000010

#define VOXWARE_CODEC_RT_8K_HQ24            0x01000004  // Do not use - unavailable for this release.
#define VOXWARE_CODEC_RT_8K_VR12            0x01000008  // Do not use - unavailable for this release.
#define VOXWARE_CODEC_RT_8K_VR15            0x01000009  // Do not use - unavailable for this release.
#define VOXWARE_CODEC_RT_8K_UQ              0x0100000C  // Do not use - unavailable for this release.
#define VOXWARE_CODEC_MM_11K                0x02000002  // Do not use - unavailable for this release.
#define VOXWARE_CODEC_MM_22K                0x04000002  // Do not use - unavailable for this release.

#define VOXWARE_CODEC_DEFAULT               0x00000000
#define VOXWARE_CODEC_RT24                  VOXWARE_CODEC_RT_8K     // obs spelling

/*Add for AUDIO codec*/
#define VOXWARE_CODEC_AC_8K                 0X08000001
#define VOXWARE_CODEC_AC_11K                0X08000002
#define VOXWARE_CODEC_AC_16K                0X08000003
#define VOXWARE_CODEC_AC_22K                0X08000004

/* These are provided for backwards compatibility The #define's above should be used. */
#define VOXWARE_AUDIO_CODEC_08              VOXWARE_CODEC_AC_8K
#define VOXWARE_AUDIO_CODEC_11              VOXWARE_CODEC_AC_11K
#define VOXWARE_AUDIO_CODEC_16              VOXWARE_CODEC_AC_16K
#define VOXWARE_AUDIO_CODEC_22              VOXWARE_CODEC_AC_22K

/*****************************************************************************
**
**  These are used to define the input and output types.  They should be ORed
**  together to create the type that is needed.  For example compressing
**  a Windows Wave file would use VOXTYPE_FORMAT_WAVE|VOXTYPE_LOCATION_FILE.
**  To specify the compressed output Wave file, use: VOXTYPE_FORMAT_WAVE|
**  VOXTYPE_LOCATION_FILE|VOXTYPE_QUALIFIER_VOX.
**
*****************************************************************************/

//#define VOXTYPE_FORMAT_IRRELEVENT     (unsigned long)(0x00000000L)
#define VOXTYPE_FORMAT_WAVE             (unsigned long)(0x00010000L)
#define VOXTYPE_FORMAT_RAW              (unsigned long)(0x00020000L)
#define VOXTYPE_FORMAT_AIFF             (unsigned long)(0x00030000L)
#define VOXTYPE_FORMAT_VFONT            (unsigned long)(0x00040000L)
#define VOXTYPE_FORMAT_PITCH            (unsigned long)(0x00050000L)

#define VOXTYPE_LOCATION_FILE           (unsigned long)(0x00000100L)
#define VOXTYPE_LOCATION_BUFFER         (unsigned long)(0x00000200L)
#define VOXTYPE_LOCATION_FSSPEC         (unsigned long)(0x00000300L)
#define VOXTYPE_LOCATION_SOUND          (unsigned long)(0x00000400L)
#define VOXTYPE_LOCATION_FUNCTION       (unsigned long)(0x00000800L)
#define VOXTYPE_LOCATION_STRING         (unsigned long)(0x00001000L)
#define VOXTYPE_LOCATION_BITSTREAM      (unsigned long)(0x00001100L)
#define VOXTYPE_LOCATION_VOICE_FONT_ID  (unsigned long)(0x00001200L)

#define VOXTYPE_QUALIFIER_PCM           (unsigned long)(0x00000000L)
#define VOXTYPE_QUALIFIER_VOX           (unsigned long)(0x00000001L)
#define VOXTYPE_QUALIFIER_PITCH         (unsigned long)(0x00000002L)
//#define VOXTYPE_QUALIFIER_ASYNC       (unsigned long)(0x00000004L)

#define VOXTYPE_CONST_FILE_INFO         (unsigned long)(0x10000000L)
#define VOXTYPE_CONST_VERSION           (unsigned long)(0x20000000L)
#define VOXTYPE_CONST_BENCHMARK         (unsigned long)(0x40000000L)

#define VOXTYPE_FORMAT_MASK             (unsigned long)(0x000F0000L)
#define VOXTYPE_LOCATION_MASK           (unsigned long)(0x0000FF00L)

#define VOXTYPE_VFONT_FILENAME          (unsigned long)(VOXTYPE_FORMAT_VFONT|VOXTYPE_LOCATION_FILE)
#define VOXTYPE_VFONT_DESCRIPTION       (unsigned long)(VOXTYPE_FORMAT_VFONT|VOXTYPE_LOCATION_STRING)
#define VOXTYPE_VFONT_IDENTIFIER        (unsigned long)(VOXTYPE_FORMAT_VFONT|VOXTYPE_LOCATION_BUFFER)



/*
    The following constants can be used in the dwOutputType and dwInputType
    fields of the VoxwareData structure instead of ORing the flags above...
*/
#define TYPE_WAVE_PCM_FILE              (unsigned long)(VOXTYPE_FORMAT_WAVE|VOXTYPE_LOCATION_FILE|VOXTYPE_QUALIFIER_PCM)
#define TYPE_AIFF_PCM_FILE              (unsigned long)(VOXTYPE_FORMAT_AIFF|VOXTYPE_LOCATION_FILE|VOXTYPE_QUALIFIER_PCM)
#define TYPE_RAW_PCM_FILE               (unsigned long)(VOXTYPE_FORMAT_RAW|VOXTYPE_LOCATION_FILE|VOXTYPE_QUALIFIER_PCM)

#define TYPE_WAVE_PCM_FSSPEC            (unsigned long)(VOXTYPE_FORMAT_WAVE|VOXTYPE_LOCATION_FSSPEC|VOXTYPE_QUALIFIER_PCM)
#define TYPE_AIFF_PCM_FSSPEC            (unsigned long)(VOXTYPE_FORMAT_AIFF|VOXTYPE_LOCATION_FSSPEC|VOXTYPE_QUALIFIER_PCM)
#define TYPE_RAW_PCM_FSSPEC             (unsigned long)(VOXTYPE_FORMAT_RAW|VOXTYPE_LOCATION_FSSPEC|VOXTYPE_QUALIFIER_PCM)

#define TYPE_WAVE_PCM_BUFFER            (unsigned long)(VOXTYPE_FORMAT_WAVE|VOXTYPE_LOCATION_BUFFER|VOXTYPE_QUALIFIER_PCM)
#define TYPE_AIFF_PCM_BUFFER            (unsigned long)(VOXTYPE_FORMAT_AIFF|VOXTYPE_LOCATION_BUFFER|VOXTYPE_QUALIFIER_PCM)
#define TYPE_RAW_PCM_BUFFER             (unsigned long)(VOXTYPE_FORMAT_RAW|VOXTYPE_LOCATION_BUFFER|VOXTYPE_QUALIFIER_PCM)

#define TYPE_WAVE_PCM_FUNCTION          (unsigned long)(VOXTYPE_FORMAT_WAVE|VOXTYPE_LOCATION_FUNCTION|VOXTYPE_QUALIFIER_PCM)
#define TYPE_AIFF_PCM_FUNCTION          (unsigned long)(VOXTYPE_FORMAT_AIFF|VOXTYPE_LOCATION_FUNCTION|VOXTYPE_QUALIFIER_PCM)
#define TYPE_RAW_PCM_FUNCTION           (unsigned long)(VOXTYPE_FORMAT_RAW|VOXTYPE_LOCATION_FUNCTION|VOXTYPE_QUALIFIER_PCM)


#define TYPE_WAVE_VOX_FILE              (unsigned long)(VOXTYPE_FORMAT_WAVE|VOXTYPE_LOCATION_FILE|VOXTYPE_QUALIFIER_VOX)
//#define TYPE_AIFF_VOX_FILE            (unsigned long)(VOXTYPE_FORMAT_AIFF|VOXTYPE_LOCATION_FILE|VOXTYPE_QUALIFIER_VOX)
#define TYPE_RAW_VOX_FILE               (unsigned long)(VOXTYPE_FORMAT_RAW|VOXTYPE_LOCATION_FILE|VOXTYPE_QUALIFIER_VOX)

#define TYPE_WAVE_VOX_FSSPEC            (unsigned long)(VOXTYPE_FORMAT_WAVE|VOXTYPE_LOCATION_FSSPEC|VOXTYPE_QUALIFIER_VOX)
//#define TYPE_AIFF_VOX_FSSPEC          (unsigned long)(VOXTYPE_FORMAT_AIFF|VOXTYPE_LOCATION_FSSPEC|VOXTYPE_QUALIFIER_VOX)
#define TYPE_RAW_VOX_FSSPEC             (unsigned long)(VOXTYPE_FORMAT_RAW|VOXTYPE_LOCATION_FSSPEC|VOXTYPE_QUALIFIER_VOX)

#define TYPE_WAVE_VOX_BUFFER            (unsigned long)(VOXTYPE_FORMAT_WAVE|VOXTYPE_LOCATION_BUFFER|VOXTYPE_QUALIFIER_VOX)
#define TYPE_AIFF_VOX_BUFFER            (unsigned long)(VOXTYPE_FORMAT_AIFF|VOXTYPE_LOCATION_BUFFER|VOXTYPE_QUALIFIER_VOX)
#define TYPE_RAW_VOX_BUFFER             (unsigned long)(VOXTYPE_FORMAT_RAW|VOXTYPE_LOCATION_BUFFER|VOXTYPE_QUALIFIER_VOX)

#define TYPE_WAVE_VOX_FUNCTION          (unsigned long)(VOXTYPE_FORMAT_WAVE|VOXTYPE_LOCATION_FUNCTION|VOXTYPE_QUALIFIER_VOX)
#define TYPE_AIFF_VOX_FUNCTION          (unsigned long)(VOXTYPE_FORMAT_AIFF|VOXTYPE_LOCATION_FUNCTION|VOXTYPE_QUALIFIER_VOX)
#define TYPE_RAW_VOX_FUNCTION           (unsigned long)(VOXTYPE_FORMAT_RAW|VOXTYPE_LOCATION_FUNCTION|VOXTYPE_QUALIFIER_VOX)

#define TYPE_PLAY_WINMM                 (unsigned long)(VOXTYPE_LOCATION_SOUND|VOXTYPE_QUALIFIER_PCM)
#define TYPE_PLAY_MAC                   (unsigned long)(VOXTYPE_LOCATION_SOUND|VOXTYPE_QUALIFIER_PCM)

#define VOXTYPE_RECORD_PCM              (unsigned long)(VOXTYPE_LOCATION_SOUND|VOXTYPE_QUALIFIER_PCM)       // ###dave 03/25/96: added this

#define TYPE_RAW_VOX_BITSTREAM          (unsigned long)(VOXTYPE_FORMAT_RAW|VOXTYPE_LOCATION_BITSTREAM|VOXTYPE_QUALIFIER_VOX)

#define VOX_VOICE_FONT_PREVIEW_ID       (unsigned long)(VOXTYPE_FORMAT_WAVE|VOXTYPE_LOCATION_VOICE_FONT_ID|VOXTYPE_QUALIFIER_VOX)




/*****************************************************************************
**
**  Some defines that can be used with the ToolVox routines.
**
*****************************************************************************/

/* This is the maximum filename size that can be used with the ToolVox      */
/* routines.  Does not include the space for the NULL.                      */
#define MAX_FILENAMESIZE    255         /* char szFile[MAX_FILENAMESIZE+1]; */

#define WAVE_FORMAT_VOXWARE                 0x0062
// ###dave -lee we need to define _WAVE & _AIFF, and friends... (raw)

#define VOXWARE_WAVE_FILE_MONO              1
#define VOXWARE_WAVE_FILE_STEREO            2

#define VOXWARE_SAMPLING_RATE_8K            8000L
#define VOXWARE_SAMPLING_RATE_11K           11025L
#define VOXWARE_SAMPLING_RATE_11127K        0x2B7745D1L /*11127.27273 khz */
#define VOXWARE_SAMPLING_RATE_16K           16000L
#define VOXWARE_SAMPLING_RATE_22K           22050L
#define VOXWARE_SAMPLING_RATE_22254K        0x56EE8BA3L /*22254.54545 khz */
#define VOXWARE_SAMPLING_RATE_32K           32000L
#define VOXWARE_SAMPLING_RATE_44K           44100L

#define VOXWARE_SAMPLING_INDEX_8K           0
#define VOXWARE_SAMPLING_INDEX_11k          1
#define VOXWARE_SAMPLING_INDEX_16K          2
#define VOXWARE_SAMPLING_INDEX_22K          3
#define VOXWARE_SAMPLING_INDEX_32K          4
#define VOXWARE_SAMPLING_INDEX_44K          5
#define VOXWARE_SAMPLING_INDEX_11127K       6
#define VOXWARE_SAMPLING_INDEX_22254K       7


#define VOXWARE_SAMPLING_INDEX_END          (VOXWARE_SAMPLING_INDEX_22254K+1)
#define VOXWARE_NUM_SAMPLING_RATES          (VOXWARE_SAMPLING_INDEX_22254K+1)

#define VOXWARE_SAMPLING_RATE_ARRAY         {VOXWARE_SAMPLING_RATE_8K,    \
                                             VOXWARE_SAMPLING_RATE_11K,   \
                                             VOXWARE_SAMPLING_RATE_16K,   \
                                             VOXWARE_SAMPLING_RATE_22K,   \
                                             VOXWARE_SAMPLING_RATE_32K,   \
                                             VOXWARE_SAMPLING_RATE_44K,   \
                                             VOXWARE_SAMPLING_RATE_11127K,\
                                             VOXWARE_SAMPLING_RATE_22254K,\
                                             0L}

#define VOXWARE_SAMPLE_SIZE_8BIT            1
#define VOXWARE_SAMPLE_SIZE_16BIT           2

#define VOXWARE_SAMPLE_INDEX_8BIT           0
#define VOXWARE_SAMPLE_INDEX_16BIT          1

#define VOXWARE_SAMPLE_INDEX_END            (VOXWARE_SAMPLE_INDEX_16BIT+1)
#define VOXWARE_NUM_SAMPLE_SIZES            (VOXWARE_SAMPLE_INDEX_16BIT+1)

#define VOXWARE_SAMPLE_SIZE_ARRAY           {VOXWARE_SAMPLE_SIZE_8BIT,    \
                                             VOXWARE_SAMPLE_SIZE_16BIT,   \
                                             0L}



/*****************************************************************************
**
**  Some consts so you don't hard-coding anything!
**
*****************************************************************************/

#define GAIN_ENERGY_LEVEL_CLIPPING  (char) 200  /* You may be clipping if > */
                                                /* ...this level.           */
#define GAIN_ENERGY_LEVEL_TOO_LOW   (char) 150  /* Speaker should not go    */
                                                /* below this for very long.*/


#define MAX_WARPED_LENGTH_FACTOR     (float) 5.0
#define MIN_WARPED_LENGTH_FACTOR     (float) 0.2
#define DEFAULT_WARPED_LENGTH_FACTOR (float) 1.0


#define USE_ABSOLUTE_PITCH           TRUE
#define USE_RELATIVE_PITCH           FALSE  /* Default for bAbsolutePitchFlag */

#define MAX_RELATIVE_PITCH           (float) +24.0
#define MIN_RELATIVE_PITCH           (float) -24.0
#define DEFAULT_RELATIVE_PITCH       (float) 0.0

#define MAX_ABSOLUTE_PITCH           (float) 400.0
#define MIN_ABSOLUTE_PITCH           (float) 70.0


#define USE_ABSOLUTE_GAIN            TRUE
#define USE_RELATIVE_GAIN            FALSE  /* Default for bUseAbsoluteGainFlag */

#define MAX_RELATIVE_GAIN            (float) +100.0
#define MIN_RELATIVE_GAIN            (float) -100.0
#define DEFAULT_RELATIVE_GAIN        (float) 0.0

#define MAX_ABSOLUTE_GAIN            (float) 255.0
#define MIN_ABSOLUTE_GAIN            (float) 0.0

#define NO_VOICE_FONT                NULL


/* Assign the wVoiceFontType field to one of the following */
#define VOICE_FONT_VERSION_ONE_POINTER      (short) 0
#define VOICE_FONT_BETA_2_STRUCT            (short) 1



/*
    The manual say's that the minimum size buffer you should ever
    pass in is 512 bytes.  (Truth is, we don't need that much,
    but since I haven't called by psychic friend yet... I don't
    know what the future will hold -- :)
*/
#define VOX_MINIMUM_BUFFER_SIZE             (unsigned long) 512



/* Version format: vers.rel.maint.patch - this is version 1.1.8.192           */
#define TOOLVOX_VERSION_NUMBER              0x00010001L
#define TOOLVOX_MAINTENANCE_LEVEL           0x000800C0L

/* The following are obsolete spellings - use the constants #defined above.  */
#define VOXWARE_VERSION                     TOOLVOX_VERSION_NUMBER
#define VOXWARE_MAINT                       TOOLVOX_MAINTENANCE_LEVEL




/*****************************************************************************
**
**  Message values that are passed to the callback & the dwStatus field
**
*****************************************************************************/

#define VOXWARE_PLAYBACKERROR               10
#define VOXWARE_STARTCOMPRESS               11
#define VOXWARE_ENDCOMPRESS                 12
#define VOXWARE_STARTDECOMPRESS             13
#define VOXWARE_ENDDECOMPRESS               14
#define VOXWARE_STARTPLAY                   15
#define VOXWARE_ENDPLAY                     16
//#define VOXWARE_STARTPITCH                17
//#define VOXWARE_ENDPITCH                  18
#define VOXWARE_GETINFO                     19
#define VOXWARE_STARTRECORD                 20      /* Added in Version 1.1 */
#define VOXWARE_ENDRECORD                   21      /* Added in Version 1.1 */
#define VOXWARE_RECORDERROR                 22      /* Added in Version 1.1 */
#define VOXWARE_WAITING_TO_CLEANUP          23      /* Added in Version 1.1 */
#define VOXWARE_PAUSED                      24      /* Added in Version 1.2 */
// ###lee: where is SOUNDOUT ??


#define VOXWARE_FUNCTION_INIT               40      /* Added in Version 1.1 */
#define VOXWARE_FUNCTION_READ               41      /* Added in Version 1.1 */
#define VOXWARE_FUNCTION_WRITE              42      /* Added in Version 1.1 */
#define VOXWARE_FUNCTION_CLEANUP            43      /* Added in Version 1.1 */



/*****************************************************************************
**
**  Messages that are sent to the FILTER function
**
*****************************************************************************/

#define VOXWARE_FILTER_INIT                 50
#define VOXWARE_FILTER_PCM                  51
#define VOXWARE_FILTER_CLEANUP              52



/*****************************************************************************
**
**  Messages that are returned from the low level compress and decompress 
**  functions.
**
*****************************************************************************/

#define VOXWARE_NEED_MORE_INPUT_DATA        91
#define VOXWARE_OUTPUT_BUFFER_FULL          92
#define VOXWARE_USER_ABORTED                93
#define VOXWARE_USER_PAUSED                 94



/*****************************************************************************
**
**  Error return values.
**  All the errors are described in more detail in the file: tvgetstr.c
**
*****************************************************************************/

/* This is the return code for successful functions that work.              */
#define VOX_NO_ERROR                        00000

#define obsERR_BITS_PER_SAMPLE              101
#define obsERR_SAMPLE_RATE                  102
#define VOXERR_FORMAT_PCM                   103
#define VOXERR_NOT_MONO                     104
#define VOXERR_VOXWARE_DATA_POINTER_NULL    105
#define VOXERR_VOXWARE_DATA_SIZE            106
#define VOXERR_VOX_FILE_INFO_POINTER_NULL   107
#define VOXERR_VOX_FILE_INFO_SIZE           108
#define VOXERR_VOX_VERSION_POINTER_NULL     109
#define VOXERR_VOX_VERSION_SIZE             110
#define VOXERR_VOX_VOICE_POINTER_NULL       111
#define VOXERR_VOX_VOICE_NAME_NULL          112
#define VOXERR_MORPHIO_POINTER_BAD          113
#define VOXERR_INVALID_INPUT_TYPE           114
#define VOXERR_INVALID_OUTPUT_TYPE          115
#define VOXERR_INPUT_HANDLER                116
#define VOXERR_OUTPUT_HANDLER               117
#define VOXERR_PRIME_TIME                   118
#define VOXERR_WARP_2_BIG                   119
#define VOXERR_WARP_2_SMALL                 120
#define VOXERR_ABS_PITCH_2_BIG              121
#define VOXERR_ABS_PITCH_2_SMALL            122
#define VOXERR_REL_PITCH_2_BIG              123
#define VOXERR_REL_PITCH_2_SMALL            124
#define VOXERR_CODEC_UNKNOWN                125
#define VOXERR_INPUT_BYTES_PER_SAMPLE       126
#define VOXERR_OUTPUT_BYTES_PER_SAMPLE      127
#define VOXERR_FORMAT_VOX                   128
#define VOXERR_UNKNOWN_ATTRIBUTE            129
#define VOXERR_VWD_OR_VCB_IS_NULL           130
#define VOXERR_NO_MARKERS_IN_INPUT_FILE     131
#define VOXERR_BAD_SRC_4_MARKER_INPUT       132
#define VOXERR_UNSUPPORTED_MARKER_CHUNK     133
#define VOXERR_INVALID_MARKER_SELECTOR      134
#define VOXERR_NO_INPUT_SAMPLE_SIZE         135
#define VOXERR_NO_INPUT_SAMPLING_RATE       136
#define VOXERR_BAD_INPUT_SAMPLING_RATE      137
#define VOXERR_BAD_OUTPUT_SAMPLING_RATE     138

/* A spot check of your machine and system software revealed a slight problem. */
/* These are MacOS ONLY errors: */
#define VOXERR_VALIDATE_NO_GESTALT          139
#define VOXERR_VALIDATE_CPU_OLD_AS_DIRT     140
#define VOXERR_VALIDATE_NO_FPU              141
#define VOXERR_VALIDATE_FILE_MGR_TOO_OLD    142
#define VOXERR_VALIDATE_NO_NOTIFICATION_MGR 143
#define VOXERR_VALIDATE_YIKES_NO_SOUND_MGR  144
#define VOXERR_VALIDATE_U_LACK_16BIT_SND    145
#define VOXERR_VALIDATE_SND_MGR_NOT_CURRENT 146

#define VOXERR_DONT_GIVE_INPUT_SAMPLE_RATE  147
#define VOXERR_DONT_GIVE_INPUT_SAMPLE_SIZE  148
#define VOXERR_NO_SAMPLE_RATE_FOR_VOX       149
#define VOXERR_NO_SAMPLE_SIZE_FOR_VOX       150
#define VOXERR_EFFECTS_ARE_NOT_SUPPORTED    151
#define VOXERR_FX_R_4_DECOMPRESSION_ONLY    152
#define VOXERR_ONLY_RT_8K_CODEC_AVAILABLE   153
#define VOXERR_INVALID_MARKER_SIZEOF        154
#define VOXERR_INVALID_VCB                  155
#define VOXERR_INVALID_OUTPUT_VARS          156
#define VOXERR_INVALID_INPUT_VARS           157
#define VOXERR_ABS_GAIN_2_BIG               158
#define VOXERR_ABS_GAIN_2_SMALL             159
#define VOXERR_REL_GAIN_2_BIG               160
#define VOXERR_REL_GAIN_2_SMALL             161
#define VOXERR_VOXWARE_ALREADY_INITIALIZED  162
#define VOXERR_MUST_PLAY_TO_LOCATION_SOUND  163 // new, not yet in manual
#define VOXERR_MUST_CALL_DCMP_TOOLVOX_PLAY  164 // new, not yet in manual
#define VOXERR_FPU_DETECT                   165 // new, not yet in manual
#define VOXERR_IDENTIFYING_THE_CPU          166 // new, not yet in manual
#define obsERR_IDENTIFYING_THE_OS           167 // bozo sabotage error... never use this.
#define VOXERR_CODEC_NOT_AVAILABLE          168 // new, not yet in manual
#define obsERR_DONT_GIVE_CODEC              169 // bozo sabotage error... never use this.
#define VOXERR_DONT_GIVE_CMP_SETTINGS       170 // new, not yet in manual
#define VOXERR_LINK_WITH_NATIVE_CODE        171 // mac only: new, not yet in manual


/* File I/O error codes.                                                    */
#define obsERR_DATA_CHUNK_SEEK              201
#define obsERR_FMT_CHUNK_READ               202
#define VOXERR_FMT_CHUNK_SIZE               203
#define VOXERR_RIFF_CHUNK_READ              204
#define VOXERR_VOX_HEADER_FLAG              205
#define VOXERR_VOX_HEADER_SIZE              206
#define obsERR_VOX_INIT_READ                207
#define obsERR_VOX_INIT_WRITE               208
#define obsERR_VOX_OPEN                     209
#define obsERR_VOX_READ                     210
#define obsERR_VOX_WRITE                    211
#define obsERR_WAVE_CKID                    212
#define obsERR_WAVE_INIT_READ               213
#define obsERR_WAVE_INIT_WRITE              214
#define obsERR_WAVE_OPEN                    215
#define obsERR_WAVE_READ                    216
#define obsERR_WAVE_WRITE                   217
#define VOXERR_VFONT_OPEN                   218                                 
#define VOXERR_VFONT_READ                   219
#define VOXERR_VFONT_WRITE                  220
#define VOXERR_ENDOFFILE_REACHED            221
#define VOXERR_READING_FROM_FILE            222
#define VOXERR_WRITING_TO_FILE              223
#define VOXERR_BAD_PATH_TO_FILE             224
#define VOXERR_SET_FPOS                     225
#define VOXERR_OPENING_INPUT_FILE           226
#define VOXERR_OPENING_OUTPUT_FILE          227
#define VOXERR_CLOSING_INPUT_FILE           228
#define VOXERR_CLOSING_OUTPUT_FILE          229
#define VOXERR_READING_MARKER               230
#define VOXERR_SEEKING_TO_MARKER_CHUNK      231
#define VOXERR_MARKER_SEEK_RESET            232
#define VOXERR_FAILED_SEEK_TO_MARKER        233
#define VOXERR_REOPENING_MARKER_LOG         234
#define VOXERR_OPENING_MARKER_LOG           235
#define VOXERR_FAILED_TO_GET_EOF            236
#define VOXERR_SEEKING_PAST_HEADER          237
#define VOXERR_ENDOFBUFFER_REACHED          238


/* Format/Header error codes                                                */
#define WheReIsErOrNuMbErThReEoOnE          301     //###lee
#define VOXERR_UNKNOWN_HEADER_FORMAT        302
#define VOXERR_WE_DONT_SUPPORT_RIFX         303
#define VOXERR_WAVE_DOESNT_START_RIFF       304
#define VOXERR_CANT_SURF_THIS_WAVE          305
#define VOXERR_AIFF_COULDNT_FIND_ALL_CHUNKS 306
#define VOXERR_HDR_MISSING_BITS_OF_DATA     307
#define VOXERR_NOT_A_COMPRESSED_HEADER      308
#define VOXERR_NO_CODECID_FOR_HEADER        309     // new MJ51
#define VOXERR_UNKNOWN_CODECID_IN_HEADER    310     // new MJ67


/* Memory allocation error codes.                                           */
#define VOXERR_PLAYBACK_MEMORY              401
#define VOXERR_VOXSTRUCT_MEMORY             402
#define obsERR_VOX_DATA_MEMORY              403
#define obsERR_WAVE_DATA_MEMORY             404
#define VOXERR_WAVEHDR_MEMORY               405
#define VOXERR_VER_INFO_MEMORY              406
#define VOXERR_MORPHIO_MEMORY               407
#define VOXERR_INPUT_VARS_MEMORY            408
#define VOXERR_OUTPUT_VARS_MEMORY           409
#define VOXERR_RECORD_MEMORY                410
#define VOXERR_MARKERS_MEMORY               411
#define VOXERR_FILELINK_MEMORY              412
#define VOXERR_READ_COMPLETION_MEMORY       413
#define VOXERR_INPUT_DISKIO_MEMORY          414
#define VOXERR_OUTPUT_DISKIO_MEMORY         415
#define VOXERR_FLUSH_BUFFER_MEMORY          416
#define VOXERR_FUNCIN_BUFFER_MEMORY         417
#define VOXERR_FUNCOUT_BUFFER_MEMORY        418
#define VOXERR_DEFERRED_TASK_PROC_MEMORY    419
#define VOXERR_DEFERRED_TASK_MEMORY         420
#define VOXERR_SOUND_CALLBACK_PROC_MEMORY   421
#define VOXERR_NOTIFICATION_REC_MEMORY      422
#define VOXERR_PLAYBACK_VARS_MEMORY         423
#define VOXERR_PLAYBACK_BUFFER_MEMORY       424
#define VOXERR_ORD_MARKERS_MEMORY           425
#define VOXERR_MARKERS_TEMP_CHUNK_MEMORY    426
#define VOXERR_WAVE_MARKER_CHUNK_MEMORY     427
#define VOXERR_GENERIC_MARKERS_MEMORY       428
#define VOXERR_TEMP_INPUT_BUFFER_MEMORY     429


/* General system error codes.                                              */
#define VOXERR_BREW_BLACK_ART               501
#define VOXERR_COMPRESSION                  502
#define VOXERR_CREATE_WINDOW                503
#define VOXERR_DECOMPRESSION                504
#define VOXERR_WAVEOUT_OPEN                 505
#define VOXERR_PREPARE_HDR                  506
#define VOXERR_WAVEOUT_WRITE                507
#define VOXERR_WAVEIN_ADD_BUFFER            508
#define VOXERR_WAVEIN_OPEN                  509
#define VOXERR_WAVEIN_START                 510
#define VOXERR_FUNCTION_EXPIRED             511
#define VOXERR_NOT_ENOUGH_DATA_TO_COMPRESS  512
#define VOXERR_COMPRESS_BUFFER              513
#define VOXERR_PROCESSING_ABORTED           514 // new, not yet in manual
#define VOXERR_PAUSING                      515 // new MJ44 -- not yet in manual
#define VOXERR_RESUMING                     516 // new MJ44 -- not yet in manual
#define VOXERR_FLUSHCMD                     517 // new MJ44 -- not yet in manual
#define VOXERR_QUIETCMD                     518 // new MJ44 -- not yet in manual
#define VOXERR_BUFFERCMD                    519 // new MJ44 -- not yet in manual
#define VOXERR_CALLBACKCMD                  520 // new MJ44 -- not yet in manual
#define VOXERR_DTINSTALL_FAILED             521 // new MJ44 -- not yet in manual


/* Errors that occur using the VoiceFonts API.                              */
#define VOXERR_VFONT_FILE_NOT_FOUND         601
#define VOXERR_VFONT_WRONGNAME              602
#define VOXERR_VFONT_DELETE                 603
#define VOXERR_VFONT_FILE_FORMAT            604
#define VOXERR_MORPHIO_HANDLE_INVALID       605
#define VOXERR_VFII_UNKNOWN_VOICE_FONT      606 // new MJ50 -- not yet in manual
#define VOXERR_VFII_UNKNOWN_VF_CHUNK_SIZE   607 // new MJ50 -- not yet in manual







/*****************************************************************************
**
**  Header routines to convert from digitized speech to VOX data.  They all 
**  take PCM data as input and output compressed VOX data.
**
*****************************************************************************/


/* New calls that have the properties of all of the other calls.  The routine
** use the dwInputType and dwOutputType fields to tell how to process the data.
** This makes the older calls obsolete (yet still supported).
*/
VOXWARE_RETCODE VOXAPI cmpToolVoxRecord(LPVOXWARE_DATA lpVoxwareData);
VOXWARE_RETCODE VOXAPI cmpToolVoxCompress(LPVOXWARE_DATA lpVoxwareData);


/* These routines are used for low-level compression.                       */ 
VOXWARE_RETCODE VOXAPI cmpVoxInit(LPVOXWARE_DATA lpVoxwareData);
VOXWARE_RETCODE VOXAPI cmpVoxConvert(LPVOXWARE_DATA lpVoxwareData);
VOXWARE_RETCODE VOXAPI cmpVoxFree(LPVOXWARE_DATA lpVoxwareData);


/* Older functions that are only here for backwards compatibility. */
VOXWARE_RETCODE VOXAPI cmpFileToVoxFile(LPVOXWARE_DATA lpVoxwareData);
VOXWARE_RETCODE VOXAPI cmpFileToVoxBuffer(LPVOXWARE_DATA lpVoxwareData);
VOXWARE_RETCODE VOXAPI cmpBufferToVoxFile(LPVOXWARE_DATA lpVoxwareData);




/*****************************************************************************
**
**  Header routines to convert from VOX data to digitized speech.  They all 
**  take VOX data as input (either as VOX file or a buffer of VOX data) and 
**  output decompressed PCM data.  There is also a routine to play VOX or PCM
**  data.
**
*****************************************************************************/

VOXWARE_RETCODE VOXAPI dcmpToolVoxPlay(LPVOXWARE_DATA lpVoxwareData);
VOXWARE_RETCODE VOXAPI dcmpToolVoxDecompress(LPVOXWARE_DATA lpVoxwareData);
VOXWARE_RETCODE VOXAPI dcmpToolVoxStop(LPVOXWARE_DATA lpVoxwareData);           		// MacOS only
VOXWARE_RETCODE VOXAPI dcmpToolVoxPause(LPVOXWARE_DATA lpVoxwareData);                  // Hidden API don't use.
VOXWARE_RETCODE VOXAPI dcmpToolVoxResume(LPVOXWARE_DATA lpVoxwareData);                 // Hidden API don't use.
VOXWARE_RETCODE VOXAPI dcmpSetBitOffset(LPVOXWARE_DATA lpVoxwareData, short newOffset); // Hidden API don't use.


/* These routines are used for low-level decompression.                     */ 
VOXWARE_RETCODE VOXAPI dcmpVoxInit(LPVOXWARE_DATA lpVoxwareData);
VOXWARE_RETCODE VOXAPI dcmpVoxConvert(LPVOXWARE_DATA lpVoxwareData);
VOXWARE_RETCODE VOXAPI dcmpVoxFree(LPVOXWARE_DATA lpVoxwareData);


/* Older functions that are only here for backwards compatibility. */
VOXWARE_RETCODE VOXAPI dcmpVoxFileToFile(LPVOXWARE_DATA lpVoxwareData);
VOXWARE_RETCODE VOXAPI dcmpVoxFileToBuffer(LPVOXWARE_DATA lpVoxwareData);
VOXWARE_RETCODE VOXAPI dcmpVoxBufferToFile(LPVOXWARE_DATA lpVoxwareData);
VOXWARE_RETCODE VOXAPI dcmpPlayVoxFile(LPVOXWARE_DATA lpVoxwareData);
VOXWARE_RETCODE VOXAPI dcmpPlayVoxBuffer(LPVOXWARE_DATA lpVoxwareData);




/*****************************************************************************
**
**  These utility routines can be used to get information about the speed of 
**  the ToolVox compression or to get file information about an AIFF or VOX 
**  file.  There are also routines to get and set different attributes of the
**  ToolVox API.             
**
*****************************************************************************/

VOXWARE_RETCODE VOXAPI utilGetFileInfo(LPVOXWARE_DATA lpVoxwareData);
VOXWARE_RETCODE VOXAPI utilGetVersionInfo(LPVOXWARE_DATA lpVoxwareData);
VOXWARE_RETCODE VOXAPI utilToolVoxGetAttribute(
                            LPVOXWARE_DATA lpVoxwareData,
                            LPVOX_COMMAND lpCommand);
VOXWARE_RETCODE VOXAPI utilToolVoxSetAttribute(
                            LPVOXWARE_DATA lpVoxwareData,
                            LPVOX_COMMAND lpCommand);




/*****************************************************************************
**
**  This prototype is for the callback function that can be passed to ToolVox 
**  in the VOXWARE_DATA structure.  If you create this function, it must be
**  in a FIXED/Locked segment!
**
*****************************************************************************/

VOXAPI_CALLBACK VoxFunc(
                    unsigned short wVox, 
                    unsigned short wMessage, 
                    LPVOXWARE_DATA lpVoxwareData);




#ifndef VOXWARE_SUN
    #if PRAGMA_ALIGN_SUPPORTED
        #pragma options align=reset
    #elif VOXWARE_HP
        #pragma HP_ALIGN HPUX_NATURAL
    #else
        #pragma pack()
    #endif
#endif



#ifdef __cplusplus
}
#endif



#endif /*__TOOLVOX_H_*/
