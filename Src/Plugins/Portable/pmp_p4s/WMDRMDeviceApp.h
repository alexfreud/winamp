

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Tue Jan 10 22:44:12 2006
 */
/* Compiler settings for \Wmsdk\Wmfsdk95\Wmdm\idl\WMDRMDeviceApp.idl:
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

#ifndef __WMDRMDeviceApp_h__
#define __WMDRMDeviceApp_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IWMDRMDeviceApp_FWD_DEFINED__
#define __IWMDRMDeviceApp_FWD_DEFINED__
typedef interface IWMDRMDeviceApp IWMDRMDeviceApp;
#endif 	/* __IWMDRMDeviceApp_FWD_DEFINED__ */


#ifndef __IWMDRMDeviceApp2_FWD_DEFINED__
#define __IWMDRMDeviceApp2_FWD_DEFINED__
typedef interface IWMDRMDeviceApp2 IWMDRMDeviceApp2;
#endif 	/* __IWMDRMDeviceApp2_FWD_DEFINED__ */


#ifndef __WMDRMDeviceApp_FWD_DEFINED__
#define __WMDRMDeviceApp_FWD_DEFINED__

#ifdef __cplusplus
typedef class WMDRMDeviceApp WMDRMDeviceApp;
#else
typedef struct WMDRMDeviceApp WMDRMDeviceApp;
#endif /* __cplusplus */

#endif 	/* __WMDRMDeviceApp_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "mswmdm.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

/* interface __MIDL_itf_WMDRMDeviceApp_0000 */
/* [local] */ 

// WMDRM Device status flags
#define WMDRM_DEVICE_ISWMDRM                 0x00000001L
#define WMDRM_DEVICE_NEEDCLOCK               0x00000002L
#define WMDRM_DEVICE_REVOKED                 0x00000004L
#define WMDRM_CLIENT_NEEDINDIV               0x00000008L
#define WMDRM_DEVICE_REFRESHCLOCK            0x00000010L
// WMDRM Query Device flags
#define WMDRM_QUERY_DEVICE_ISWMDRM           0x00000001L
#define WMDRM_QUERY_DEVICE_CLOCKSTATUS       0x00000002L
#define WMDRM_QUERY_DEVICE_ISREVOKED         0x00000004L
#define WMDRM_QUERY_CLIENT_INDIVSTATUS       0x00000008L
// ProcessMeterResponse flags
#define WMDRM_METER_RESPONSE_ALL             0x00000000L
#define WMDRM_METER_RESPONSE_PARTIAL         0x00000001L


extern RPC_IF_HANDLE __MIDL_itf_WMDRMDeviceApp_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_WMDRMDeviceApp_0000_v0_0_s_ifspec;

#ifndef __IWMDRMDeviceApp_INTERFACE_DEFINED__
#define __IWMDRMDeviceApp_INTERFACE_DEFINED__

