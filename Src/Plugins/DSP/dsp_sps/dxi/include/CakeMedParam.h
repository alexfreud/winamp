/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Fri May 03 10:13:47 2002
 */
/* Compiler settings for CakeMedParam.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifndef __CakeMedParam_h__
#define __CakeMedParam_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IMediaParamsUICallback_FWD_DEFINED__
#define __IMediaParamsUICallback_FWD_DEFINED__
typedef interface IMediaParamsUICallback IMediaParamsUICallback;
#endif 	/* __IMediaParamsUICallback_FWD_DEFINED__ */


#ifndef __IMediaParamsSetUICallback_FWD_DEFINED__
#define __IMediaParamsSetUICallback_FWD_DEFINED__
typedef interface IMediaParamsSetUICallback IMediaParamsSetUICallback;
#endif 	/* __IMediaParamsSetUICallback_FWD_DEFINED__ */


#ifndef __IMediaParamsCapture_FWD_DEFINED__
#define __IMediaParamsCapture_FWD_DEFINED__
typedef interface IMediaParamsCapture IMediaParamsCapture;
#endif 	/* __IMediaParamsCapture_FWD_DEFINED__ */


#ifndef __IMediaParamsUICallback_FWD_DEFINED__
#define __IMediaParamsUICallback_FWD_DEFINED__
typedef interface IMediaParamsUICallback IMediaParamsUICallback;
#endif 	/* __IMediaParamsUICallback_FWD_DEFINED__ */


#ifndef __IMediaParamsSetUICallback_FWD_DEFINED__
#define __IMediaParamsSetUICallback_FWD_DEFINED__
typedef interface IMediaParamsSetUICallback IMediaParamsSetUICallback;
#endif 	/* __IMediaParamsSetUICallback_FWD_DEFINED__ */


#ifndef __IMediaParamsCapture_FWD_DEFINED__
#define __IMediaParamsCapture_FWD_DEFINED__
typedef interface IMediaParamsCapture IMediaParamsCapture;
#endif 	/* __IMediaParamsCapture_FWD_DEFINED__ */


void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IMediaParamsUICallback_INTERFACE_DEFINED__
#define __IMediaParamsUICallback_INTERFACE_DEFINED__

/* interface IMediaParamsUICallback */
/* [version][uuid][local][object] */ 


