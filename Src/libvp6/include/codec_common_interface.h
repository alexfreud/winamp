/****************************************************************************
*
*   Module Title :     codec_common_if.h
*
*   Description  :     Common codec definitions.
*
****************************************************************************/
#ifndef __INC_CODEC_COMMON_INTERFACE_H
#define __INC_CODEC_COMMON_INTERFACE_H

/****************************************************************************
* Macros
****************************************************************************/
#define __export   
#define _export  
#define DllExport   __declspec( dllexport )
#define DllImport   __declspec( dllimport )

// Playback error codes 
#define NO_DECODER_ERROR			0
#define REMOTE_DECODER_ERROR        -1

#define	DFR_BAD_DCT_COEFF			-100
#define	DFR_ZERO_LENGTH_FRAME		-101
#define DFR_FRAME_SIZE_INVALID		-102
#define DFR_OUTPUT_BUFFER_OVERFLOW	-103
#define DFR_INVALID_FRAME_HEADER    -104
#define FR_INVALID_MODE_TOKEN       -110
#define ETR_ALLOCATION_ERROR		-200
#define ETR_INVALID_ROOT_PTR		-201
#define SYNCH_ERROR					-400
#define BUFFER_UNDERFLOW_ERROR		-500
#define PB_IB_OVERFLOW_ERROR        -501

// External error triggers
#define PB_HEADER_CHECKSUM_ERROR    -601
#define PB_DATA_CHECKSUM_ERROR      -602

// DCT Error Codes
#define DDCT_EXPANSION_ERROR        -700
#define DDCT_INVALID_TOKEN_ERROR    -701

// ExceptionErrors
#define GEN_EXCEPTIONS              -800
#define EX_UNQUAL_ERROR             -801

// Unrecoverable error codes
#define FATAL_PLAYBACK_ERROR        -1000
#define GEN_ERROR_CREATING_CDC		-1001
#define GEN_THREAD_CREATION_ERROR   -1002
#define DFR_CREATE_BMP_FAILED		-1003

/****************************************************************************
* Typedefs
****************************************************************************/
typedef struct      // YUV buffer configuration structure
{
    int   YWidth;
    int   YHeight;
    int   YStride;

    int   UVWidth;
    int   UVHeight;
    int   UVStride;

    char *YBuffer;
    char *UBuffer;
    char *VBuffer;

} YUV_BUFFER_CONFIG;

typedef enum
{
    C_SET_KEY_FRAME,
    C_SET_FIXED_Q,
    C_SET_FIRSTPASS_FILE,
    C_SET_EXPERIMENTAL_MIN,
    C_SET_EXPERIMENTAL_MAX = C_SET_EXPERIMENTAL_MIN + 255,
	C_SET_CHECKPROTECT,
	C_SET_TESTMODE,
	C_SET_INTERNAL_SIZE,
	C_SET_RECOVERY_FRAME,
	C_SET_REFERENCEFRAME,
    C_SET_GOLDENFRAME
} C_SETTING;

typedef enum
{
    MAINTAIN_ASPECT_RATIO = 0x0,
    SCALE_TO_FIT          = 0x1,
    CENTER                = 0x2,
    OTHER                 = 0x3
} SCALE_MODE;

#endif
