// *************************************************************************
//  rapitypes.h
//
//  Copyright 2002 Microsoft Corporation, All Rights Reserved
//
//  Typedefs common to public and private RAPI idl.
//
// ***************************************************************************

#pragma once

#ifdef MIDL_ONLY

typedef BYTE far * LPBYTE;
#define STDAPICALLTYPE __stdcall
#endif // MIDL_ONLY

// 
// The Windows CE WIN32_FIND_DATA structure differs from the
// Windows WIN32_FIND_DATA stucture so we copy the Windows CE
// definition to here so that both sides match.
//
#define MAX_PATH 260

typedef struct CE_FIND_DATA
{
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
    DWORD    dwOID;
    WCHAR    cFileName[MAX_PATH];
} CE_FIND_DATA;

typedef CE_FIND_DATA*  LPCE_FIND_DATA;

typedef CE_FIND_DATA** LPLPCE_FIND_DATA;

//
// These are flags for CeFindAllFiles
//
#define FAF_ATTRIBUTES      ((DWORD) 0x01)
#define FAF_CREATION_TIME   ((DWORD) 0x02)
#define FAF_LASTACCESS_TIME ((DWORD) 0x04)
#define FAF_LASTWRITE_TIME  ((DWORD) 0x08)
#define FAF_SIZE_HIGH       ((DWORD) 0x10)
#define FAF_SIZE_LOW        ((DWORD) 0x20)
#define FAF_OID             ((DWORD) 0x40)
#define FAF_NAME            ((DWORD) 0x80)
#define FAF_FLAG_COUNT      ((UINT)  8)
#define FAF_ATTRIB_CHILDREN ((DWORD)            0x01000)
#define FAF_ATTRIB_NO_HIDDEN ((DWORD)            0x02000)
#define FAF_FOLDERS_ONLY    ((DWORD)            0x04000)
#define FAF_NO_HIDDEN_SYS_ROMMODULES ((DWORD)    0x08000)
#define FAF_GETTARGET        ((DWORD)            0x10000)

#define FAD_OID             ((WORD) 0x01)
#define FAD_FLAGS           ((WORD) 0x02)
#define FAD_NAME            ((WORD) 0x04)
#define FAD_TYPE            ((WORD) 0x08)
#define FAD_NUM_RECORDS     ((WORD) 0x10)
#define FAD_NUM_SORT_ORDER  ((WORD) 0x20)
#define FAD_SIZE            ((WORD) 0x40)
#define FAD_LAST_MODIFIED   ((WORD) 0x80)
#define FAD_SORT_SPECS      ((WORD) 0x100)
#define FAD_FLAG_COUNT      ((UINT) 9)

#ifndef FILE_ATTRIBUTE_INROM
#define FILE_ATTRIBUTE_INROM        0x00000040
#endif
#ifndef FILE_ATTRIBUTE_ROMSTATICREF
#define FILE_ATTRIBUTE_ROMSTATICREF 0x00001000
#endif
#ifndef FILE_ATTRIBUTE_ROMMODULE
#define FILE_ATTRIBUTE_ROMMODULE    0x00002000
#endif

//
// The following is not a standard Windows CE File Attribute.
//
#ifndef FILE_ATTRIBUTE_HAS_CHILDREN
#define FILE_ATTRIBUTE_HAS_CHILDREN 0x00010000
#endif
#ifndef FILE_ATTRIBUTE_SHORTCUT
#define FILE_ATTRIBUTE_SHORTCUT        0x00020000
#endif

typedef enum RAPISTREAMFLAG
{
    STREAM_TIMEOUT_READ
} RAPISTREAMFLAG;

// forward define
#ifdef MIDL_ONLY
interface IRAPIStream;
#else
typedef struct IRAPIStream IRAPIStream;
#endif

