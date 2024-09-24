#ifndef CDDBPLUGIN_WORKORDERMGR_H
#define CDDBPLUGIN_WORKORDERMGR_H

// sig gen headers
#include "CDDBPlugInBase.h"

#define		CDDBMODULE_WORKORDERMGR_IF_NAME		"workordermanagerModuleID"
#define		CDDBMODULE_WORKORDERMGR				0x10000

#define		CDDBMODULE_QUERY_INTERFACE_NAME		"CDDBModuleQueryInterface"

// supported interfaces
#define WORK_ORDER_MANAGER_BASE_INTERFACE "CDDBModuleInterface"
#define WORK_ORDER_MANAGER_INTERFACE "workordermanager"
#define WORK_ORDER_MANAGER_INTERFACE2 "workordermanager2"

#ifndef CDDB_PLUGIN_SIGGEN_H

typedef enum
{
	SG_NoError                = 0,
	SG_SignatureAcquired      = 1,
	SG_SignatureNotAcquired   = 2,
	SG_UnsupportedFormat      = 3,
	SG_ProcessingError        = 4,
	SG_InitializationError    = 5,
	SG_DeinitializationError  = 6,
	SG_InvalidParamError      = 7,
	SG_InternalError          = 8,
	SG_NotInitializedError    = 9,
	SG_OutOfMemory            = 10,
	SG_NotImplementedError    = 11
}
SigGenResultCode;

#endif //#ifndef CDDB_PLUGIN_SIGGEN_H


//
//
//
#ifndef CDDBMODULEWORKORDERMGR
#define CDDBMODULEWORKORDERMGR

#define	CDDBMODULE_WORKORDER_MGR_VERSION	1

typedef struct WorkOrderInstance* WorkOrderHandle;

typedef struct
{
	CDDBModuleInterface			base;

	unsigned int version;  /* current version is defined by CDDBMODULE_WORKORDER_MGR_VERSION */
	unsigned int size;     /* sizeof(CDDBModuleWorkOrder) */
	unsigned int flags;    /* nothing defined yet */

	int (__stdcall *Initialize)(void* cddbcontrol, char* path);
	int (__stdcall *Shutdown)(void);

	int (__stdcall *GetSigHandle)(void** handle, void* disc, long track_num);
	int (__stdcall *WriteSigData)(void* handle, void* data, long size);
	int (__stdcall *CloseSig)(void* handle);
	int (__stdcall *AbortSig)(void* handle);

} CDDBModuleWorkOrderManagerInterface;

typedef struct
{
	CDDBModuleInterface			base;

	/*  SetAlwaysGenerate
	* Description: Enables/disables a Work Order Plugin DLL to always generate
	*				a signature, regardless of work orders.
	*
	* Args:	dll_filepath	- full path and filename of the plugin DLL
	*						for example: (C:\App\Cddb12Tone.dll)
	*		b_always_generate	- enable/disable ignoring work orders
	*						for example: (0 or 1)
	*
	* Returns:	0 for success or an error.
	*			Failure conditions include:
	*				Invalid argument
	*/
	int (__stdcall *SetAlwaysGenerate)(char* dll_filepath,long b_always_generate);

} CDDBModuleWorkOrderManagerInterface2;

#endif	/* CDDBMODULEWORKORDERMGR */

#endif /* CDDBPLUGIN_WORKORDERMGR_H */
