/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Fri May 03 10:13:47 2002
 */
/* Compiler settings for CakeMedParam.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


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

const IID IID_IMediaParamsUICallback = {0xB8E0480A,0xE08D,0x4a5d,{0x92,0x28,0x24,0x80,0x17,0x03,0x23,0x68}};


const IID IID_IMediaParamsSetUICallback = {0xF5011136,0xC416,0x48b9,{0x8C,0x35,0xE7,0xC5,0xF9,0xAA,0x6F,0xDF}};


const IID IID_IMediaParamsCapture = {0x970FED79,0x6DEB,0x4ec4,{0xA6,0xEE,0xF7,0x2C,0x6B,0xA5,0x45,0xCC}};


const IID LIBID_CakeMedParam = {0xA8F8EF3E,0x4E39,0x49e2,{0x89,0xE7,0x3C,0x91,0x94,0x2C,0xC5,0x7B}};


#ifdef __cplusplus
}
#endif