// RAPI extension on Windows CE (e.g., MyFunctionFOO) called via CeRapiInvoke should be declared as:
// EXTERN_C RAPIEXT MyFunctionFOO;
typedef  HRESULT (STDAPICALLTYPE RAPIEXT)(
         DWORD             cbInput,     // [IN]
         BYTE        *     pInput,      // [IN]
         DWORD       *     pcbOutput,   // [OUT]
         BYTE        **    ppOutput,    // [OUT]
         IRAPIStream *     pIRAPIStream // [IN]
         );                             

//
// The following definitions are for the client side only,
// because they are already defined on Windows CE.
//
#ifndef UNDER_CE

#include <stddef.h>

typedef struct STORE_INFORMATION
{
    DWORD dwStoreSize;
    DWORD dwFreeSize;
} STORE_INFORMATION;


typedef STORE_INFORMATION * LPSTORE_INFORMATION;

typedef DWORD CEPROPID;

typedef CEPROPID * PCEPROPID;

#define TypeFromPropID(propid) LOWORD(propid)

typedef DWORD CEOID;

typedef CEOID *PCEOID;

typedef struct CEGUID
{
    DWORD Data1;
    DWORD Data2;
    DWORD Data3;
    DWORD Data4;
} CEGUID;

typedef CEGUID * PCEGUID;

typedef struct CENOTIFICATION {
    DWORD dwSize;
    DWORD dwParam;
    UINT  uType;
    CEGUID guid;
    CEOID  oid;
    CEOID  oidParent;
} CENOTIFICATION;

#define CEDB_EXNOTIFICATION    0x00000001

typedef struct CENOTIFYREQUEST {
    DWORD dwSize;
    HWND  hwnd;
    DWORD dwFlags;
    HANDLE hHeap;
    DWORD  dwParam;
} CENOTIFYREQUEST;

typedef CENOTIFYREQUEST * PCENOTIFYREQUEST;

typedef struct CEFILEINFO
{
    DWORD   dwAttributes;
    CEOID   oidParent;
    WCHAR   szFileName[MAX_PATH];
    FILETIME ftLastChanged;
    DWORD   dwLength;
} CEFILEINFO;

typedef struct CEDIRINFO {
    DWORD   dwAttributes;
    CEOID   oidParent;
    WCHAR   szDirName[MAX_PATH];
} CEDIRINFO;

typedef struct CERECORDINFO {
    CEOID  oidParent;
} CERECORDINFO;

#define CEDB_SORT_DESCENDING        0x00000001
#define CEDB_SORT_CASEINSENSITIVE   0x00000002
#define CEDB_SORT_UNKNOWNFIRST      0x00000004
#define CEDB_SORT_GENERICORDER      0x00000008

typedef struct SORTORDERSPEC {
    CEPROPID  propid;
    DWORD     dwFlags;
} SORTORDERSPEC;

#define CEDB_MAXDBASENAMELEN  32
#define CEDB_MAXDBASENAMELEN  32
#define CEDB_MAXSORTORDER     4
#define CEDB_MAXSORTORDER     4

#define CEDB_VALIDNAME      0x0001
#define CEDB_VALIDTYPE      0x0002
#define CEDB_VALIDSORTSPEC  0x0004
#define CEDB_VALIDMODTIME   0x0008
#define CEDB_VALIDDBFLAGS   0x0010
#define CEDB_VALIDCREATE (CEDB_VALIDNAME|CEDB_VALIDTYPE|CEDB_VALIDSORTSPEC|CEDB_VALIDDBFLAGS)

#define CEDB_NOCOMPRESS     0x00010000

typedef struct CEDBASEINFO 
{
    DWORD   dwFlags;
    WCHAR   szDbaseName[CEDB_MAXDBASENAMELEN];
    DWORD   dwDbaseType;
    WORD    wNumRecords;
    WORD    wNumSortOrder;
    DWORD   dwSize;
    FILETIME ftLastModified;
    SORTORDERSPEC rgSortSpecs[CEDB_MAXSORTORDER];
} CEDBASEINFO;

