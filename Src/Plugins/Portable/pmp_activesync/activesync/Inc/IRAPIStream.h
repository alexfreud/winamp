

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Fri Sep 17 22:09:50 2004
 */
/* Compiler settings for ..\IRAPIStream.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __IRAPIStream_h__
#define __IRAPIStream_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IRAPIStream_FWD_DEFINED__
#define __IRAPIStream_FWD_DEFINED__
typedef interface IRAPIStream IRAPIStream;
#endif 	/* __IRAPIStream_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "rapitypes.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IRAPIStream_INTERFACE_DEFINED__
#define __IRAPIStream_INTERFACE_DEFINED__

/* interface IRAPIStream */
/* [object][uuid] */ 


EXTERN_C const IID IID_IRAPIStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("449FE623-24B0-454b-A889-129BB05DDBED")
    IRAPIStream : public IStream
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetRapiStat( 
            /* [in] */ RAPISTREAMFLAG Flag,
            /* [in] */ DWORD dwValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRapiStat( 
            /* [in] */ RAPISTREAMFLAG Flag,
            /* [out] */ DWORD *pdwValue) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRAPIStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRAPIStream * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRAPIStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRAPIStream * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Read )( 
            IRAPIStream * This,
            /* [length_is][size_is][out] */ void *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG *pcbRead);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Write )( 
            IRAPIStream * This,
            /* [size_is][in] */ const void *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG *pcbWritten);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Seek )( 
            IRAPIStream * This,
            /* [in] */ LARGE_INTEGER dlibMove,
            /* [in] */ DWORD dwOrigin,
            /* [out] */ ULARGE_INTEGER *plibNewPosition);
        
        HRESULT ( STDMETHODCALLTYPE *SetSize )( 
            IRAPIStream * This,
            /* [in] */ ULARGE_INTEGER libNewSize);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *CopyTo )( 
            IRAPIStream * This,
            /* [unique][in] */ IStream *pstm,
            /* [in] */ ULARGE_INTEGER cb,
            /* [out] */ ULARGE_INTEGER *pcbRead,
            /* [out] */ ULARGE_INTEGER *pcbWritten);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            IRAPIStream * This,
            /* [in] */ DWORD grfCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Revert )( 
            IRAPIStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *LockRegion )( 
            IRAPIStream * This,
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *UnlockRegion )( 
            IRAPIStream * This,
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *Stat )( 
            IRAPIStream * This,
            /* [out] */ STATSTG *pstatstg,
            /* [in] */ DWORD grfStatFlag);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IRAPIStream * This,
            /* [out] */ IStream **ppstm);
        
        HRESULT ( STDMETHODCALLTYPE *SetRapiStat )( 
            IRAPIStream * This,
            /* [in] */ RAPISTREAMFLAG Flag,
            /* [in] */ DWORD dwValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetRapiStat )( 
            IRAPIStream * This,
            /* [in] */ RAPISTREAMFLAG Flag,
            /* [out] */ DWORD *pdwValue);
        
        END_INTERFACE
    } IRAPIStreamVtbl;

    interface IRAPIStream
    {
        CONST_VTBL struct IRAPIStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRAPIStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRAPIStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRAPIStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRAPIStream_Read(This,pv,cb,pcbRead)	\
    (This)->lpVtbl -> Read(This,pv,cb,pcbRead)

#define IRAPIStream_Write(This,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> Write(This,pv,cb,pcbWritten)


#define IRAPIStream_Seek(This,dlibMove,dwOrigin,plibNewPosition)	\
    (This)->lpVtbl -> Seek(This,dlibMove,dwOrigin,plibNewPosition)

#define IRAPIStream_SetSize(This,libNewSize)	\
    (This)->lpVtbl -> SetSize(This,libNewSize)

#define IRAPIStream_CopyTo(This,pstm,cb,pcbRead,pcbWritten)	\
    (This)->lpVtbl -> CopyTo(This,pstm,cb,pcbRead,pcbWritten)

#define IRAPIStream_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define IRAPIStream_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define IRAPIStream_LockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> LockRegion(This,libOffset,cb,dwLockType)

#define IRAPIStream_UnlockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> UnlockRegion(This,libOffset,cb,dwLockType)

#define IRAPIStream_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#define IRAPIStream_Clone(This,ppstm)	\
    (This)->lpVtbl -> Clone(This,ppstm)


#define IRAPIStream_SetRapiStat(This,Flag,dwValue)	\
    (This)->lpVtbl -> SetRapiStat(This,Flag,dwValue)

#define IRAPIStream_GetRapiStat(This,Flag,pdwValue)	\
    (This)->lpVtbl -> GetRapiStat(This,Flag,pdwValue)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRAPIStream_SetRapiStat_Proxy( 
    IRAPIStream * This,
    /* [in] */ RAPISTREAMFLAG Flag,
    /* [in] */ DWORD dwValue);


void __RPC_STUB IRAPIStream_SetRapiStat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPIStream_GetRapiStat_Proxy( 
    IRAPIStream * This,
    /* [in] */ RAPISTREAMFLAG Flag,
    /* [out] */ DWORD *pdwValue);


void __RPC_STUB IRAPIStream_GetRapiStat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRAPIStream_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