EXTERN_C const IID IID_IMediaParamsUICallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B8E0480A-E08D-4a5d-9228-248017032368")
    IMediaParamsUICallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ParamsBeginCapture( 
            /* [in] */ DWORD __RPC_FAR *aIndex,
            /* [in] */ DWORD cPoints) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ParamsChanged( 
            /* [in] */ DWORD __RPC_FAR *aIndex,
            /* [in] */ DWORD cPoints,
            /* [in] */ MP_DATA __RPC_FAR *paData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ParamsEndCapture( 
            /* [in] */ DWORD __RPC_FAR *aIndex,
            /* [in] */ DWORD cPoints) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMediaParamsUICallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMediaParamsUICallback __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMediaParamsUICallback __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMediaParamsUICallback __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ParamsBeginCapture )( 
            IMediaParamsUICallback __RPC_FAR * This,
            /* [in] */ DWORD __RPC_FAR *aIndex,
            /* [in] */ DWORD cPoints);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ParamsChanged )( 
            IMediaParamsUICallback __RPC_FAR * This,
            /* [in] */ DWORD __RPC_FAR *aIndex,
            /* [in] */ DWORD cPoints,
            /* [in] */ MP_DATA __RPC_FAR *paData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ParamsEndCapture )( 
            IMediaParamsUICallback __RPC_FAR * This,
            /* [in] */ DWORD __RPC_FAR *aIndex,
            /* [in] */ DWORD cPoints);
        
        END_INTERFACE
    } IMediaParamsUICallbackVtbl;

    interface IMediaParamsUICallback
    {
        CONST_VTBL struct IMediaParamsUICallbackVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMediaParamsUICallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMediaParamsUICallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMediaParamsUICallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMediaParamsUICallback_ParamsBeginCapture(This,aIndex,cPoints)	\
    (This)->lpVtbl -> ParamsBeginCapture(This,aIndex,cPoints)

#define IMediaParamsUICallback_ParamsChanged(This,aIndex,cPoints,paData)	\
    (This)->lpVtbl -> ParamsChanged(This,aIndex,cPoints,paData)

#define IMediaParamsUICallback_ParamsEndCapture(This,aIndex,cPoints)	\
    (This)->lpVtbl -> ParamsEndCapture(This,aIndex,cPoints)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IMediaParamsUICallback_ParamsBeginCapture_Proxy( 
    IMediaParamsUICallback __RPC_FAR * This,
    /* [in] */ DWORD __RPC_FAR *aIndex,
    /* [in] */ DWORD cPoints);


void __RPC_STUB IMediaParamsUICallback_ParamsBeginCapture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaParamsUICallback_ParamsChanged_Proxy( 
    IMediaParamsUICallback __RPC_FAR * This,
    /* [in] */ DWORD __RPC_FAR *aIndex,
    /* [in] */ DWORD cPoints,
    /* [in] */ MP_DATA __RPC_FAR *paData);


void __RPC_STUB IMediaParamsUICallback_ParamsChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaParamsUICallback_ParamsEndCapture_Proxy( 
    IMediaParamsUICallback __RPC_FAR * This,
    /* [in] */ DWORD __RPC_FAR *aIndex,
    /* [in] */ DWORD cPoints);


void __RPC_STUB IMediaParamsUICallback_ParamsEndCapture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMediaParamsUICallback_INTERFACE_DEFINED__ */


#ifndef __IMediaParamsSetUICallback_INTERFACE_DEFINED__
#define __IMediaParamsSetUICallback_INTERFACE_DEFINED__

/* interface IMediaParamsSetUICallback */
/* [version][uuid][local][object] */ 


EXTERN_C const IID IID_IMediaParamsSetUICallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F5011136-C416-48b9-8C35-E7C5F9AA6FDF")
    IMediaParamsSetUICallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetUICallback( 
            /* [in] */ IMediaParamsUICallback __RPC_FAR *pICallback) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMediaParamsSetUICallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMediaParamsSetUICallback __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMediaParamsSetUICallback __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMediaParamsSetUICallback __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetUICallback )( 
            IMediaParamsSetUICallback __RPC_FAR * This,
            /* [in] */ IMediaParamsUICallback __RPC_FAR *pICallback);
        
        END_INTERFACE
    } IMediaParamsSetUICallbackVtbl;

    interface IMediaParamsSetUICallback
    {
        CONST_VTBL struct IMediaParamsSetUICallbackVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMediaParamsSetUICallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMediaParamsSetUICallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMediaParamsSetUICallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMediaParamsSetUICallback_SetUICallback(This,pICallback)	\
    (This)->lpVtbl -> SetUICallback(This,pICallback)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IMediaParamsSetUICallback_SetUICallback_Proxy( 
    IMediaParamsSetUICallback __RPC_FAR * This,
    /* [in] */ IMediaParamsUICallback __RPC_FAR *pICallback);


void __RPC_STUB IMediaParamsSetUICallback_SetUICallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMediaParamsSetUICallback_INTERFACE_DEFINED__ */


#ifndef __IMediaParamsCapture_INTERFACE_DEFINED__
#define __IMediaParamsCapture_INTERFACE_DEFINED__

/* interface IMediaParamsCapture */
/* [version][uuid][local][object] */ 


EXTERN_C const IID IID_IMediaParamsCapture;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("970FED79-6DEB-4ec4-A6EE-F72C6BA545CC")
    IMediaParamsCapture : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ParamCapture( 
            /* [in] */ DWORD dwIndex,
            /* [in] */ REFERENCE_TIME refTimeCapture,
            /* [in] */ MP_FLAGS flags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ParamRelease( 
            /* [in] */ DWORD dwIndex,
            /* [in] */ REFERENCE_TIME refTimeRelease,
            /* [in] */ MP_FLAGS flags) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMediaParamsCaptureVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMediaParamsCapture __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMediaParamsCapture __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMediaParamsCapture __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ParamCapture )( 
            IMediaParamsCapture __RPC_FAR * This,
            /* [in] */ DWORD dwIndex,
            /* [in] */ REFERENCE_TIME refTimeCapture,
            /* [in] */ MP_FLAGS flags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ParamRelease )( 
            IMediaParamsCapture __RPC_FAR * This,
            /* [in] */ DWORD dwIndex,
            /* [in] */ REFERENCE_TIME refTimeRelease,
            /* [in] */ MP_FLAGS flags);
        
        END_INTERFACE
    } IMediaParamsCaptureVtbl;

    interface IMediaParamsCapture
    {
        CONST_VTBL struct IMediaParamsCaptureVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMediaParamsCapture_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMediaParamsCapture_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMediaParamsCapture_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMediaParamsCapture_ParamCapture(This,dwIndex,refTimeCapture,flags)	\
    (This)->lpVtbl -> ParamCapture(This,dwIndex,refTimeCapture,flags)

#define IMediaParamsCapture_ParamRelease(This,dwIndex,refTimeRelease,flags)	\
    (This)->lpVtbl -> ParamRelease(This,dwIndex,refTimeRelease,flags)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IMediaParamsCapture_ParamCapture_Proxy( 
    IMediaParamsCapture __RPC_FAR * This,
    /* [in] */ DWORD dwIndex,
    /* [in] */ REFERENCE_TIME refTimeCapture,
    /* [in] */ MP_FLAGS flags);


void __RPC_STUB IMediaParamsCapture_ParamCapture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaParamsCapture_ParamRelease_Proxy( 
    IMediaParamsCapture __RPC_FAR * This,
    /* [in] */ DWORD dwIndex,
    /* [in] */ REFERENCE_TIME refTimeRelease,
    /* [in] */ MP_FLAGS flags);


void __RPC_STUB IMediaParamsCapture_ParamRelease_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMediaParamsCapture_INTERFACE_DEFINED__ */



#ifndef __CakeMedParam_LIBRARY_DEFINED__
#define __CakeMedParam_LIBRARY_DEFINED__

/* library CakeMedParam */
/* [helpstring][version][uuid] */ 





EXTERN_C const IID LIBID_CakeMedParam;
#endif /* __CakeMedParam_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
