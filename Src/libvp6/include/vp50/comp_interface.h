#if !defined(COMP_INTERFACE_H)
#define COMP_INTERFACE_H
/****************************************************************************
*
*   Module Title :     COMP_INTERFACE.H
*
*   Description  :     Interface to video codec demo compressor DLL
*
*   AUTHOR       :     Paul Wilkins
*
*****************************************************************************
*   Revision History
* 
*   1.04 JBB 26 AUG 00 JBB Added fixed q setting
*   1.03 PGW 07/12/99  Retro fit JBB changes
*   1.02 PGW 16/09/99  Interface changes to simplify things for command line 
*                      compressor.
*   1.01 PGW 07/07/99  Added COMP_CONFIG.
*   1.00 PGW 28/06/99  New configuration baseline
*
*****************************************************************************
*/

//  C4514  Unreferenced inline function has been removed
#pragma warning(disable: 4514)

#include "codec_common_interface.h"
#include "type_aliases.h"
#include "vp50_comp_interface.h"

#ifdef __cplusplus
extern "C"
{
#endif
typedef struct CP_INSTANCE * xCP_INST;
extern BOOL CCONV StartEncoder( xCP_INST *cpi, COMP_CONFIG_VP5 * CompConfig );
extern void CCONV ChangeCompressorSetting ( xCP_INST cpi, C_SETTING Setting, int Value );
extern void CCONV ChangeEncoderConfig (  xCP_INST cpi, COMP_CONFIG_VP5 * CompConfig );
extern UINT32 CCONV EncodeFrame(  xCP_INST cpi, unsigned char * InBmpIPtr, unsigned char * InBmpPtr, unsigned char * OutPutPtr, unsigned int * is_key );
extern UINT32 CCONV EncodeFrameYuv(  xCP_INST cpi, YUV_INPUT_BUFFER_CONFIG *  YuvInputData, unsigned char * OutPutPtr, unsigned int * is_key );
extern BOOL CCONV StopEncoder( xCP_INST *cpi);
extern void VPEInitLibrary(void);
extern void VPEDeInitLibrary(void);
extern const char * CCONV VP50E_GetVersionNumber(void);
extern UINT32 CCONV VPGetState(xCP_INST cpi, void * ret);
extern void CCONV VPSetState(xCP_INST cpi, void * ret);
extern int CCONV VPGetPB(xCP_INST cpi);
#ifdef __cplusplus

}
#endif


#endif 