typedef struct CEDB_FIND_DATA {
    CEOID       OidDb;
    CEDBASEINFO DbInfo;
} CEDB_FIND_DATA;


typedef CEDB_FIND_DATA * LPCEDB_FIND_DATA;

typedef CEDB_FIND_DATA ** LPLPCEDB_FIND_DATA;

#define OBJTYPE_INVALID     0
#define OBJTYPE_FILE        1
#define OBJTYPE_DIRECTORY   2
#define OBJTYPE_DATABASE    3
#define OBJTYPE_RECORD      4

typedef struct CEOIDINFO
{
    WORD  wObjType;
    WORD  wPad;
    union 
    {
        CEFILEINFO infFile;
        CEDIRINFO    infDirectory;
        CEDBASEINFO  infDatabase;
        CERECORDINFO infRecord;    
    };
} CEOIDINFO;

#define CEDB_AUTOINCREMENT      0x00000001

#define CEDB_SEEK_CEOID         0x00000001
#define CEDB_SEEK_BEGINNING     0x00000002
#define CEDB_SEEK_END           0x00000004
#define CEDB_SEEK_CURRENT       0x00000008
#define CEDB_SEEK_VALUESMALLER     0x00000010
#define CEDB_SEEK_VALUEFIRSTEQUAL  0x00000020
#define CEDB_SEEK_VALUEGREATER     0x00000040
#define CEDB_SEEK_VALUENEXTEQUAL   0x00000080

typedef struct CEBLOB{
    DWORD       dwCount;
    LPBYTE      lpb;
} CEBLOB;

#define CEVT_I2       2
#define CEVT_UI2      18
#define CEVT_I4       3
#define CEVT_UI4      19
#define CEVT_FILETIME 64
#define CEVT_LPWSTR   31
#define CEVT_BLOB     65
#define CEVT_BOOL     11
#define    CEVT_R8       5

typedef union CEVALUNION {
    short           iVal;
    USHORT          uiVal;
    long            lVal;
    ULONG           ulVal;
    FILETIME        filetime;
    LPWSTR          lpwstr;
    CEBLOB          blob;
    BOOL            boolVal;
    double          dblVal;
} CEVALUNION;
 
#define CEDB_PROPNOTFOUND 0x0100
#define CEDB_PROPDELETE   0x0200

typedef struct CEPROPVAL {
    CEPROPID    propid;
    WORD        wLenData;
    WORD        wFlags;
    CEVALUNION  val;
} CEPROPVAL;

typedef CEPROPVAL * PCEPROPVAL;


#define CEDB_MAXDATABLOCKSIZE  4092
#define CEDB_MAXPROPDATASIZE   (CEDB_MAXDATABLOCKSIZE*16)
#define CEDB_MAXRECORDSIZE     (128*1024)

#define CEDB_ALLOWREALLOC  0x00000001

#define CREATE_SYSTEMGUID(pguid) (memset((pguid), 0, sizeof(CEGUID)))
#define CREATE_INVALIDGUID(pguid) (memset((pguid), -1, sizeof(CEGUID)))

#define CHECK_SYSTEMGUID(pguid) !((pguid)->Data1|(pguid)->Data2|(pguid)->Data3|(pguid)->Data4)
#define CHECK_INVALIDGUID(pguid) !~((pguid)->Data1&(pguid)->Data2&(pguid)->Data3&(pguid)->Data4)

#define SYSMEM_CHANGED    0
#define SYSMEM_MUSTREBOOT 1
#define SYSMEM_REBOOTPENDING 2
#define SYSMEM_FAILED 3

typedef struct CEOSVERSIONINFO {
    DWORD dwOSVersionInfoSize; 
    DWORD dwMajorVersion; 
    DWORD dwMinorVersion; 
    DWORD dwBuildNumber; 
    DWORD dwPlatformId; 
    WCHAR szCSDVersion[ 128 ]; 
} CEOSVERSIONINFO;

