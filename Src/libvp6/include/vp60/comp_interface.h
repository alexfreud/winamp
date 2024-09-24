/****************************************************************************
*
*   Module Title :     comp_interface.h
*
*   Description  :     Interface to video codec demo compressor DLL
*
****************************************************************************/
#ifndef __INC_COMP_INTERFACE_H
#define __INC_COMP_INTERFACE_H

/****************************************************************************
* Include Files
****************************************************************************/
#include "type_aliases.h"
#include "codec_common_interface.h"
#include "vp60_comp_interface.h"

/****************************************************************************
* Macros
****************************************************************************/
//  C4514  Unreferenced inline function has been removed
#ifdef _MSC_VER
#pragma warning(disable: 4514)
#endif
/****************************************************************************
* Typedefs
****************************************************************************/
typedef struct CP_INSTANCE * xCP_INST;

/****************************************************************************
* Exports
****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

extern BOOL CCONV StartEncoder ( xCP_INST *cpi, COMP_CONFIG_VP6 *CompConfig );
extern void CCONV ChangeCompressorSetting ( xCP_INST cpi, C_SETTING Setting, int Value );
extern void CCONV ChangeEncoderConfig (  xCP_INST cpi, COMP_CONFIG_VP6 *CompConfig );
extern UINT32 CCONV EncodeFrame (  xCP_INST cpi, unsigned char *InBmpIPtr, unsigned char *InBmpPtr, unsigned char *OutPutPtr, unsigned int *is_key );
extern UINT32 CCONV EncodeFrameYuv (  xCP_INST cpi, YUV_INPUT_BUFFER_CONFIG *YuvInputData, unsigned char *OutPutPtr, unsigned int *is_key );
extern BOOL CCONV StopEncoder ( xCP_INST *cpi );
extern void VPEInitLibrary ( void );
extern void VPEDeInitLibrary ( void );
extern const char *CCONV VP50E_GetVersionNumber ( void );
extern UINT32 CCONV VPGetState ( xCP_INST cpi, void *ret );
extern void CCONV VPSetState ( xCP_INST cpi, void *ret );
extern int CCONV VPGetPB ( xCP_INST cpi );

#ifdef __cplusplus
}
#endif

#endif
