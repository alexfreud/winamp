/****************************************************************************
*
*   Module Title :     VFW_PB_INTERFACE.H
*
*   Description  :     Interface to video codec demo decompressor DLL
*
*   AUTHOR       :     Paul Wilkins
*
*****************************************************************************
*   Revision History
* 
*   1.03 YWX 17/Dec/02 Added enum for setup DeInteralcedMode
*   1.02 JBB 25 AUG 00 Versioning
*   1.01 PGW 29/06/99  Interface to DecodeFrame() changed.
*   1.00 PGW 07/06/99  Baseline code.
*
*****************************************************************************
*/

#ifndef VFW_PB_INTERFACE
#define VFW_PB_INTERFACE

#include "codec_common_interface.h"

#include "type_aliases.h"
typedef struct PB_INSTANCE * xPB_INST;
//#include "pbdll.h"


//	Settings Control	
typedef enum
{
	PBC_SET_POSTPROC,
	PBC_SET_CPUFREE,
    PBC_MAX_PARAM,
	PBC_SET_TESTMODE,
	PBC_SET_PBSTRUCT,
	PBC_SET_BLACKCLAMP,
	PBC_SET_WHITECLAMP,
	PBC_SET_REFERENCEFRAME,
    PBC_SET_DEINTERLACEMODE

} PB_COMMAND_TYPE;


#ifdef __cplusplus
extern "C"
{
#endif

extern BOOL CCONV VP5_StartDecoder( xPB_INST *pbi, UINT32  ImageWidth, UINT32  ImageHeight );
extern void CCONV VP5_GetPbParam( xPB_INST, PB_COMMAND_TYPE Command, UINT32 *Parameter );
extern void CCONV VP5_SetPbParam( xPB_INST, PB_COMMAND_TYPE Command, UINT32 Parameter );
extern void CCONV VP5_GetYUVConfig( xPB_INST, YUV_BUFFER_CONFIG * YuvConfig );
extern const char * CCONV VP31D_GetVersionNumber(void);

extern int CCONV VP5_DecodeFrameToYUV( xPB_INST, char * VideoBufferPtr, unsigned int ByteCount,
                                    UINT32 ImageWidth, UINT32 ImageHeight );
extern BOOL CCONV VP5_StopDecoder(xPB_INST *pbi);

#ifdef __cplusplus
}
#endif
#endif