typedef CEOSVERSIONINFO * LPCEOSVERSIONINFO;

#define AC_LINE_OFFLINE                 0x00
#define AC_LINE_ONLINE                  0x01
#define AC_LINE_BACKUP_POWER            0x02
#define AC_LINE_UNKNOWN                 0xFF

#define BATTERY_FLAG_HIGH               0x01
#define BATTERY_FLAG_LOW                0x02
#define BATTERY_FLAG_CRITICAL           0x04
#define BATTERY_FLAG_CHARGING           0x08
#define BATTERY_FLAG_NO_BATTERY         0x80
#define BATTERY_FLAG_UNKNOWN            0xFF

#define BATTERY_PERCENTAGE_UNKNOWN      0xFF

#define BATTERY_LIFE_UNKNOWN        0xFFFFFFFF

typedef struct SYSTEM_POWER_STATUS_EX{
    BYTE ACLineStatus;
    BYTE BatteryFlag;
    BYTE BatteryLifePercent;
    BYTE Reserved1;
    DWORD BatteryLifeTime;
    DWORD BatteryFullLifeTime;
    BYTE Reserved2;
    BYTE BackupBatteryFlag;
    BYTE BackupBatteryLifePercent;
    BYTE Reserved3;
    DWORD BackupBatteryLifeTime;
    DWORD BackupBatteryFullLifeTime;
} SYSTEM_POWER_STATUS_EX;

typedef SYSTEM_POWER_STATUS_EX * PSYSTEM_POWER_STATUS_EX;

typedef SYSTEM_POWER_STATUS_EX * LPSYSTEM_POWER_STATUS_EX;


//
// MessageId: CERAPI_E_ALREADYINITIALIZED
//
//  CeRapiInit(Ex) has already been successfully called
//
#define CERAPI_E_ALREADYINITIALIZED     0x80041001

typedef struct RAPIINIT
{
    DWORD cbSize;
    HANDLE heRapiInit;
    HRESULT hrRapiInit;
} RAPIINIT;

//
// Instruction set definitions for CeQueryInstructionSet() 
//
// PROCESSOR_ARCHITECTURE_x are already defined in desktop winnt.h
// Here we include the CE specific definitions from CE winnt.h
//
#define PROCESSOR_INSTRUCTION_CODE(arch, core, feature)      \
    ((arch) << 24 | (core) << 16 | (feature))
#define PROCESSOR_X86_32BIT_CORE                1

#define PROCESSOR_MIPS16_CORE                   1
#define PROCESSOR_MIPSII_CORE                   2
#define PROCESSOR_MIPSIV_CORE                   3

#define PROCESSOR_HITACHI_SH3_CORE              1
#define PROCESSOR_HITACHI_SH4_CORE              2

#define PROCESSOR_ARM_V4_CORE                   1
#define PROCESSOR_ARM_V4I_CORE                  2
#define PROCESSOR_ARM_V4T_CORE                  3

#define PROCESSOR_FEATURE_NOFP                  0
#define PROCESSOR_FEATURE_FP                    1
#define PROCESSOR_FEATURE_DSP                   PROCESSOR_FEATURE_FP

#define PROCESSOR_QUERY_INSTRUCTION             PROCESSOR_INSTRUCTION_CODE(0,0,0)

#define PROCESSOR_X86_32BIT_INSTRUCTION         \
    PROCESSOR_INSTRUCTION_CODE(PROCESSOR_ARCHITECTURE_INTEL, PROCESSOR_X86_32BIT_CORE, PROCESSOR_FEATURE_FP)

#define PROCESSOR_MIPS_MIPS16_INSTRUCTION       \
    PROCESSOR_INSTRUCTION_CODE(PROCESSOR_ARCHITECTURE_MIPS,  PROCESSOR_MIPS16_CORE, PROCESSOR_FEATURE_NOFP)
