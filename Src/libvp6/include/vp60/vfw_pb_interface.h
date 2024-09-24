/****************************************************************************
*
*   Module Title :     vfw_pb_interface.h
*
*   Description  :     Codec interface specification header file.
*
****************************************************************************/
#ifndef __INC_VFW_PB_INTERFACE
#define __INC_VFW_PB_INTERFACE

/****************************************************************************
*  Header Files
****************************************************************************/
#include "codec_common_interface.h"
#include "type_aliases.h"
#ifdef __GNUC__
#include <inttypes.h>
#elif defined(_WIN32)
#include <stddef.h>
#endif
/****************************************************************************
*  Typedefs
****************************************************************************/
typedef struct PB_INSTANCE * xPB_INST;

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
    PBC_SET_DEINTERLACEMODE,
    PBC_SET_ADDNOISE

} PB_COMMAND_TYPE;

/****************************************************************************
*  Exports
****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

extern BOOL CCONV VP6_StartDecoder ( xPB_INST *pbi, UINT32  ImageWidth, UINT32  ImageHeight );
extern void CCONV VP6_GetPbParam ( xPB_INST, PB_COMMAND_TYPE Command, UINT32 *Parameter );
extern void CCONV VP6_SetPbParam ( xPB_INST, PB_COMMAND_TYPE Command, uintptr_t Parameter );
extern void CCONV VP6_GetYUVConfig ( xPB_INST, YUV_BUFFER_CONFIG * YuvConfig );
extern const char * CCONV VP31D_GetVersionNumber ( void );
extern int CCONV VP6_DecodeFrameToYUV ( xPB_INST, char * VideoBufferPtr, unsigned int ByteCount);
extern BOOL CCONV VP6_StopDecoder ( xPB_INST *pbi );

#ifdef __cplusplus
}
#endif

#endif