/* interface IWMDRMDeviceApp */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IWMDRMDeviceApp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("93AFDB44-B1E1-411d-B89B-75AD4F97882B")
    IWMDRMDeviceApp : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GenerateMeterChallenge( 
            /* [in] */ IWMDMDevice *pDevice,
            /* [in] */ BSTR bstrMeterCert,
            /* [out] */ BSTR *pbstrMeterURL,
            /* [out] */ BSTR *pbstrMeterData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ProcessMeterResponse( 
            /* [in] */ IWMDMDevice *pDevice,
            /* [in] */ BYTE *pbResponse,
            /* [in] */ DWORD cbResponse,
            /* [out] */ DWORD *pdwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryDeviceStatus( 
            /* [in] */ IWMDMDevice *pDevice,
            /* [out] */ DWORD *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AcquireDeviceData( 
            /* [in] */ IWMDMDevice *pDevice,
            /* [in] */ IWMDMProgress3 *pProgressCallback,
            /* [in] */ DWORD dwFlags,
            /* [out] */ DWORD *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SynchronizeLicenses( 
            /* [in] */ IWMDMDevice *pDevice,
            /* [in] */ IWMDMProgress3 *pProgressCallback,
            /* [in] */ DWORD cMinCountThreshold,
            /* [in] */ DWORD cMinHoursThreshold) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWMDRMDeviceAppVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDRMDeviceApp * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDRMDeviceApp * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDRMDeviceApp * This);
        
        HRESULT ( STDMETHODCALLTYPE *GenerateMeterChallenge )( 
            IWMDRMDeviceApp * This,
            /* [in] */ IWMDMDevice *pDevice,
            /* [in] */ BSTR bstrMeterCert,
            /* [out] */ BSTR *pbstrMeterURL,
            /* [out] */ BSTR *pbstrMeterData);
        
        HRESULT ( STDMETHODCALLTYPE *ProcessMeterResponse )( 
            IWMDRMDeviceApp * This,
            /* [in] */ IWMDMDevice *pDevice,
            /* [in] */ BYTE *pbResponse,
            /* [in] */ DWORD cbResponse,
            /* [out] */ DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *QueryDeviceStatus )( 
            IWMDRMDeviceApp * This,
            /* [in] */ IWMDMDevice *pDevice,
            /* [out] */ DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *AcquireDeviceData )( 
            IWMDRMDeviceApp * This,
            /* [in] */ IWMDMDevice *pDevice,
            /* [in] */ IWMDMProgress3 *pProgressCallback,
            /* [in] */ DWORD dwFlags,
            /* [out] */ DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *SynchronizeLicenses )( 
            IWMDRMDeviceApp * This,
            /* [in] */ IWMDMDevice *pDevice,
            /* [in] */ IWMDMProgress3 *pProgressCallback,
            /* [in] */ DWORD cMinCountThreshold,
            /* [in] */ DWORD cMinHoursThreshold);
        
        END_INTERFACE
    } IWMDRMDeviceAppVtbl;

    interface IWMDRMDeviceApp
    {
        CONST_VTBL struct IWMDRMDeviceAppVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDRMDeviceApp_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDRMDeviceApp_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDRMDeviceApp_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDRMDeviceApp_GenerateMeterChallenge(This,pDevice,bstrMeterCert,pbstrMeterURL,pbstrMeterData)	\
    (This)->lpVtbl -> GenerateMeterChallenge(This,pDevice,bstrMeterCert,pbstrMeterURL,pbstrMeterData)

#define IWMDRMDeviceApp_ProcessMeterResponse(This,pDevice,pbResponse,cbResponse,pdwFlags)	\
    (This)->lpVtbl -> ProcessMeterResponse(This,pDevice,pbResponse,cbResponse,pdwFlags)

#define IWMDRMDeviceApp_QueryDeviceStatus(This,pDevice,pdwStatus)	\
    (This)->lpVtbl -> QueryDeviceStatus(This,pDevice,pdwStatus)

#define IWMDRMDeviceApp_AcquireDeviceData(This,pDevice,pProgressCallback,dwFlags,pdwStatus)	\
    (This)->lpVtbl -> AcquireDeviceData(This,pDevice,pProgressCallback,dwFlags,pdwStatus)

#define IWMDRMDeviceApp_SynchronizeLicenses(This,pDevice,pProgressCallback,cMinCountThreshold,cMinHoursThreshold)	\
    (This)->lpVtbl -> SynchronizeLicenses(This,pDevice,pProgressCallback,cMinCountThreshold,cMinHoursThreshold)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IWMDRMDeviceApp_GenerateMeterChallenge_Proxy( 
    IWMDRMDeviceApp * This,
    /* [in] */ IWMDMDevice *pDevice,
    /* [in] */ BSTR bstrMeterCert,
    /* [out] */ BSTR *pbstrMeterURL,
    /* [out] */ BSTR *pbstrMeterData);


void __RPC_STUB IWMDRMDeviceApp_GenerateMeterChallenge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDRMDeviceApp_ProcessMeterResponse_Proxy( 
    IWMDRMDeviceApp * This,
    /* [in] */ IWMDMDevice *pDevice,
    /* [in] */ BYTE *pbResponse,
    /* [in] */ DWORD cbResponse,
    /* [out] */ DWORD *pdwFlags);


void __RPC_STUB IWMDRMDeviceApp_ProcessMeterResponse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDRMDeviceApp_QueryDeviceStatus_Proxy( 
    IWMDRMDeviceApp * This,
    /* [in] */ IWMDMDevice *pDevice,
    /* [out] */ DWORD *pdwStatus);


void __RPC_STUB IWMDRMDeviceApp_QueryDeviceStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDRMDeviceApp_AcquireDeviceData_Proxy( 
    IWMDRMDeviceApp * This,
    /* [in] */ IWMDMDevice *pDevice,
    /* [in] */ IWMDMProgress3 *pProgressCallback,
    /* [in] */ DWORD dwFlags,
    /* [out] */ DWORD *pdwStatus);


void __RPC_STUB IWMDRMDeviceApp_AcquireDeviceData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDRMDeviceApp_SynchronizeLicenses_Proxy( 
    IWMDRMDeviceApp * This,
    /* [in] */ IWMDMDevice *pDevice,
    /* [in] */ IWMDMProgress3 *pProgressCallback,
    /* [in] */ DWORD cMinCountThreshold,
    /* [in] */ DWORD cMinHoursThreshold);


void __RPC_STUB IWMDRMDeviceApp_SynchronizeLicenses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWMDRMDeviceApp_INTERFACE_DEFINED__ */


#ifndef __IWMDRMDeviceApp2_INTERFACE_DEFINED__
#define __IWMDRMDeviceApp2_INTERFACE_DEFINED__

/* interface IWMDRMDeviceApp2 */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IWMDRMDeviceApp2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("600D6E55-DEA5-4e4c-9C3A-6BD642A45B9D")
    IWMDRMDeviceApp2 : public IWMDRMDeviceApp
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryDeviceStatus2( 
            /* [in] */ IWMDMDevice *pDevice,
            /* [in] */ DWORD dwFlags,
            /* [out] */ DWORD *pdwStatus) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWMDRMDeviceApp2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDRMDeviceApp2 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDRMDeviceApp2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDRMDeviceApp2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GenerateMeterChallenge )( 
            IWMDRMDeviceApp2 * This,
            /* [in] */ IWMDMDevice *pDevice,
            /* [in] */ BSTR bstrMeterCert,
            /* [out] */ BSTR *pbstrMeterURL,
            /* [out] */ BSTR *pbstrMeterData);
        
        HRESULT ( STDMETHODCALLTYPE *ProcessMeterResponse )( 
            IWMDRMDeviceApp2 * This,
            /* [in] */ IWMDMDevice *pDevice,
            /* [in] */ BYTE *pbResponse,
            /* [in] */ DWORD cbResponse,
            /* [out] */ DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *QueryDeviceStatus )( 
            IWMDRMDeviceApp2 * This,
            /* [in] */ IWMDMDevice *pDevice,
            /* [out] */ DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *AcquireDeviceData )( 
            IWMDRMDeviceApp2 * This,
            /* [in] */ IWMDMDevice *pDevice,
            /* [in] */ IWMDMProgress3 *pProgressCallback,
            /* [in] */ DWORD dwFlags,
            /* [out] */ DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *SynchronizeLicenses )( 
            IWMDRMDeviceApp2 * This,
            /* [in] */ IWMDMDevice *pDevice,
            /* [in] */ IWMDMProgress3 *pProgressCallback,
            /* [in] */ DWORD cMinCountThreshold,
            /* [in] */ DWORD cMinHoursThreshold);
        
        HRESULT ( STDMETHODCALLTYPE *QueryDeviceStatus2 )( 
            IWMDRMDeviceApp2 * This,
            /* [in] */ IWMDMDevice *pDevice,
            /* [in] */ DWORD dwFlags,
            /* [out] */ DWORD *pdwStatus);
        
        END_INTERFACE
    } IWMDRMDeviceApp2Vtbl;

    interface IWMDRMDeviceApp2
    {
        CONST_VTBL struct IWMDRMDeviceApp2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDRMDeviceApp2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDRMDeviceApp2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDRMDeviceApp2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDRMDeviceApp2_GenerateMeterChallenge(This,pDevice,bstrMeterCert,pbstrMeterURL,pbstrMeterData)	\
    (This)->lpVtbl -> GenerateMeterChallenge(This,pDevice,bstrMeterCert,pbstrMeterURL,pbstrMeterData)

#define IWMDRMDeviceApp2_ProcessMeterResponse(This,pDevice,pbResponse,cbResponse,pdwFlags)	\
    (This)->lpVtbl -> ProcessMeterResponse(This,pDevice,pbResponse,cbResponse,pdwFlags)

#define IWMDRMDeviceApp2_QueryDeviceStatus(This,pDevice,pdwStatus)	\
    (This)->lpVtbl -> QueryDeviceStatus(This,pDevice,pdwStatus)

#define IWMDRMDeviceApp2_AcquireDeviceData(This,pDevice,pProgressCallback,dwFlags,pdwStatus)	\
    (This)->lpVtbl -> AcquireDeviceData(This,pDevice,pProgressCallback,dwFlags,pdwStatus)

#define IWMDRMDeviceApp2_SynchronizeLicenses(This,pDevice,pProgressCallback,cMinCountThreshold,cMinHoursThreshold)	\
    (This)->lpVtbl -> SynchronizeLicenses(This,pDevice,pProgressCallback,cMinCountThreshold,cMinHoursThreshold)


#define IWMDRMDeviceApp2_QueryDeviceStatus2(This,pDevice,dwFlags,pdwStatus)	\
    (This)->lpVtbl -> QueryDeviceStatus2(This,pDevice,dwFlags,pdwStatus)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IWMDRMDeviceApp2_QueryDeviceStatus2_Proxy( 
    IWMDRMDeviceApp2 * This,
    /* [in] */ IWMDMDevice *pDevice,
    /* [in] */ DWORD dwFlags,
    /* [out] */ DWORD *pdwStatus);


void __RPC_STUB IWMDRMDeviceApp2_QueryDeviceStatus2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWMDRMDeviceApp2_INTERFACE_DEFINED__ */



#ifndef __WMDRMDeviceAppLib_LIBRARY_DEFINED__
#define __WMDRMDeviceAppLib_LIBRARY_DEFINED__

/* library WMDRMDeviceAppLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_WMDRMDeviceAppLib;

EXTERN_C const CLSID CLSID_WMDRMDeviceApp;

#ifdef __cplusplus

class DECLSPEC_UUID("5C140836-43DE-11d3-847D-00C04F79DBC0")
WMDRMDeviceApp;
#endif
#endif /* __WMDRMDeviceAppLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


