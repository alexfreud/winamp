

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


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

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IWMDRMDeviceApp,0x93AFDB44,0xB1E1,0x411d,0xB8,0x9B,0x75,0xAD,0x4F,0x97,0x88,0x2B);


MIDL_DEFINE_GUID(IID, IID_IWMDRMDeviceApp2,0x600D6E55,0xDEA5,0x4e4c,0x9C,0x3A,0x6B,0xD6,0x42,0xA4,0x5B,0x9D);


MIDL_DEFINE_GUID(IID, LIBID_WMDRMDeviceAppLib,0x50BB7AB2,0x0498,0x450D,0xA2,0xC3,0x81,0xCC,0x17,0xFD,0x15,0x4D);


MIDL_DEFINE_GUID(CLSID, CLSID_WMDRMDeviceApp,0x5C140836,0x43DE,0x11d3,0x84,0x7D,0x00,0xC0,0x4F,0x79,0xDB,0xC0);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

