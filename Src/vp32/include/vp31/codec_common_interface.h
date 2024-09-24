//==========================================================================
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1999 - 2001  On2 Technologies Inc. All Rights Reserved.
//
//--------------------------------------------------------------------------


/****************************************************************************
*
*   Module Title :     codec_common_if.H
*
*   Description  :     Interface to video codec demo decompressor DLL
*
*****************************************************************************
*/

#ifndef CODEC_COMMON_INTERFACE_H
#define CODEC_COMMON_INTERFACE_H

#define __export   
#define _export  
#define DllExport   __declspec( dllexport )
#define DllImport   __declspec( dllimport )

// Playback ERROR Codes. 
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

#endif