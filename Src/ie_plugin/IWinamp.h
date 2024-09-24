

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Mon Aug 24 16:18:09 2009
 */
/* Compiler settings for .\IWinamp.idl:
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

#ifndef __IWinamp_h__
#define __IWinamp_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IWinamp_FWD_DEFINED__
#define __IWinamp_FWD_DEFINED__
typedef interface IWinamp IWinamp;
#endif 	/* __IWinamp_FWD_DEFINED__ */


#ifndef __Winamp_FWD_DEFINED__
#define __Winamp_FWD_DEFINED__

#ifdef __cplusplus
typedef class Winamp Winamp;
#else
typedef struct Winamp Winamp;
#endif /* __cplusplus */

#endif 	/* __Winamp_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IWinamp_INTERFACE_DEFINED__
#define __IWinamp_INTERFACE_DEFINED__

/* interface IWinamp */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IWinamp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("007374A5-FE80-4b29-AF8C-FD9C0F16C85C")
    IWinamp : public IObjectWithSite
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IWinampVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWinamp * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWinamp * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWinamp * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetSite )( 
            IWinamp * This,
            /* [in] */ IUnknown *pUnkSite);
        
        HRESULT ( STDMETHODCALLTYPE *GetSite )( 
            IWinamp * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvSite);
        
        END_INTERFACE
    } IWinampVtbl;

    interface IWinamp
    {
        CONST_VTBL struct IWinampVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWinamp_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IWinamp_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IWinamp_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IWinamp_SetSite(This,pUnkSite)	\
    ( (This)->lpVtbl -> SetSite(This,pUnkSite) ) 

#define IWinamp_GetSite(This,riid,ppvSite)	\
    ( (This)->lpVtbl -> GetSite(This,riid,ppvSite) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IWinamp_INTERFACE_DEFINED__ */



#ifndef __ElevatorLib_LIBRARY_DEFINED__
#define __ElevatorLib_LIBRARY_DEFINED__

/* library ElevatorLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_ElevatorLib;

EXTERN_C const CLSID CLSID_Winamp;

#ifdef __cplusplus

class DECLSPEC_UUID("D9C17076-9F55-49b5-8BEB-6A857931E62C")
Winamp;
#endif
#endif /* __ElevatorLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