#define PROCESSOR_MIPS_MIPSII_INSTRUCTION       \
    PROCESSOR_INSTRUCTION_CODE(PROCESSOR_ARCHITECTURE_MIPS,  PROCESSOR_MIPSII_CORE, PROCESSOR_FEATURE_NOFP)
#define PROCESSOR_MIPS_MIPSIIFP_INSTRUCTION     \
    PROCESSOR_INSTRUCTION_CODE(PROCESSOR_ARCHITECTURE_MIPS,  PROCESSOR_MIPSII_CORE, PROCESSOR_FEATURE_FP)
#define PROCESSOR_MIPS_MIPSIV_INSTRUCTION       \
    PROCESSOR_INSTRUCTION_CODE(PROCESSOR_ARCHITECTURE_MIPS,  PROCESSOR_MIPSIV_CORE, PROCESSOR_FEATURE_NOFP)
#define PROCESSOR_MIPS_MIPSIVFP_INSTRUCTION     \
    PROCESSOR_INSTRUCTION_CODE(PROCESSOR_ARCHITECTURE_MIPS,  PROCESSOR_MIPSIV_CORE, PROCESSOR_FEATURE_FP)

#define PROCESSOR_HITACHI_SH3_INSTRUCTION       \
    PROCESSOR_INSTRUCTION_CODE(PROCESSOR_ARCHITECTURE_SHX,   PROCESSOR_HITACHI_SH3_CORE, PROCESSOR_FEATURE_NOFP)
#define PROCESSOR_HITACHI_SH3DSP_INSTRUCTION    \
    PROCESSOR_INSTRUCTION_CODE(PROCESSOR_ARCHITECTURE_SHX,   PROCESSOR_HITACHI_SH3_CORE, PROCESSOR_FEATURE_DSP)
#define PROCESSOR_HITACHI_SH4_INSTRUCTION       \
    PROCESSOR_INSTRUCTION_CODE(PROCESSOR_ARCHITECTURE_SHX,   PROCESSOR_HITACHI_SH4_CORE, PROCESSOR_FEATURE_FP)

#define PROCESSOR_ARM_V4_INSTRUCTION            \
    PROCESSOR_INSTRUCTION_CODE(PROCESSOR_ARCHITECTURE_ARM,   PROCESSOR_ARM_V4_CORE, PROCESSOR_FEATURE_NOFP)
#define PROCESSOR_ARM_V4FP_INSTRUCTION          \
    PROCESSOR_INSTRUCTION_CODE(PROCESSOR_ARCHITECTURE_ARM,   PROCESSOR_ARM_V4_CORE, PROCESSOR_FEATURE_FP)
#define PROCESSOR_ARM_V4I_INSTRUCTION           \
    PROCESSOR_INSTRUCTION_CODE(PROCESSOR_ARCHITECTURE_ARM,   PROCESSOR_ARM_V4I_CORE, PROCESSOR_FEATURE_NOFP)
#define PROCESSOR_ARM_V4IFP_INSTRUCTION         \
    PROCESSOR_INSTRUCTION_CODE(PROCESSOR_ARCHITECTURE_ARM,   PROCESSOR_ARM_V4I_CORE, PROCESSOR_FEATURE_FP)
#define PROCESSOR_ARM_V4T_INSTRUCTION           \
    PROCESSOR_INSTRUCTION_CODE(PROCESSOR_ARCHITECTURE_ARM,   PROCESSOR_ARM_V4T_CORE, PROCESSOR_FEATURE_NOFP)
#define PROCESSOR_ARM_V4TFP_INSTRUCTION         \
    PROCESSOR_INSTRUCTION_CODE(PROCESSOR_ARCHITECTURE_ARM,   PROCESSOR_ARM_V4T_CORE, PROCESSOR_FEATURE_FP)


#endif // !UNDER_CE
