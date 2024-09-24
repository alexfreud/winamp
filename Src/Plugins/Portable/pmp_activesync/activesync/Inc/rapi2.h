

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Fri Apr 22 19:21:11 2005
 */
/* Compiler settings for .\RAPI2.idl:
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

#ifndef __RAPI2_h__
#define __RAPI2_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IRAPISession_FWD_DEFINED__
#define __IRAPISession_FWD_DEFINED__
typedef interface IRAPISession IRAPISession;
#endif 	/* __IRAPISession_FWD_DEFINED__ */


#ifndef __IRAPIDevice_FWD_DEFINED__
#define __IRAPIDevice_FWD_DEFINED__
typedef interface IRAPIDevice IRAPIDevice;
#endif 	/* __IRAPIDevice_FWD_DEFINED__ */


#ifndef __IRAPIEnumDevices_FWD_DEFINED__
#define __IRAPIEnumDevices_FWD_DEFINED__
typedef interface IRAPIEnumDevices IRAPIEnumDevices;
#endif 	/* __IRAPIEnumDevices_FWD_DEFINED__ */


#ifndef __IRAPISink_FWD_DEFINED__
#define __IRAPISink_FWD_DEFINED__
typedef interface IRAPISink IRAPISink;
#endif 	/* __IRAPISink_FWD_DEFINED__ */


#ifndef __IRAPIDesktop_FWD_DEFINED__
#define __IRAPIDesktop_FWD_DEFINED__
typedef interface IRAPIDesktop IRAPIDesktop;
#endif 	/* __IRAPIDesktop_FWD_DEFINED__ */


#ifndef __RAPI_FWD_DEFINED__
#define __RAPI_FWD_DEFINED__

#ifdef __cplusplus
typedef class RAPI RAPI;
#else
typedef struct RAPI RAPI;
#endif /* __cplusplus */

#endif 	/* __RAPI_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "rapitypes.h"
#include "rapitypes2.h"
#include "irapistream.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

/* interface __MIDL_itf_RAPI2_0000 */
/* [local] */ 

#define NO_APIMAP 1
#include "rapi.h"
#ifndef UNDER_CE


extern RPC_IF_HANDLE __MIDL_itf_RAPI2_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_RAPI2_0000_v0_0_s_ifspec;

#ifndef __IRAPISession_INTERFACE_DEFINED__
#define __IRAPISession_INTERFACE_DEFINED__

/* interface IRAPISession */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_IRAPISession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("76a78b7d-8e54-4c06-ac38-459e6a1ab5e3")
    IRAPISession : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CeRapiInit( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CeRapiUninit( void) = 0;
        
        virtual DWORD STDMETHODCALLTYPE CeGetLastError( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CeRapiGetError( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CeRapiFreeBuffer( 
            void *Buffer) = 0;
        
        virtual HANDLE STDMETHODCALLTYPE CeFindFirstFile( 
            LPCWSTR FileName,
            LPCE_FIND_DATA FindData) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeFindNextFile( 
            HANDLE FoundFile,
            LPCE_FIND_DATA FindData) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeFindClose( 
            HANDLE FoundFile) = 0;
        
        virtual DWORD STDMETHODCALLTYPE CeGetFileAttributes( 
            LPCWSTR FileName) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeSetFileAttributes( 
            LPCWSTR FileName,
            DWORD FileAttrib) = 0;
        
        virtual HANDLE STDMETHODCALLTYPE CeCreateFile( 
            LPCWSTR lpFileName,
            DWORD dwDesiredAccess,
            DWORD dwShareMode,
            LPSECURITY_ATTRIBUTES lpSecurityAttributes,
            DWORD dwCreationDistribution,
            DWORD dwFlagsAndAttributes,
            HANDLE hTemplateFile) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeReadFile( 
            HANDLE hFile,
            LPVOID lpBuffer,
            DWORD nNumberOfBytesToRead,
            LPDWORD lpNumberOfBytesRead,
            LPOVERLAPPED lpOverlapped) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeWriteFile( 
            HANDLE hFile,
            LPCVOID lpBuffer,
            DWORD nNumberOfBytesToWrite,
            LPDWORD lpNumberOfBytesWritten,
            LPOVERLAPPED lpOverlapped) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeCloseHandle( 
            HANDLE hObject) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeFindAllFiles( 
            LPCWSTR Path,
            DWORD Flags,
            LPDWORD pFoundCount,
            LPLPCE_FIND_DATA ppFindDataArray) = 0;
        
        virtual HANDLE STDMETHODCALLTYPE CeFindFirstDatabase( 
            DWORD dwDbaseType) = 0;
        
        virtual CEOID STDMETHODCALLTYPE CeFindNextDatabase( 
            HANDLE hEnum) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeOidGetInfo( 
            CEOID oid,
            CEOIDINFO *oidInfo) = 0;
        
        virtual CEOID STDMETHODCALLTYPE CeCreateDatabase( 
            LPWSTR lpszName,
            DWORD dwDbaseType,
            WORD cNumSortOrder,
            SORTORDERSPEC *rgSortSpecs) = 0;
        
        virtual HANDLE STDMETHODCALLTYPE CeOpenDatabase( 
            PCEOID poid,
            LPWSTR lpszName,
            CEPROPID propid,
            DWORD dwFlags,
            HWND hwndNotify) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeDeleteDatabase( 
            CEOID oidDbase) = 0;
        
        virtual CEOID STDMETHODCALLTYPE CeReadRecordProps( 
            HANDLE hDbase,
            DWORD dwFlags,
            LPWORD lpcPropID,
            CEPROPID *rgPropID,
            LPBYTE *lplpBuffer,
            LPDWORD lpcbBuffer) = 0;
        
        virtual CEOID STDMETHODCALLTYPE CeWriteRecordProps( 
            HANDLE hDbase,
            CEOID oidRecord,
            WORD cPropID,
            CEPROPVAL *rgPropVal) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeDeleteRecord( 
            HANDLE hDatabase,
            CEOID oidRecord) = 0;
        
        virtual CEOID STDMETHODCALLTYPE CeSeekDatabase( 
            HANDLE hDatabase,
            DWORD dwSeekType,
            DWORD dwValue,
            LPDWORD lpdwIndex) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeSetDatabaseInfo( 
            CEOID oidDbase,
            CEDBASEINFO *pNewInfo) = 0;
        
        virtual DWORD STDMETHODCALLTYPE CeSetFilePointer( 
            HANDLE hFile,
            LONG lDistanceToMove,
            PLONG lpDistanceToMoveHigh,
            DWORD dwMoveMethod) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeSetEndOfFile( 
            HANDLE hFile) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeCreateDirectory( 
            LPCWSTR lpPathName,
            LPSECURITY_ATTRIBUTES lpSecurityAttributes) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeRemoveDirectory( 
            LPCWSTR lpPathName) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeCreateProcess( 
            LPCWSTR lpszImageName,
            LPCWSTR lpszCmdLine,
            LPSECURITY_ATTRIBUTES lpsaProcess,
            LPSECURITY_ATTRIBUTES lpsaThread,
            BOOL fInheritHandles,
            DWORD fdwCreate,
            LPVOID lpvEnvironment,
            LPWSTR lpszCurDir,
            LPSTARTUPINFOW lpsiStartInfo,
            LPPROCESS_INFORMATION lppiProcInfo) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeMoveFile( 
            LPCWSTR lpExistingFileName,
            LPCWSTR lpNewFileName) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeCopyFile( 
            LPCWSTR lpExistingFileName,
            LPCWSTR lpNewFileName,
            BOOL bFailIfExists) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeDeleteFile( 
            LPCWSTR FileName) = 0;
        
        virtual DWORD STDMETHODCALLTYPE CeGetFileSize( 
            HANDLE hFile,
            LPDWORD lpFileSizeHigh) = 0;
        
        virtual LONG STDMETHODCALLTYPE CeRegOpenKeyEx( 
            HKEY hKey,
            LPCWSTR lpszSubKey,
            DWORD dwReserved,
            REGSAM samDesired,
            PHKEY phkResult) = 0;
        
        virtual LONG STDMETHODCALLTYPE CeRegEnumKeyEx( 
            HKEY hKey,
            DWORD dwIndex,
            LPWSTR lpName,
            LPDWORD lpcbName,
            LPDWORD lpReserved,
            LPWSTR lpClass,
            LPDWORD lpcbClass,
            PFILETIME lpftLastWriteTime) = 0;
        
        virtual LONG STDMETHODCALLTYPE CeRegCreateKeyEx( 
            HKEY hKey,
            LPCWSTR lpszSubKey,
            DWORD dwReserved,
            LPWSTR lpszClass,
            DWORD fdwOptions,
            REGSAM samDesired,
            LPSECURITY_ATTRIBUTES lpSecurityAttributes,
            PHKEY phkResult,
            LPDWORD lpdwDisposition) = 0;
        
        virtual LONG STDMETHODCALLTYPE CeRegCloseKey( 
            HKEY hKey) = 0;
        
        virtual LONG STDMETHODCALLTYPE CeRegDeleteKey( 
            HKEY hKey,
            LPCWSTR lpszSubKey) = 0;
        
        virtual LONG STDMETHODCALLTYPE CeRegEnumValue( 
            HKEY hKey,
            DWORD dwIndex,
            LPWSTR lpszValueName,
            LPDWORD lpcbValueName,
            LPDWORD lpReserved,
            LPDWORD lpType,
            LPBYTE lpData,
            LPDWORD lpcbData) = 0;
        
        virtual LONG STDMETHODCALLTYPE CeRegDeleteValue( 
            HKEY hKey,
            LPCWSTR lpszValueName) = 0;
        
        virtual LONG STDMETHODCALLTYPE CeRegQueryInfoKey( 
            HKEY hKey,
            LPWSTR lpClass,
            LPDWORD lpcbClass,
            LPDWORD lpReserved,
            LPDWORD lpcSubKeys,
            LPDWORD lpcbMaxSubKeyLen,
            LPDWORD lpcbMaxClassLen,
            LPDWORD lpcValues,
            LPDWORD lpcbMaxValueNameLen,
            LPDWORD lpcbMaxValueLen,
            LPDWORD lpcbSecurityDescriptor,
            PFILETIME lpftLastWriteTime) = 0;
        
        virtual LONG STDMETHODCALLTYPE CeRegQueryValueEx( 
            HKEY hKey,
            LPCWSTR lpValueName,
            LPDWORD lpReserved,
            LPDWORD lpType,
            LPBYTE lpData,
            LPDWORD lpcbData) = 0;
        
        virtual LONG STDMETHODCALLTYPE CeRegSetValueEx( 
            HKEY hKey,
            LPCWSTR lpValueName,
            DWORD Reserved,
            DWORD dwType,
            BYTE *lpData,
            DWORD cbData) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeGetStoreInformation( 
            LPSTORE_INFORMATION lpsi) = 0;
        
        virtual INT STDMETHODCALLTYPE CeGetSystemMetrics( 
            INT nIndex) = 0;
        
        virtual INT STDMETHODCALLTYPE CeGetDesktopDeviceCaps( 
            INT nIndex) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeFindAllDatabases( 
            DWORD DbaseType,
            WORD Flags,
            LPWORD cFindData,
            LPLPCEDB_FIND_DATA ppFindData) = 0;
        
        virtual void STDMETHODCALLTYPE CeGetSystemInfo( 
            LPSYSTEM_INFO lpSystemInfo) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeSHCreateShortcut( 
            LPWSTR lpszShortcut,
            LPWSTR lpszTarget) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeSHGetShortcutTarget( 
            LPWSTR lpszShortcut,
            LPWSTR lpszTarget,
            int cbMax) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeCheckPassword( 
            LPWSTR lpszPassword) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeGetFileTime( 
            HANDLE hFile,
            LPFILETIME lpCreationTime,
            LPFILETIME lpLastAccessTime,
            LPFILETIME lpLastWriteTime) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeSetFileTime( 
            HANDLE hFile,
            FILETIME *lpCreationTime,
            FILETIME *lpLastAccessTime,
            FILETIME *lpLastWriteTime) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeGetVersionEx( 
            LPCEOSVERSIONINFO lpVersionInformation) = 0;
        
        virtual HWND STDMETHODCALLTYPE CeGetWindow( 
            HWND hWnd,
            UINT uCmd) = 0;
        
        virtual LONG STDMETHODCALLTYPE CeGetWindowLong( 
            HWND hWnd,
            int nIndex) = 0;
        
        virtual INT STDMETHODCALLTYPE CeGetWindowText( 
            HWND hWnd,
            LPWSTR lpString,
            int nMaxCount) = 0;
        
        virtual INT STDMETHODCALLTYPE CeGetClassName( 
            HWND hWnd,
            LPWSTR lpClassName,
            int nMaxCount) = 0;
        
        virtual void STDMETHODCALLTYPE CeGlobalMemoryStatus( 
            LPMEMORYSTATUS lpmst) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeGetSystemPowerStatusEx( 
            PSYSTEM_POWER_STATUS_EX pstatus,
            BOOL fUpdate) = 0;
        
        virtual DWORD STDMETHODCALLTYPE CeGetTempPath( 
            DWORD nBufferLength,
            LPWSTR lpBuffer) = 0;
        
        virtual DWORD STDMETHODCALLTYPE CeGetSpecialFolderPath( 
            int nFolder,
            DWORD nBufferLength,
            LPWSTR lpBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CeRapiInvoke( 
            LPCWSTR pDllPath,
            LPCWSTR pFunctionName,
            DWORD cbInput,
            BYTE *pInput,
            DWORD *pcbOutput,
            BYTE **ppOutput,
            IRAPIStream **ppIRAPIStream,
            DWORD dwReserved) = 0;
        
        virtual HANDLE STDMETHODCALLTYPE CeFindFirstDatabaseEx( 
            PCEGUID pguid,
            DWORD dwDbaseType) = 0;
        
        virtual CEOID STDMETHODCALLTYPE CeFindNextDatabaseEx( 
            HANDLE hEnum,
            PCEGUID pguid) = 0;
        
        virtual CEOID STDMETHODCALLTYPE CeCreateDatabaseEx( 
            PCEGUID pceguid,
            CEDBASEINFO *lpCEDBInfo) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeSetDatabaseInfoEx( 
            PCEGUID pceguid,
            CEOID oidDbase,
            CEDBASEINFO *pNewInfo) = 0;
        
        virtual HANDLE STDMETHODCALLTYPE CeOpenDatabaseEx( 
            PCEGUID pceguid,
            PCEOID poid,
            LPWSTR lpszName,
            CEPROPID propid,
            DWORD dwFlags,
            CENOTIFYREQUEST *pReq) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeDeleteDatabaseEx( 
            PCEGUID pceguid,
            CEOID oidDbase) = 0;
        
        virtual CEOID STDMETHODCALLTYPE CeReadRecordPropsEx( 
            HANDLE hDbase,
            DWORD dwFlags,
            LPWORD lpcPropID,
            CEPROPID *rgPropID,
            LPBYTE *lplpBuffer,
            LPDWORD lpcbBuffer,
            HANDLE hHeap) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeMountDBVol( 
            PCEGUID pceguid,
            LPWSTR lpszDBVol,
            DWORD dwFlags) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeUnmountDBVol( 
            PCEGUID pceguid) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeFlushDBVol( 
            PCEGUID pceguid) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeEnumDBVolumes( 
            PCEGUID pceguid,
            LPWSTR lpBuf,
            DWORD dwNumChars) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeOidGetInfoEx( 
            PCEGUID pceguid,
            CEOID oid,
            CEOIDINFO *oidInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CeSyncStart( 
            LPCWSTR szCommand) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CeSyncStop( void) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeQueryInstructionSet( 
            DWORD dwInstructionSet,
            LPDWORD lpdwCurrentInstructionSet) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CeGetDiskFreeSpaceEx( 
            LPCWSTR lpDirectoryName,
            ULARGE_INTEGER *lpFreeBytesAvailableToCaller,
            ULARGE_INTEGER *lpTotalNumberOfBytes,
            ULARGE_INTEGER *lpTotalNumberOfFreeBytes) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRAPISessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRAPISession * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRAPISession * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRAPISession * This);
        
        HRESULT ( STDMETHODCALLTYPE *CeRapiInit )( 
            IRAPISession * This);
        
        HRESULT ( STDMETHODCALLTYPE *CeRapiUninit )( 
            IRAPISession * This);
        
        DWORD ( STDMETHODCALLTYPE *CeGetLastError )( 
            IRAPISession * This);
        
        HRESULT ( STDMETHODCALLTYPE *CeRapiGetError )( 
            IRAPISession * This);
        
        HRESULT ( STDMETHODCALLTYPE *CeRapiFreeBuffer )( 
            IRAPISession * This,
            void *Buffer);
        
        HANDLE ( STDMETHODCALLTYPE *CeFindFirstFile )( 
            IRAPISession * This,
            LPCWSTR FileName,
            LPCE_FIND_DATA FindData);
        
        BOOL ( STDMETHODCALLTYPE *CeFindNextFile )( 
            IRAPISession * This,
            HANDLE FoundFile,
            LPCE_FIND_DATA FindData);
        
        BOOL ( STDMETHODCALLTYPE *CeFindClose )( 
            IRAPISession * This,
            HANDLE FoundFile);
        
        DWORD ( STDMETHODCALLTYPE *CeGetFileAttributes )( 
            IRAPISession * This,
            LPCWSTR FileName);
        
        BOOL ( STDMETHODCALLTYPE *CeSetFileAttributes )( 
            IRAPISession * This,
            LPCWSTR FileName,
            DWORD FileAttrib);
        
        HANDLE ( STDMETHODCALLTYPE *CeCreateFile )( 
            IRAPISession * This,
            LPCWSTR lpFileName,
            DWORD dwDesiredAccess,
            DWORD dwShareMode,
            LPSECURITY_ATTRIBUTES lpSecurityAttributes,
            DWORD dwCreationDistribution,
            DWORD dwFlagsAndAttributes,
            HANDLE hTemplateFile);
        
        BOOL ( STDMETHODCALLTYPE *CeReadFile )( 
            IRAPISession * This,
            HANDLE hFile,
            LPVOID lpBuffer,
            DWORD nNumberOfBytesToRead,
            LPDWORD lpNumberOfBytesRead,
            LPOVERLAPPED lpOverlapped);
        
        BOOL ( STDMETHODCALLTYPE *CeWriteFile )( 
            IRAPISession * This,
            HANDLE hFile,
            LPCVOID lpBuffer,
            DWORD nNumberOfBytesToWrite,
            LPDWORD lpNumberOfBytesWritten,
            LPOVERLAPPED lpOverlapped);
        
        BOOL ( STDMETHODCALLTYPE *CeCloseHandle )( 
            IRAPISession * This,
            HANDLE hObject);
        
        BOOL ( STDMETHODCALLTYPE *CeFindAllFiles )( 
            IRAPISession * This,
            LPCWSTR Path,
            DWORD Flags,
            LPDWORD pFoundCount,
            LPLPCE_FIND_DATA ppFindDataArray);
        
        HANDLE ( STDMETHODCALLTYPE *CeFindFirstDatabase )( 
            IRAPISession * This,
            DWORD dwDbaseType);
        
        CEOID ( STDMETHODCALLTYPE *CeFindNextDatabase )( 
            IRAPISession * This,
            HANDLE hEnum);
        
        BOOL ( STDMETHODCALLTYPE *CeOidGetInfo )( 
            IRAPISession * This,
            CEOID oid,
            CEOIDINFO *oidInfo);
        
        CEOID ( STDMETHODCALLTYPE *CeCreateDatabase )( 
            IRAPISession * This,
            LPWSTR lpszName,
            DWORD dwDbaseType,
            WORD cNumSortOrder,
            SORTORDERSPEC *rgSortSpecs);
        
        HANDLE ( STDMETHODCALLTYPE *CeOpenDatabase )( 
            IRAPISession * This,
            PCEOID poid,
            LPWSTR lpszName,
            CEPROPID propid,
            DWORD dwFlags,
            HWND hwndNotify);
        
        BOOL ( STDMETHODCALLTYPE *CeDeleteDatabase )( 
            IRAPISession * This,
            CEOID oidDbase);
        
        CEOID ( STDMETHODCALLTYPE *CeReadRecordProps )( 
            IRAPISession * This,
            HANDLE hDbase,
            DWORD dwFlags,
            LPWORD lpcPropID,
            CEPROPID *rgPropID,
            LPBYTE *lplpBuffer,
            LPDWORD lpcbBuffer);
        
        CEOID ( STDMETHODCALLTYPE *CeWriteRecordProps )( 
            IRAPISession * This,
            HANDLE hDbase,
            CEOID oidRecord,
            WORD cPropID,
            CEPROPVAL *rgPropVal);
        
        BOOL ( STDMETHODCALLTYPE *CeDeleteRecord )( 
            IRAPISession * This,
            HANDLE hDatabase,
            CEOID oidRecord);
        
        CEOID ( STDMETHODCALLTYPE *CeSeekDatabase )( 
            IRAPISession * This,
            HANDLE hDatabase,
            DWORD dwSeekType,
            DWORD dwValue,
            LPDWORD lpdwIndex);
        
        BOOL ( STDMETHODCALLTYPE *CeSetDatabaseInfo )( 
            IRAPISession * This,
            CEOID oidDbase,
            CEDBASEINFO *pNewInfo);
        
        DWORD ( STDMETHODCALLTYPE *CeSetFilePointer )( 
            IRAPISession * This,
            HANDLE hFile,
            LONG lDistanceToMove,
            PLONG lpDistanceToMoveHigh,
            DWORD dwMoveMethod);
        
        BOOL ( STDMETHODCALLTYPE *CeSetEndOfFile )( 
            IRAPISession * This,
            HANDLE hFile);
        
        BOOL ( STDMETHODCALLTYPE *CeCreateDirectory )( 
            IRAPISession * This,
            LPCWSTR lpPathName,
            LPSECURITY_ATTRIBUTES lpSecurityAttributes);
        
        BOOL ( STDMETHODCALLTYPE *CeRemoveDirectory )( 
            IRAPISession * This,
            LPCWSTR lpPathName);
        
        BOOL ( STDMETHODCALLTYPE *CeCreateProcess )( 
            IRAPISession * This,
            LPCWSTR lpszImageName,
            LPCWSTR lpszCmdLine,
            LPSECURITY_ATTRIBUTES lpsaProcess,
            LPSECURITY_ATTRIBUTES lpsaThread,
            BOOL fInheritHandles,
            DWORD fdwCreate,
            LPVOID lpvEnvironment,
            LPWSTR lpszCurDir,
            LPSTARTUPINFOW lpsiStartInfo,
            LPPROCESS_INFORMATION lppiProcInfo);
        
        BOOL ( STDMETHODCALLTYPE *CeMoveFile )( 
            IRAPISession * This,
            LPCWSTR lpExistingFileName,
            LPCWSTR lpNewFileName);
        
        BOOL ( STDMETHODCALLTYPE *CeCopyFile )( 
            IRAPISession * This,
            LPCWSTR lpExistingFileName,
            LPCWSTR lpNewFileName,
            BOOL bFailIfExists);
        
        BOOL ( STDMETHODCALLTYPE *CeDeleteFile )( 
            IRAPISession * This,
            LPCWSTR FileName);
        
        DWORD ( STDMETHODCALLTYPE *CeGetFileSize )( 
            IRAPISession * This,
            HANDLE hFile,
            LPDWORD lpFileSizeHigh);
        
        LONG ( STDMETHODCALLTYPE *CeRegOpenKeyEx )( 
            IRAPISession * This,
            HKEY hKey,
            LPCWSTR lpszSubKey,
            DWORD dwReserved,
            REGSAM samDesired,
            PHKEY phkResult);
        
        LONG ( STDMETHODCALLTYPE *CeRegEnumKeyEx )( 
            IRAPISession * This,
            HKEY hKey,
            DWORD dwIndex,
            LPWSTR lpName,
            LPDWORD lpcbName,
            LPDWORD lpReserved,
            LPWSTR lpClass,
            LPDWORD lpcbClass,
            PFILETIME lpftLastWriteTime);
        
        LONG ( STDMETHODCALLTYPE *CeRegCreateKeyEx )( 
            IRAPISession * This,
            HKEY hKey,
            LPCWSTR lpszSubKey,
            DWORD dwReserved,
            LPWSTR lpszClass,
            DWORD fdwOptions,
            REGSAM samDesired,
            LPSECURITY_ATTRIBUTES lpSecurityAttributes,
            PHKEY phkResult,
            LPDWORD lpdwDisposition);
        
        LONG ( STDMETHODCALLTYPE *CeRegCloseKey )( 
            IRAPISession * This,
            HKEY hKey);
        
        LONG ( STDMETHODCALLTYPE *CeRegDeleteKey )( 
            IRAPISession * This,
            HKEY hKey,
            LPCWSTR lpszSubKey);
        
        LONG ( STDMETHODCALLTYPE *CeRegEnumValue )( 
            IRAPISession * This,
            HKEY hKey,
            DWORD dwIndex,
            LPWSTR lpszValueName,
            LPDWORD lpcbValueName,
            LPDWORD lpReserved,
            LPDWORD lpType,
            LPBYTE lpData,
            LPDWORD lpcbData);
        
        LONG ( STDMETHODCALLTYPE *CeRegDeleteValue )( 
            IRAPISession * This,
            HKEY hKey,
            LPCWSTR lpszValueName);
        
        LONG ( STDMETHODCALLTYPE *CeRegQueryInfoKey )( 
            IRAPISession * This,
            HKEY hKey,
            LPWSTR lpClass,
            LPDWORD lpcbClass,
            LPDWORD lpReserved,
            LPDWORD lpcSubKeys,
            LPDWORD lpcbMaxSubKeyLen,
            LPDWORD lpcbMaxClassLen,
            LPDWORD lpcValues,
            LPDWORD lpcbMaxValueNameLen,
            LPDWORD lpcbMaxValueLen,
            LPDWORD lpcbSecurityDescriptor,
            PFILETIME lpftLastWriteTime);
        
        LONG ( STDMETHODCALLTYPE *CeRegQueryValueEx )( 
            IRAPISession * This,
            HKEY hKey,
            LPCWSTR lpValueName,
            LPDWORD lpReserved,
            LPDWORD lpType,
            LPBYTE lpData,
            LPDWORD lpcbData);
        
        LONG ( STDMETHODCALLTYPE *CeRegSetValueEx )( 
            IRAPISession * This,
            HKEY hKey,
            LPCWSTR lpValueName,
            DWORD Reserved,
            DWORD dwType,
            BYTE *lpData,
            DWORD cbData);
        
        BOOL ( STDMETHODCALLTYPE *CeGetStoreInformation )( 
            IRAPISession * This,
            LPSTORE_INFORMATION lpsi);
        
        INT ( STDMETHODCALLTYPE *CeGetSystemMetrics )( 
            IRAPISession * This,
            INT nIndex);
        
        INT ( STDMETHODCALLTYPE *CeGetDesktopDeviceCaps )( 
            IRAPISession * This,
            INT nIndex);
        
        BOOL ( STDMETHODCALLTYPE *CeFindAllDatabases )( 
            IRAPISession * This,
            DWORD DbaseType,
            WORD Flags,
            LPWORD cFindData,
            LPLPCEDB_FIND_DATA ppFindData);
        
        void ( STDMETHODCALLTYPE *CeGetSystemInfo )( 
            IRAPISession * This,
            LPSYSTEM_INFO lpSystemInfo);
        
        BOOL ( STDMETHODCALLTYPE *CeSHCreateShortcut )( 
            IRAPISession * This,
            LPWSTR lpszShortcut,
            LPWSTR lpszTarget);
        
        BOOL ( STDMETHODCALLTYPE *CeSHGetShortcutTarget )( 
            IRAPISession * This,
            LPWSTR lpszShortcut,
            LPWSTR lpszTarget,
            int cbMax);
        
        BOOL ( STDMETHODCALLTYPE *CeCheckPassword )( 
            IRAPISession * This,
            LPWSTR lpszPassword);
        
        BOOL ( STDMETHODCALLTYPE *CeGetFileTime )( 
            IRAPISession * This,
            HANDLE hFile,
            LPFILETIME lpCreationTime,
            LPFILETIME lpLastAccessTime,
            LPFILETIME lpLastWriteTime);
        
        BOOL ( STDMETHODCALLTYPE *CeSetFileTime )( 
            IRAPISession * This,
            HANDLE hFile,
            FILETIME *lpCreationTime,
            FILETIME *lpLastAccessTime,
            FILETIME *lpLastWriteTime);
        
        BOOL ( STDMETHODCALLTYPE *CeGetVersionEx )( 
            IRAPISession * This,
            LPCEOSVERSIONINFO lpVersionInformation);
        
        HWND ( STDMETHODCALLTYPE *CeGetWindow )( 
            IRAPISession * This,
            HWND hWnd,
            UINT uCmd);
        
        LONG ( STDMETHODCALLTYPE *CeGetWindowLong )( 
            IRAPISession * This,
            HWND hWnd,
            int nIndex);
        
        INT ( STDMETHODCALLTYPE *CeGetWindowText )( 
            IRAPISession * This,
            HWND hWnd,
            LPWSTR lpString,
            int nMaxCount);
        
        INT ( STDMETHODCALLTYPE *CeGetClassName )( 
            IRAPISession * This,
            HWND hWnd,
            LPWSTR lpClassName,
            int nMaxCount);
        
        void ( STDMETHODCALLTYPE *CeGlobalMemoryStatus )( 
            IRAPISession * This,
            LPMEMORYSTATUS lpmst);
        
        BOOL ( STDMETHODCALLTYPE *CeGetSystemPowerStatusEx )( 
            IRAPISession * This,
            PSYSTEM_POWER_STATUS_EX pstatus,
            BOOL fUpdate);
        
        DWORD ( STDMETHODCALLTYPE *CeGetTempPath )( 
            IRAPISession * This,
            DWORD nBufferLength,
            LPWSTR lpBuffer);
        
        DWORD ( STDMETHODCALLTYPE *CeGetSpecialFolderPath )( 
            IRAPISession * This,
            int nFolder,
            DWORD nBufferLength,
            LPWSTR lpBuffer);
        
        HRESULT ( STDMETHODCALLTYPE *CeRapiInvoke )( 
            IRAPISession * This,
            LPCWSTR pDllPath,
            LPCWSTR pFunctionName,
            DWORD cbInput,
            BYTE *pInput,
            DWORD *pcbOutput,
            BYTE **ppOutput,
            IRAPIStream **ppIRAPIStream,
            DWORD dwReserved);
        
        HANDLE ( STDMETHODCALLTYPE *CeFindFirstDatabaseEx )( 
            IRAPISession * This,
            PCEGUID pguid,
            DWORD dwDbaseType);
        
        CEOID ( STDMETHODCALLTYPE *CeFindNextDatabaseEx )( 
            IRAPISession * This,
            HANDLE hEnum,
            PCEGUID pguid);
        
        CEOID ( STDMETHODCALLTYPE *CeCreateDatabaseEx )( 
            IRAPISession * This,
            PCEGUID pceguid,
            CEDBASEINFO *lpCEDBInfo);
        
        BOOL ( STDMETHODCALLTYPE *CeSetDatabaseInfoEx )( 
            IRAPISession * This,
            PCEGUID pceguid,
            CEOID oidDbase,
            CEDBASEINFO *pNewInfo);
        
        HANDLE ( STDMETHODCALLTYPE *CeOpenDatabaseEx )( 
            IRAPISession * This,
            PCEGUID pceguid,
            PCEOID poid,
            LPWSTR lpszName,
            CEPROPID propid,
            DWORD dwFlags,
            CENOTIFYREQUEST *pReq);
        
        BOOL ( STDMETHODCALLTYPE *CeDeleteDatabaseEx )( 
            IRAPISession * This,
            PCEGUID pceguid,
            CEOID oidDbase);
        
        CEOID ( STDMETHODCALLTYPE *CeReadRecordPropsEx )( 
            IRAPISession * This,
            HANDLE hDbase,
            DWORD dwFlags,
            LPWORD lpcPropID,
            CEPROPID *rgPropID,
            LPBYTE *lplpBuffer,
            LPDWORD lpcbBuffer,
            HANDLE hHeap);
        
        BOOL ( STDMETHODCALLTYPE *CeMountDBVol )( 
            IRAPISession * This,
            PCEGUID pceguid,
            LPWSTR lpszDBVol,
            DWORD dwFlags);
        
        BOOL ( STDMETHODCALLTYPE *CeUnmountDBVol )( 
            IRAPISession * This,
            PCEGUID pceguid);
        
        BOOL ( STDMETHODCALLTYPE *CeFlushDBVol )( 
            IRAPISession * This,
            PCEGUID pceguid);
        
        BOOL ( STDMETHODCALLTYPE *CeEnumDBVolumes )( 
            IRAPISession * This,
            PCEGUID pceguid,
            LPWSTR lpBuf,
            DWORD dwNumChars);
        
        BOOL ( STDMETHODCALLTYPE *CeOidGetInfoEx )( 
            IRAPISession * This,
            PCEGUID pceguid,
            CEOID oid,
            CEOIDINFO *oidInfo);
        
        HRESULT ( STDMETHODCALLTYPE *CeSyncStart )( 
            IRAPISession * This,
            LPCWSTR szCommand);
        
        HRESULT ( STDMETHODCALLTYPE *CeSyncStop )( 
            IRAPISession * This);
        
        BOOL ( STDMETHODCALLTYPE *CeQueryInstructionSet )( 
            IRAPISession * This,
            DWORD dwInstructionSet,
            LPDWORD lpdwCurrentInstructionSet);
        
        BOOL ( STDMETHODCALLTYPE *CeGetDiskFreeSpaceEx )( 
            IRAPISession * This,
            LPCWSTR lpDirectoryName,
            ULARGE_INTEGER *lpFreeBytesAvailableToCaller,
            ULARGE_INTEGER *lpTotalNumberOfBytes,
            ULARGE_INTEGER *lpTotalNumberOfFreeBytes);
        
        END_INTERFACE
    } IRAPISessionVtbl;

    interface IRAPISession
    {
        CONST_VTBL struct IRAPISessionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRAPISession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRAPISession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRAPISession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRAPISession_CeRapiInit(This)	\
    (This)->lpVtbl -> CeRapiInit(This)

#define IRAPISession_CeRapiUninit(This)	\
    (This)->lpVtbl -> CeRapiUninit(This)

#define IRAPISession_CeGetLastError(This)	\
    (This)->lpVtbl -> CeGetLastError(This)

#define IRAPISession_CeRapiGetError(This)	\
    (This)->lpVtbl -> CeRapiGetError(This)

#define IRAPISession_CeRapiFreeBuffer(This,Buffer)	\
    (This)->lpVtbl -> CeRapiFreeBuffer(This,Buffer)

#define IRAPISession_CeFindFirstFile(This,FileName,FindData)	\
    (This)->lpVtbl -> CeFindFirstFile(This,FileName,FindData)

#define IRAPISession_CeFindNextFile(This,FoundFile,FindData)	\
    (This)->lpVtbl -> CeFindNextFile(This,FoundFile,FindData)

#define IRAPISession_CeFindClose(This,FoundFile)	\
    (This)->lpVtbl -> CeFindClose(This,FoundFile)

#define IRAPISession_CeGetFileAttributes(This,FileName)	\
    (This)->lpVtbl -> CeGetFileAttributes(This,FileName)

#define IRAPISession_CeSetFileAttributes(This,FileName,FileAttrib)	\
    (This)->lpVtbl -> CeSetFileAttributes(This,FileName,FileAttrib)

#define IRAPISession_CeCreateFile(This,lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDistribution,dwFlagsAndAttributes,hTemplateFile)	\
    (This)->lpVtbl -> CeCreateFile(This,lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDistribution,dwFlagsAndAttributes,hTemplateFile)

#define IRAPISession_CeReadFile(This,hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped)	\
    (This)->lpVtbl -> CeReadFile(This,hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped)

#define IRAPISession_CeWriteFile(This,hFile,lpBuffer,nNumberOfBytesToWrite,lpNumberOfBytesWritten,lpOverlapped)	\
    (This)->lpVtbl -> CeWriteFile(This,hFile,lpBuffer,nNumberOfBytesToWrite,lpNumberOfBytesWritten,lpOverlapped)

#define IRAPISession_CeCloseHandle(This,hObject)	\
    (This)->lpVtbl -> CeCloseHandle(This,hObject)

#define IRAPISession_CeFindAllFiles(This,Path,Flags,pFoundCount,ppFindDataArray)	\
    (This)->lpVtbl -> CeFindAllFiles(This,Path,Flags,pFoundCount,ppFindDataArray)

#define IRAPISession_CeFindFirstDatabase(This,dwDbaseType)	\
    (This)->lpVtbl -> CeFindFirstDatabase(This,dwDbaseType)

#define IRAPISession_CeFindNextDatabase(This,hEnum)	\
    (This)->lpVtbl -> CeFindNextDatabase(This,hEnum)

#define IRAPISession_CeOidGetInfo(This,oid,oidInfo)	\
    (This)->lpVtbl -> CeOidGetInfo(This,oid,oidInfo)

#define IRAPISession_CeCreateDatabase(This,lpszName,dwDbaseType,cNumSortOrder,rgSortSpecs)	\
    (This)->lpVtbl -> CeCreateDatabase(This,lpszName,dwDbaseType,cNumSortOrder,rgSortSpecs)

#define IRAPISession_CeOpenDatabase(This,poid,lpszName,propid,dwFlags,hwndNotify)	\
    (This)->lpVtbl -> CeOpenDatabase(This,poid,lpszName,propid,dwFlags,hwndNotify)

#define IRAPISession_CeDeleteDatabase(This,oidDbase)	\
    (This)->lpVtbl -> CeDeleteDatabase(This,oidDbase)

#define IRAPISession_CeReadRecordProps(This,hDbase,dwFlags,lpcPropID,rgPropID,lplpBuffer,lpcbBuffer)	\
    (This)->lpVtbl -> CeReadRecordProps(This,hDbase,dwFlags,lpcPropID,rgPropID,lplpBuffer,lpcbBuffer)

#define IRAPISession_CeWriteRecordProps(This,hDbase,oidRecord,cPropID,rgPropVal)	\
    (This)->lpVtbl -> CeWriteRecordProps(This,hDbase,oidRecord,cPropID,rgPropVal)

#define IRAPISession_CeDeleteRecord(This,hDatabase,oidRecord)	\
    (This)->lpVtbl -> CeDeleteRecord(This,hDatabase,oidRecord)

#define IRAPISession_CeSeekDatabase(This,hDatabase,dwSeekType,dwValue,lpdwIndex)	\
    (This)->lpVtbl -> CeSeekDatabase(This,hDatabase,dwSeekType,dwValue,lpdwIndex)

#define IRAPISession_CeSetDatabaseInfo(This,oidDbase,pNewInfo)	\
    (This)->lpVtbl -> CeSetDatabaseInfo(This,oidDbase,pNewInfo)

#define IRAPISession_CeSetFilePointer(This,hFile,lDistanceToMove,lpDistanceToMoveHigh,dwMoveMethod)	\
    (This)->lpVtbl -> CeSetFilePointer(This,hFile,lDistanceToMove,lpDistanceToMoveHigh,dwMoveMethod)

#define IRAPISession_CeSetEndOfFile(This,hFile)	\
    (This)->lpVtbl -> CeSetEndOfFile(This,hFile)

#define IRAPISession_CeCreateDirectory(This,lpPathName,lpSecurityAttributes)	\
    (This)->lpVtbl -> CeCreateDirectory(This,lpPathName,lpSecurityAttributes)

#define IRAPISession_CeRemoveDirectory(This,lpPathName)	\
    (This)->lpVtbl -> CeRemoveDirectory(This,lpPathName)

#define IRAPISession_CeCreateProcess(This,lpszImageName,lpszCmdLine,lpsaProcess,lpsaThread,fInheritHandles,fdwCreate,lpvEnvironment,lpszCurDir,lpsiStartInfo,lppiProcInfo)	\
    (This)->lpVtbl -> CeCreateProcess(This,lpszImageName,lpszCmdLine,lpsaProcess,lpsaThread,fInheritHandles,fdwCreate,lpvEnvironment,lpszCurDir,lpsiStartInfo,lppiProcInfo)

#define IRAPISession_CeMoveFile(This,lpExistingFileName,lpNewFileName)	\
    (This)->lpVtbl -> CeMoveFile(This,lpExistingFileName,lpNewFileName)

#define IRAPISession_CeCopyFile(This,lpExistingFileName,lpNewFileName,bFailIfExists)	\
    (This)->lpVtbl -> CeCopyFile(This,lpExistingFileName,lpNewFileName,bFailIfExists)

#define IRAPISession_CeDeleteFile(This,FileName)	\
    (This)->lpVtbl -> CeDeleteFile(This,FileName)

#define IRAPISession_CeGetFileSize(This,hFile,lpFileSizeHigh)	\
    (This)->lpVtbl -> CeGetFileSize(This,hFile,lpFileSizeHigh)

#define IRAPISession_CeRegOpenKeyEx(This,hKey,lpszSubKey,dwReserved,samDesired,phkResult)	\
    (This)->lpVtbl -> CeRegOpenKeyEx(This,hKey,lpszSubKey,dwReserved,samDesired,phkResult)

#define IRAPISession_CeRegEnumKeyEx(This,hKey,dwIndex,lpName,lpcbName,lpReserved,lpClass,lpcbClass,lpftLastWriteTime)	\
    (This)->lpVtbl -> CeRegEnumKeyEx(This,hKey,dwIndex,lpName,lpcbName,lpReserved,lpClass,lpcbClass,lpftLastWriteTime)

#define IRAPISession_CeRegCreateKeyEx(This,hKey,lpszSubKey,dwReserved,lpszClass,fdwOptions,samDesired,lpSecurityAttributes,phkResult,lpdwDisposition)	\
    (This)->lpVtbl -> CeRegCreateKeyEx(This,hKey,lpszSubKey,dwReserved,lpszClass,fdwOptions,samDesired,lpSecurityAttributes,phkResult,lpdwDisposition)

#define IRAPISession_CeRegCloseKey(This,hKey)	\
    (This)->lpVtbl -> CeRegCloseKey(This,hKey)

#define IRAPISession_CeRegDeleteKey(This,hKey,lpszSubKey)	\
    (This)->lpVtbl -> CeRegDeleteKey(This,hKey,lpszSubKey)

#define IRAPISession_CeRegEnumValue(This,hKey,dwIndex,lpszValueName,lpcbValueName,lpReserved,lpType,lpData,lpcbData)	\
    (This)->lpVtbl -> CeRegEnumValue(This,hKey,dwIndex,lpszValueName,lpcbValueName,lpReserved,lpType,lpData,lpcbData)

#define IRAPISession_CeRegDeleteValue(This,hKey,lpszValueName)	\
    (This)->lpVtbl -> CeRegDeleteValue(This,hKey,lpszValueName)

#define IRAPISession_CeRegQueryInfoKey(This,hKey,lpClass,lpcbClass,lpReserved,lpcSubKeys,lpcbMaxSubKeyLen,lpcbMaxClassLen,lpcValues,lpcbMaxValueNameLen,lpcbMaxValueLen,lpcbSecurityDescriptor,lpftLastWriteTime)	\
    (This)->lpVtbl -> CeRegQueryInfoKey(This,hKey,lpClass,lpcbClass,lpReserved,lpcSubKeys,lpcbMaxSubKeyLen,lpcbMaxClassLen,lpcValues,lpcbMaxValueNameLen,lpcbMaxValueLen,lpcbSecurityDescriptor,lpftLastWriteTime)

#define IRAPISession_CeRegQueryValueEx(This,hKey,lpValueName,lpReserved,lpType,lpData,lpcbData)	\
    (This)->lpVtbl -> CeRegQueryValueEx(This,hKey,lpValueName,lpReserved,lpType,lpData,lpcbData)

#define IRAPISession_CeRegSetValueEx(This,hKey,lpValueName,Reserved,dwType,lpData,cbData)	\
    (This)->lpVtbl -> CeRegSetValueEx(This,hKey,lpValueName,Reserved,dwType,lpData,cbData)

#define IRAPISession_CeGetStoreInformation(This,lpsi)	\
    (This)->lpVtbl -> CeGetStoreInformation(This,lpsi)

#define IRAPISession_CeGetSystemMetrics(This,nIndex)	\
    (This)->lpVtbl -> CeGetSystemMetrics(This,nIndex)

#define IRAPISession_CeGetDesktopDeviceCaps(This,nIndex)	\
    (This)->lpVtbl -> CeGetDesktopDeviceCaps(This,nIndex)

#define IRAPISession_CeFindAllDatabases(This,DbaseType,Flags,cFindData,ppFindData)	\
    (This)->lpVtbl -> CeFindAllDatabases(This,DbaseType,Flags,cFindData,ppFindData)

#define IRAPISession_CeGetSystemInfo(This,lpSystemInfo)	\
    (This)->lpVtbl -> CeGetSystemInfo(This,lpSystemInfo)

#define IRAPISession_CeSHCreateShortcut(This,lpszShortcut,lpszTarget)	\
    (This)->lpVtbl -> CeSHCreateShortcut(This,lpszShortcut,lpszTarget)

#define IRAPISession_CeSHGetShortcutTarget(This,lpszShortcut,lpszTarget,cbMax)	\
    (This)->lpVtbl -> CeSHGetShortcutTarget(This,lpszShortcut,lpszTarget,cbMax)

#define IRAPISession_CeCheckPassword(This,lpszPassword)	\
    (This)->lpVtbl -> CeCheckPassword(This,lpszPassword)

#define IRAPISession_CeGetFileTime(This,hFile,lpCreationTime,lpLastAccessTime,lpLastWriteTime)	\
    (This)->lpVtbl -> CeGetFileTime(This,hFile,lpCreationTime,lpLastAccessTime,lpLastWriteTime)

#define IRAPISession_CeSetFileTime(This,hFile,lpCreationTime,lpLastAccessTime,lpLastWriteTime)	\
    (This)->lpVtbl -> CeSetFileTime(This,hFile,lpCreationTime,lpLastAccessTime,lpLastWriteTime)

#define IRAPISession_CeGetVersionEx(This,lpVersionInformation)	\
    (This)->lpVtbl -> CeGetVersionEx(This,lpVersionInformation)

#define IRAPISession_CeGetWindow(This,hWnd,uCmd)	\
    (This)->lpVtbl -> CeGetWindow(This,hWnd,uCmd)

#define IRAPISession_CeGetWindowLong(This,hWnd,nIndex)	\
    (This)->lpVtbl -> CeGetWindowLong(This,hWnd,nIndex)

#define IRAPISession_CeGetWindowText(This,hWnd,lpString,nMaxCount)	\
    (This)->lpVtbl -> CeGetWindowText(This,hWnd,lpString,nMaxCount)

#define IRAPISession_CeGetClassName(This,hWnd,lpClassName,nMaxCount)	\
    (This)->lpVtbl -> CeGetClassName(This,hWnd,lpClassName,nMaxCount)

#define IRAPISession_CeGlobalMemoryStatus(This,lpmst)	\
    (This)->lpVtbl -> CeGlobalMemoryStatus(This,lpmst)

#define IRAPISession_CeGetSystemPowerStatusEx(This,pstatus,fUpdate)	\
    (This)->lpVtbl -> CeGetSystemPowerStatusEx(This,pstatus,fUpdate)

#define IRAPISession_CeGetTempPath(This,nBufferLength,lpBuffer)	\
    (This)->lpVtbl -> CeGetTempPath(This,nBufferLength,lpBuffer)

#define IRAPISession_CeGetSpecialFolderPath(This,nFolder,nBufferLength,lpBuffer)	\
    (This)->lpVtbl -> CeGetSpecialFolderPath(This,nFolder,nBufferLength,lpBuffer)

#define IRAPISession_CeRapiInvoke(This,pDllPath,pFunctionName,cbInput,pInput,pcbOutput,ppOutput,ppIRAPIStream,dwReserved)	\
    (This)->lpVtbl -> CeRapiInvoke(This,pDllPath,pFunctionName,cbInput,pInput,pcbOutput,ppOutput,ppIRAPIStream,dwReserved)

#define IRAPISession_CeFindFirstDatabaseEx(This,pguid,dwDbaseType)	\
    (This)->lpVtbl -> CeFindFirstDatabaseEx(This,pguid,dwDbaseType)

#define IRAPISession_CeFindNextDatabaseEx(This,hEnum,pguid)	\
    (This)->lpVtbl -> CeFindNextDatabaseEx(This,hEnum,pguid)

#define IRAPISession_CeCreateDatabaseEx(This,pceguid,lpCEDBInfo)	\
    (This)->lpVtbl -> CeCreateDatabaseEx(This,pceguid,lpCEDBInfo)

#define IRAPISession_CeSetDatabaseInfoEx(This,pceguid,oidDbase,pNewInfo)	\
    (This)->lpVtbl -> CeSetDatabaseInfoEx(This,pceguid,oidDbase,pNewInfo)

#define IRAPISession_CeOpenDatabaseEx(This,pceguid,poid,lpszName,propid,dwFlags,pReq)	\
    (This)->lpVtbl -> CeOpenDatabaseEx(This,pceguid,poid,lpszName,propid,dwFlags,pReq)

#define IRAPISession_CeDeleteDatabaseEx(This,pceguid,oidDbase)	\
    (This)->lpVtbl -> CeDeleteDatabaseEx(This,pceguid,oidDbase)

#define IRAPISession_CeReadRecordPropsEx(This,hDbase,dwFlags,lpcPropID,rgPropID,lplpBuffer,lpcbBuffer,hHeap)	\
    (This)->lpVtbl -> CeReadRecordPropsEx(This,hDbase,dwFlags,lpcPropID,rgPropID,lplpBuffer,lpcbBuffer,hHeap)

#define IRAPISession_CeMountDBVol(This,pceguid,lpszDBVol,dwFlags)	\
    (This)->lpVtbl -> CeMountDBVol(This,pceguid,lpszDBVol,dwFlags)

#define IRAPISession_CeUnmountDBVol(This,pceguid)	\
    (This)->lpVtbl -> CeUnmountDBVol(This,pceguid)

#define IRAPISession_CeFlushDBVol(This,pceguid)	\
    (This)->lpVtbl -> CeFlushDBVol(This,pceguid)

#define IRAPISession_CeEnumDBVolumes(This,pceguid,lpBuf,dwNumChars)	\
    (This)->lpVtbl -> CeEnumDBVolumes(This,pceguid,lpBuf,dwNumChars)

#define IRAPISession_CeOidGetInfoEx(This,pceguid,oid,oidInfo)	\
    (This)->lpVtbl -> CeOidGetInfoEx(This,pceguid,oid,oidInfo)

#define IRAPISession_CeSyncStart(This,szCommand)	\
    (This)->lpVtbl -> CeSyncStart(This,szCommand)

#define IRAPISession_CeSyncStop(This)	\
    (This)->lpVtbl -> CeSyncStop(This)

#define IRAPISession_CeQueryInstructionSet(This,dwInstructionSet,lpdwCurrentInstructionSet)	\
    (This)->lpVtbl -> CeQueryInstructionSet(This,dwInstructionSet,lpdwCurrentInstructionSet)

#define IRAPISession_CeGetDiskFreeSpaceEx(This,lpDirectoryName,lpFreeBytesAvailableToCaller,lpTotalNumberOfBytes,lpTotalNumberOfFreeBytes)	\
    (This)->lpVtbl -> CeGetDiskFreeSpaceEx(This,lpDirectoryName,lpFreeBytesAvailableToCaller,lpTotalNumberOfBytes,lpTotalNumberOfFreeBytes)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRAPISession_CeRapiInit_Proxy( 
    IRAPISession * This);


void __RPC_STUB IRAPISession_CeRapiInit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPISession_CeRapiUninit_Proxy( 
    IRAPISession * This);


void __RPC_STUB IRAPISession_CeRapiUninit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD STDMETHODCALLTYPE IRAPISession_CeGetLastError_Proxy( 
    IRAPISession * This);


void __RPC_STUB IRAPISession_CeGetLastError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPISession_CeRapiGetError_Proxy( 
    IRAPISession * This);


void __RPC_STUB IRAPISession_CeRapiGetError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPISession_CeRapiFreeBuffer_Proxy( 
    IRAPISession * This,
    void *Buffer);


void __RPC_STUB IRAPISession_CeRapiFreeBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HANDLE STDMETHODCALLTYPE IRAPISession_CeFindFirstFile_Proxy( 
    IRAPISession * This,
    LPCWSTR FileName,
    LPCE_FIND_DATA FindData);


void __RPC_STUB IRAPISession_CeFindFirstFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeFindNextFile_Proxy( 
    IRAPISession * This,
    HANDLE FoundFile,
    LPCE_FIND_DATA FindData);


void __RPC_STUB IRAPISession_CeFindNextFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeFindClose_Proxy( 
    IRAPISession * This,
    HANDLE FoundFile);


void __RPC_STUB IRAPISession_CeFindClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD STDMETHODCALLTYPE IRAPISession_CeGetFileAttributes_Proxy( 
    IRAPISession * This,
    LPCWSTR FileName);


void __RPC_STUB IRAPISession_CeGetFileAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeSetFileAttributes_Proxy( 
    IRAPISession * This,
    LPCWSTR FileName,
    DWORD FileAttrib);


void __RPC_STUB IRAPISession_CeSetFileAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HANDLE STDMETHODCALLTYPE IRAPISession_CeCreateFile_Proxy( 
    IRAPISession * This,
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDistribution,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile);


void __RPC_STUB IRAPISession_CeCreateFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeReadFile_Proxy( 
    IRAPISession * This,
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped);


void __RPC_STUB IRAPISession_CeReadFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeWriteFile_Proxy( 
    IRAPISession * This,
    HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped);


void __RPC_STUB IRAPISession_CeWriteFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeCloseHandle_Proxy( 
    IRAPISession * This,
    HANDLE hObject);


void __RPC_STUB IRAPISession_CeCloseHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeFindAllFiles_Proxy( 
    IRAPISession * This,
    LPCWSTR Path,
    DWORD Flags,
    LPDWORD pFoundCount,
    LPLPCE_FIND_DATA ppFindDataArray);


void __RPC_STUB IRAPISession_CeFindAllFiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HANDLE STDMETHODCALLTYPE IRAPISession_CeFindFirstDatabase_Proxy( 
    IRAPISession * This,
    DWORD dwDbaseType);


void __RPC_STUB IRAPISession_CeFindFirstDatabase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


CEOID STDMETHODCALLTYPE IRAPISession_CeFindNextDatabase_Proxy( 
    IRAPISession * This,
    HANDLE hEnum);


void __RPC_STUB IRAPISession_CeFindNextDatabase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeOidGetInfo_Proxy( 
    IRAPISession * This,
    CEOID oid,
    CEOIDINFO *oidInfo);


void __RPC_STUB IRAPISession_CeOidGetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


CEOID STDMETHODCALLTYPE IRAPISession_CeCreateDatabase_Proxy( 
    IRAPISession * This,
    LPWSTR lpszName,
    DWORD dwDbaseType,
    WORD cNumSortOrder,
    SORTORDERSPEC *rgSortSpecs);


void __RPC_STUB IRAPISession_CeCreateDatabase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HANDLE STDMETHODCALLTYPE IRAPISession_CeOpenDatabase_Proxy( 
    IRAPISession * This,
    PCEOID poid,
    LPWSTR lpszName,
    CEPROPID propid,
    DWORD dwFlags,
    HWND hwndNotify);


void __RPC_STUB IRAPISession_CeOpenDatabase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeDeleteDatabase_Proxy( 
    IRAPISession * This,
    CEOID oidDbase);


void __RPC_STUB IRAPISession_CeDeleteDatabase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


CEOID STDMETHODCALLTYPE IRAPISession_CeReadRecordProps_Proxy( 
    IRAPISession * This,
    HANDLE hDbase,
    DWORD dwFlags,
    LPWORD lpcPropID,
    CEPROPID *rgPropID,
    LPBYTE *lplpBuffer,
    LPDWORD lpcbBuffer);


void __RPC_STUB IRAPISession_CeReadRecordProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


CEOID STDMETHODCALLTYPE IRAPISession_CeWriteRecordProps_Proxy( 
    IRAPISession * This,
    HANDLE hDbase,
    CEOID oidRecord,
    WORD cPropID,
    CEPROPVAL *rgPropVal);


void __RPC_STUB IRAPISession_CeWriteRecordProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeDeleteRecord_Proxy( 
    IRAPISession * This,
    HANDLE hDatabase,
    CEOID oidRecord);


void __RPC_STUB IRAPISession_CeDeleteRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


CEOID STDMETHODCALLTYPE IRAPISession_CeSeekDatabase_Proxy( 
    IRAPISession * This,
    HANDLE hDatabase,
    DWORD dwSeekType,
    DWORD dwValue,
    LPDWORD lpdwIndex);


void __RPC_STUB IRAPISession_CeSeekDatabase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeSetDatabaseInfo_Proxy( 
    IRAPISession * This,
    CEOID oidDbase,
    CEDBASEINFO *pNewInfo);


void __RPC_STUB IRAPISession_CeSetDatabaseInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD STDMETHODCALLTYPE IRAPISession_CeSetFilePointer_Proxy( 
    IRAPISession * This,
    HANDLE hFile,
    LONG lDistanceToMove,
    PLONG lpDistanceToMoveHigh,
    DWORD dwMoveMethod);


void __RPC_STUB IRAPISession_CeSetFilePointer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeSetEndOfFile_Proxy( 
    IRAPISession * This,
    HANDLE hFile);


void __RPC_STUB IRAPISession_CeSetEndOfFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeCreateDirectory_Proxy( 
    IRAPISession * This,
    LPCWSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes);


void __RPC_STUB IRAPISession_CeCreateDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeRemoveDirectory_Proxy( 
    IRAPISession * This,
    LPCWSTR lpPathName);


void __RPC_STUB IRAPISession_CeRemoveDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeCreateProcess_Proxy( 
    IRAPISession * This,
    LPCWSTR lpszImageName,
    LPCWSTR lpszCmdLine,
    LPSECURITY_ATTRIBUTES lpsaProcess,
    LPSECURITY_ATTRIBUTES lpsaThread,
    BOOL fInheritHandles,
    DWORD fdwCreate,
    LPVOID lpvEnvironment,
    LPWSTR lpszCurDir,
    LPSTARTUPINFOW lpsiStartInfo,
    LPPROCESS_INFORMATION lppiProcInfo);


void __RPC_STUB IRAPISession_CeCreateProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeMoveFile_Proxy( 
    IRAPISession * This,
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName);


void __RPC_STUB IRAPISession_CeMoveFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeCopyFile_Proxy( 
    IRAPISession * This,
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    BOOL bFailIfExists);


void __RPC_STUB IRAPISession_CeCopyFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeDeleteFile_Proxy( 
    IRAPISession * This,
    LPCWSTR FileName);


void __RPC_STUB IRAPISession_CeDeleteFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD STDMETHODCALLTYPE IRAPISession_CeGetFileSize_Proxy( 
    IRAPISession * This,
    HANDLE hFile,
    LPDWORD lpFileSizeHigh);


void __RPC_STUB IRAPISession_CeGetFileSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


LONG STDMETHODCALLTYPE IRAPISession_CeRegOpenKeyEx_Proxy( 
    IRAPISession * This,
    HKEY hKey,
    LPCWSTR lpszSubKey,
    DWORD dwReserved,
    REGSAM samDesired,
    PHKEY phkResult);


void __RPC_STUB IRAPISession_CeRegOpenKeyEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


LONG STDMETHODCALLTYPE IRAPISession_CeRegEnumKeyEx_Proxy( 
    IRAPISession * This,
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpName,
    LPDWORD lpcbName,
    LPDWORD lpReserved,
    LPWSTR lpClass,
    LPDWORD lpcbClass,
    PFILETIME lpftLastWriteTime);


void __RPC_STUB IRAPISession_CeRegEnumKeyEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


LONG STDMETHODCALLTYPE IRAPISession_CeRegCreateKeyEx_Proxy( 
    IRAPISession * This,
    HKEY hKey,
    LPCWSTR lpszSubKey,
    DWORD dwReserved,
    LPWSTR lpszClass,
    DWORD fdwOptions,
    REGSAM samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,
    LPDWORD lpdwDisposition);


void __RPC_STUB IRAPISession_CeRegCreateKeyEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


LONG STDMETHODCALLTYPE IRAPISession_CeRegCloseKey_Proxy( 
    IRAPISession * This,
    HKEY hKey);


void __RPC_STUB IRAPISession_CeRegCloseKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


LONG STDMETHODCALLTYPE IRAPISession_CeRegDeleteKey_Proxy( 
    IRAPISession * This,
    HKEY hKey,
    LPCWSTR lpszSubKey);


void __RPC_STUB IRAPISession_CeRegDeleteKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


LONG STDMETHODCALLTYPE IRAPISession_CeRegEnumValue_Proxy( 
    IRAPISession * This,
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpszValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData);


void __RPC_STUB IRAPISession_CeRegEnumValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


LONG STDMETHODCALLTYPE IRAPISession_CeRegDeleteValue_Proxy( 
    IRAPISession * This,
    HKEY hKey,
    LPCWSTR lpszValueName);


void __RPC_STUB IRAPISession_CeRegDeleteValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


LONG STDMETHODCALLTYPE IRAPISession_CeRegQueryInfoKey_Proxy( 
    IRAPISession * This,
    HKEY hKey,
    LPWSTR lpClass,
    LPDWORD lpcbClass,
    LPDWORD lpReserved,
    LPDWORD lpcSubKeys,
    LPDWORD lpcbMaxSubKeyLen,
    LPDWORD lpcbMaxClassLen,
    LPDWORD lpcValues,
    LPDWORD lpcbMaxValueNameLen,
    LPDWORD lpcbMaxValueLen,
    LPDWORD lpcbSecurityDescriptor,
    PFILETIME lpftLastWriteTime);


void __RPC_STUB IRAPISession_CeRegQueryInfoKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


LONG STDMETHODCALLTYPE IRAPISession_CeRegQueryValueEx_Proxy( 
    IRAPISession * This,
    HKEY hKey,
    LPCWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData);


void __RPC_STUB IRAPISession_CeRegQueryValueEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


LONG STDMETHODCALLTYPE IRAPISession_CeRegSetValueEx_Proxy( 
    IRAPISession * This,
    HKEY hKey,
    LPCWSTR lpValueName,
    DWORD Reserved,
    DWORD dwType,
    BYTE *lpData,
    DWORD cbData);


void __RPC_STUB IRAPISession_CeRegSetValueEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeGetStoreInformation_Proxy( 
    IRAPISession * This,
    LPSTORE_INFORMATION lpsi);


void __RPC_STUB IRAPISession_CeGetStoreInformation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


INT STDMETHODCALLTYPE IRAPISession_CeGetSystemMetrics_Proxy( 
    IRAPISession * This,
    INT nIndex);


void __RPC_STUB IRAPISession_CeGetSystemMetrics_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


INT STDMETHODCALLTYPE IRAPISession_CeGetDesktopDeviceCaps_Proxy( 
    IRAPISession * This,
    INT nIndex);


void __RPC_STUB IRAPISession_CeGetDesktopDeviceCaps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeFindAllDatabases_Proxy( 
    IRAPISession * This,
    DWORD DbaseType,
    WORD Flags,
    LPWORD cFindData,
    LPLPCEDB_FIND_DATA ppFindData);


void __RPC_STUB IRAPISession_CeFindAllDatabases_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IRAPISession_CeGetSystemInfo_Proxy( 
    IRAPISession * This,
    LPSYSTEM_INFO lpSystemInfo);


void __RPC_STUB IRAPISession_CeGetSystemInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeSHCreateShortcut_Proxy( 
    IRAPISession * This,
    LPWSTR lpszShortcut,
    LPWSTR lpszTarget);


void __RPC_STUB IRAPISession_CeSHCreateShortcut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeSHGetShortcutTarget_Proxy( 
    IRAPISession * This,
    LPWSTR lpszShortcut,
    LPWSTR lpszTarget,
    int cbMax);


void __RPC_STUB IRAPISession_CeSHGetShortcutTarget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeCheckPassword_Proxy( 
    IRAPISession * This,
    LPWSTR lpszPassword);


void __RPC_STUB IRAPISession_CeCheckPassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeGetFileTime_Proxy( 
    IRAPISession * This,
    HANDLE hFile,
    LPFILETIME lpCreationTime,
    LPFILETIME lpLastAccessTime,
    LPFILETIME lpLastWriteTime);


void __RPC_STUB IRAPISession_CeGetFileTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeSetFileTime_Proxy( 
    IRAPISession * This,
    HANDLE hFile,
    FILETIME *lpCreationTime,
    FILETIME *lpLastAccessTime,
    FILETIME *lpLastWriteTime);


void __RPC_STUB IRAPISession_CeSetFileTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeGetVersionEx_Proxy( 
    IRAPISession * This,
    LPCEOSVERSIONINFO lpVersionInformation);


void __RPC_STUB IRAPISession_CeGetVersionEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HWND STDMETHODCALLTYPE IRAPISession_CeGetWindow_Proxy( 
    IRAPISession * This,
    HWND hWnd,
    UINT uCmd);


void __RPC_STUB IRAPISession_CeGetWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


LONG STDMETHODCALLTYPE IRAPISession_CeGetWindowLong_Proxy( 
    IRAPISession * This,
    HWND hWnd,
    int nIndex);


void __RPC_STUB IRAPISession_CeGetWindowLong_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


INT STDMETHODCALLTYPE IRAPISession_CeGetWindowText_Proxy( 
    IRAPISession * This,
    HWND hWnd,
    LPWSTR lpString,
    int nMaxCount);


void __RPC_STUB IRAPISession_CeGetWindowText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


INT STDMETHODCALLTYPE IRAPISession_CeGetClassName_Proxy( 
    IRAPISession * This,
    HWND hWnd,
    LPWSTR lpClassName,
    int nMaxCount);


void __RPC_STUB IRAPISession_CeGetClassName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IRAPISession_CeGlobalMemoryStatus_Proxy( 
    IRAPISession * This,
    LPMEMORYSTATUS lpmst);


void __RPC_STUB IRAPISession_CeGlobalMemoryStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeGetSystemPowerStatusEx_Proxy( 
    IRAPISession * This,
    PSYSTEM_POWER_STATUS_EX pstatus,
    BOOL fUpdate);


void __RPC_STUB IRAPISession_CeGetSystemPowerStatusEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD STDMETHODCALLTYPE IRAPISession_CeGetTempPath_Proxy( 
    IRAPISession * This,
    DWORD nBufferLength,
    LPWSTR lpBuffer);


void __RPC_STUB IRAPISession_CeGetTempPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD STDMETHODCALLTYPE IRAPISession_CeGetSpecialFolderPath_Proxy( 
    IRAPISession * This,
    int nFolder,
    DWORD nBufferLength,
    LPWSTR lpBuffer);


void __RPC_STUB IRAPISession_CeGetSpecialFolderPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPISession_CeRapiInvoke_Proxy( 
    IRAPISession * This,
    LPCWSTR pDllPath,
    LPCWSTR pFunctionName,
    DWORD cbInput,
    BYTE *pInput,
    DWORD *pcbOutput,
    BYTE **ppOutput,
    IRAPIStream **ppIRAPIStream,
    DWORD dwReserved);


void __RPC_STUB IRAPISession_CeRapiInvoke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HANDLE STDMETHODCALLTYPE IRAPISession_CeFindFirstDatabaseEx_Proxy( 
    IRAPISession * This,
    PCEGUID pguid,
    DWORD dwDbaseType);


void __RPC_STUB IRAPISession_CeFindFirstDatabaseEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


CEOID STDMETHODCALLTYPE IRAPISession_CeFindNextDatabaseEx_Proxy( 
    IRAPISession * This,
    HANDLE hEnum,
    PCEGUID pguid);


void __RPC_STUB IRAPISession_CeFindNextDatabaseEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


CEOID STDMETHODCALLTYPE IRAPISession_CeCreateDatabaseEx_Proxy( 
    IRAPISession * This,
    PCEGUID pceguid,
    CEDBASEINFO *lpCEDBInfo);


void __RPC_STUB IRAPISession_CeCreateDatabaseEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeSetDatabaseInfoEx_Proxy( 
    IRAPISession * This,
    PCEGUID pceguid,
    CEOID oidDbase,
    CEDBASEINFO *pNewInfo);


void __RPC_STUB IRAPISession_CeSetDatabaseInfoEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HANDLE STDMETHODCALLTYPE IRAPISession_CeOpenDatabaseEx_Proxy( 
    IRAPISession * This,
    PCEGUID pceguid,
    PCEOID poid,
    LPWSTR lpszName,
    CEPROPID propid,
    DWORD dwFlags,
    CENOTIFYREQUEST *pReq);


void __RPC_STUB IRAPISession_CeOpenDatabaseEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeDeleteDatabaseEx_Proxy( 
    IRAPISession * This,
    PCEGUID pceguid,
    CEOID oidDbase);


void __RPC_STUB IRAPISession_CeDeleteDatabaseEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


CEOID STDMETHODCALLTYPE IRAPISession_CeReadRecordPropsEx_Proxy( 
    IRAPISession * This,
    HANDLE hDbase,
    DWORD dwFlags,
    LPWORD lpcPropID,
    CEPROPID *rgPropID,
    LPBYTE *lplpBuffer,
    LPDWORD lpcbBuffer,
    HANDLE hHeap);


void __RPC_STUB IRAPISession_CeReadRecordPropsEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeMountDBVol_Proxy( 
    IRAPISession * This,
    PCEGUID pceguid,
    LPWSTR lpszDBVol,
    DWORD dwFlags);


void __RPC_STUB IRAPISession_CeMountDBVol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeUnmountDBVol_Proxy( 
    IRAPISession * This,
    PCEGUID pceguid);


void __RPC_STUB IRAPISession_CeUnmountDBVol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeFlushDBVol_Proxy( 
    IRAPISession * This,
    PCEGUID pceguid);


void __RPC_STUB IRAPISession_CeFlushDBVol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeEnumDBVolumes_Proxy( 
    IRAPISession * This,
    PCEGUID pceguid,
    LPWSTR lpBuf,
    DWORD dwNumChars);


void __RPC_STUB IRAPISession_CeEnumDBVolumes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeOidGetInfoEx_Proxy( 
    IRAPISession * This,
    PCEGUID pceguid,
    CEOID oid,
    CEOIDINFO *oidInfo);


void __RPC_STUB IRAPISession_CeOidGetInfoEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPISession_CeSyncStart_Proxy( 
    IRAPISession * This,
    LPCWSTR szCommand);


void __RPC_STUB IRAPISession_CeSyncStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPISession_CeSyncStop_Proxy( 
    IRAPISession * This);


void __RPC_STUB IRAPISession_CeSyncStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeQueryInstructionSet_Proxy( 
    IRAPISession * This,
    DWORD dwInstructionSet,
    LPDWORD lpdwCurrentInstructionSet);


void __RPC_STUB IRAPISession_CeQueryInstructionSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRAPISession_CeGetDiskFreeSpaceEx_Proxy( 
    IRAPISession * This,
    LPCWSTR lpDirectoryName,
    ULARGE_INTEGER *lpFreeBytesAvailableToCaller,
    ULARGE_INTEGER *lpTotalNumberOfBytes,
    ULARGE_INTEGER *lpTotalNumberOfFreeBytes);


void __RPC_STUB IRAPISession_CeGetDiskFreeSpaceEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRAPISession_INTERFACE_DEFINED__ */


#ifndef __IRAPIDevice_INTERFACE_DEFINED__
#define __IRAPIDevice_INTERFACE_DEFINED__

/* interface IRAPIDevice */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IRAPIDevice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8a0f1632-3905-4ca4-aea4-7e094ecbb9a7")
    IRAPIDevice : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetConnectStat( 
            /* [out] */ RAPI_DEVICESTATUS *pStat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDeviceInfo( 
            /* [out] */ RAPI_DEVICEINFO *pDevInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConnectionInfo( 
            /* [out] */ RAPI_CONNECTIONINFO *pConnInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateSession( 
            /* [out] */ IRAPISession **ppISession) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRAPIDeviceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRAPIDevice * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRAPIDevice * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRAPIDevice * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetConnectStat )( 
            IRAPIDevice * This,
            /* [out] */ RAPI_DEVICESTATUS *pStat);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeviceInfo )( 
            IRAPIDevice * This,
            /* [out] */ RAPI_DEVICEINFO *pDevInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetConnectionInfo )( 
            IRAPIDevice * This,
            /* [out] */ RAPI_CONNECTIONINFO *pConnInfo);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSession )( 
            IRAPIDevice * This,
            /* [out] */ IRAPISession **ppISession);
        
        END_INTERFACE
    } IRAPIDeviceVtbl;

    interface IRAPIDevice
    {
        CONST_VTBL struct IRAPIDeviceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRAPIDevice_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRAPIDevice_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRAPIDevice_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRAPIDevice_GetConnectStat(This,pStat)	\
    (This)->lpVtbl -> GetConnectStat(This,pStat)

#define IRAPIDevice_GetDeviceInfo(This,pDevInfo)	\
    (This)->lpVtbl -> GetDeviceInfo(This,pDevInfo)

#define IRAPIDevice_GetConnectionInfo(This,pConnInfo)	\
    (This)->lpVtbl -> GetConnectionInfo(This,pConnInfo)

#define IRAPIDevice_CreateSession(This,ppISession)	\
    (This)->lpVtbl -> CreateSession(This,ppISession)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRAPIDevice_GetConnectStat_Proxy( 
    IRAPIDevice * This,
    /* [out] */ RAPI_DEVICESTATUS *pStat);


void __RPC_STUB IRAPIDevice_GetConnectStat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPIDevice_GetDeviceInfo_Proxy( 
    IRAPIDevice * This,
    /* [out] */ RAPI_DEVICEINFO *pDevInfo);


void __RPC_STUB IRAPIDevice_GetDeviceInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPIDevice_GetConnectionInfo_Proxy( 
    IRAPIDevice * This,
    /* [out] */ RAPI_CONNECTIONINFO *pConnInfo);


void __RPC_STUB IRAPIDevice_GetConnectionInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPIDevice_CreateSession_Proxy( 
    IRAPIDevice * This,
    /* [out] */ IRAPISession **ppISession);


void __RPC_STUB IRAPIDevice_CreateSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRAPIDevice_INTERFACE_DEFINED__ */


#ifndef __IRAPIEnumDevices_INTERFACE_DEFINED__
#define __IRAPIEnumDevices_INTERFACE_DEFINED__

/* interface IRAPIEnumDevices */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IRAPIEnumDevices;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("357a557c-b03f-4240-90d8-c6c71c659bf1")
    IRAPIEnumDevices : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
            /* [out] */ IRAPIDevice **ppIDevice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG cElt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IRAPIEnumDevices **ppIEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCount( 
            /* [out] */ ULONG *pcElt) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRAPIEnumDevicesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRAPIEnumDevices * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRAPIEnumDevices * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRAPIEnumDevices * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IRAPIEnumDevices * This,
            /* [out] */ IRAPIDevice **ppIDevice);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IRAPIEnumDevices * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IRAPIEnumDevices * This,
            /* [in] */ ULONG cElt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IRAPIEnumDevices * This,
            /* [out] */ IRAPIEnumDevices **ppIEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            IRAPIEnumDevices * This,
            /* [out] */ ULONG *pcElt);
        
        END_INTERFACE
    } IRAPIEnumDevicesVtbl;

    interface IRAPIEnumDevices
    {
        CONST_VTBL struct IRAPIEnumDevicesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRAPIEnumDevices_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRAPIEnumDevices_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRAPIEnumDevices_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRAPIEnumDevices_Next(This,ppIDevice)	\
    (This)->lpVtbl -> Next(This,ppIDevice)

#define IRAPIEnumDevices_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IRAPIEnumDevices_Skip(This,cElt)	\
    (This)->lpVtbl -> Skip(This,cElt)

#define IRAPIEnumDevices_Clone(This,ppIEnum)	\
    (This)->lpVtbl -> Clone(This,ppIEnum)

#define IRAPIEnumDevices_GetCount(This,pcElt)	\
    (This)->lpVtbl -> GetCount(This,pcElt)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRAPIEnumDevices_Next_Proxy( 
    IRAPIEnumDevices * This,
    /* [out] */ IRAPIDevice **ppIDevice);


void __RPC_STUB IRAPIEnumDevices_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPIEnumDevices_Reset_Proxy( 
    IRAPIEnumDevices * This);


void __RPC_STUB IRAPIEnumDevices_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPIEnumDevices_Skip_Proxy( 
    IRAPIEnumDevices * This,
    /* [in] */ ULONG cElt);


void __RPC_STUB IRAPIEnumDevices_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPIEnumDevices_Clone_Proxy( 
    IRAPIEnumDevices * This,
    /* [out] */ IRAPIEnumDevices **ppIEnum);


void __RPC_STUB IRAPIEnumDevices_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPIEnumDevices_GetCount_Proxy( 
    IRAPIEnumDevices * This,
    /* [out] */ ULONG *pcElt);


void __RPC_STUB IRAPIEnumDevices_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRAPIEnumDevices_INTERFACE_DEFINED__ */


#ifndef __IRAPISink_INTERFACE_DEFINED__
#define __IRAPISink_INTERFACE_DEFINED__

/* interface IRAPISink */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IRAPISink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b4fd053e-4810-46db-889b-20e638e334f0")
    IRAPISink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnDeviceConnected( 
            /* [in] */ IRAPIDevice *pIDevice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnDeviceDisconnected( 
            /* [in] */ IRAPIDevice *pIDevice) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRAPISinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRAPISink * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRAPISink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRAPISink * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnDeviceConnected )( 
            IRAPISink * This,
            /* [in] */ IRAPIDevice *pIDevice);
        
        HRESULT ( STDMETHODCALLTYPE *OnDeviceDisconnected )( 
            IRAPISink * This,
            /* [in] */ IRAPIDevice *pIDevice);
        
        END_INTERFACE
    } IRAPISinkVtbl;

    interface IRAPISink
    {
        CONST_VTBL struct IRAPISinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRAPISink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRAPISink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRAPISink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRAPISink_OnDeviceConnected(This,pIDevice)	\
    (This)->lpVtbl -> OnDeviceConnected(This,pIDevice)

#define IRAPISink_OnDeviceDisconnected(This,pIDevice)	\
    (This)->lpVtbl -> OnDeviceDisconnected(This,pIDevice)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRAPISink_OnDeviceConnected_Proxy( 
    IRAPISink * This,
    /* [in] */ IRAPIDevice *pIDevice);


void __RPC_STUB IRAPISink_OnDeviceConnected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPISink_OnDeviceDisconnected_Proxy( 
    IRAPISink * This,
    /* [in] */ IRAPIDevice *pIDevice);


void __RPC_STUB IRAPISink_OnDeviceDisconnected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRAPISink_INTERFACE_DEFINED__ */


#ifndef __IRAPIDesktop_INTERFACE_DEFINED__
#define __IRAPIDesktop_INTERFACE_DEFINED__

/* interface IRAPIDesktop */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IRAPIDesktop;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("dcbeb807-14d0-4cbd-926c-b991f4fd1b91")
    IRAPIDesktop : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FindDevice( 
            /* [in] */ RAPIDEVICEID *pDeviceID,
            /* [in] */ RAPI_GETDEVICEOPCODE opFlags,
            /* [out] */ IRAPIDevice **ppIDevice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumDevices( 
            /* [out] */ IRAPIEnumDevices **ppIEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Advise( 
            /* [in] */ IRAPISink *pISink,
            /* [out] */ DWORD_PTR *pdwContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnAdvise( 
            /* [in] */ DWORD_PTR dwContext) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRAPIDesktopVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRAPIDesktop * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRAPIDesktop * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRAPIDesktop * This);
        
        HRESULT ( STDMETHODCALLTYPE *FindDevice )( 
            IRAPIDesktop * This,
            /* [in] */ RAPIDEVICEID *pDeviceID,
            /* [in] */ RAPI_GETDEVICEOPCODE opFlags,
            /* [out] */ IRAPIDevice **ppIDevice);
        
        HRESULT ( STDMETHODCALLTYPE *EnumDevices )( 
            IRAPIDesktop * This,
            /* [out] */ IRAPIEnumDevices **ppIEnum);
        
        HRESULT ( STDMETHODCALLTYPE *Advise )( 
            IRAPIDesktop * This,
            /* [in] */ IRAPISink *pISink,
            /* [out] */ DWORD_PTR *pdwContext);
        
        HRESULT ( STDMETHODCALLTYPE *UnAdvise )( 
            IRAPIDesktop * This,
            /* [in] */ DWORD_PTR dwContext);
        
        END_INTERFACE
    } IRAPIDesktopVtbl;

    interface IRAPIDesktop
    {
        CONST_VTBL struct IRAPIDesktopVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRAPIDesktop_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRAPIDesktop_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRAPIDesktop_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRAPIDesktop_FindDevice(This,pDeviceID,opFlags,ppIDevice)	\
    (This)->lpVtbl -> FindDevice(This,pDeviceID,opFlags,ppIDevice)

#define IRAPIDesktop_EnumDevices(This,ppIEnum)	\
    (This)->lpVtbl -> EnumDevices(This,ppIEnum)

#define IRAPIDesktop_Advise(This,pISink,pdwContext)	\
    (This)->lpVtbl -> Advise(This,pISink,pdwContext)

#define IRAPIDesktop_UnAdvise(This,dwContext)	\
    (This)->lpVtbl -> UnAdvise(This,dwContext)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRAPIDesktop_FindDevice_Proxy( 
    IRAPIDesktop * This,
    /* [in] */ RAPIDEVICEID *pDeviceID,
    /* [in] */ RAPI_GETDEVICEOPCODE opFlags,
    /* [out] */ IRAPIDevice **ppIDevice);


void __RPC_STUB IRAPIDesktop_FindDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPIDesktop_EnumDevices_Proxy( 
    IRAPIDesktop * This,
    /* [out] */ IRAPIEnumDevices **ppIEnum);


void __RPC_STUB IRAPIDesktop_EnumDevices_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPIDesktop_Advise_Proxy( 
    IRAPIDesktop * This,
    /* [in] */ IRAPISink *pISink,
    /* [out] */ DWORD_PTR *pdwContext);


void __RPC_STUB IRAPIDesktop_Advise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRAPIDesktop_UnAdvise_Proxy( 
    IRAPIDesktop * This,
    /* [in] */ DWORD_PTR dwContext);


void __RPC_STUB IRAPIDesktop_UnAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRAPIDesktop_INTERFACE_DEFINED__ */



#ifndef __RAPILib_LIBRARY_DEFINED__
#define __RAPILib_LIBRARY_DEFINED__

/* library RAPILib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_RAPILib;

EXTERN_C const CLSID CLSID_RAPI;

#ifdef __cplusplus

class DECLSPEC_UUID("35440327-1517-4B72-865E-3FFE8E97002F")
RAPI;
#endif
#endif /* __RAPILib_LIBRARY_DEFINED__ */

/* interface __MIDL_itf_RAPI2_0268 */
/* [local] */ 

#endif


extern RPC_IF_HANDLE __MIDL_itf_RAPI2_0268_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_RAPI2_0268_v0_0_s_ifspec;

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


