#ifndef NULLSOFT_FUNCTYPEDEFSH
#define NULLSOFT_FUNCTYPEDEFSH


typedef unsigned char * PBYTE;
// typedef UINT (CALLBACK* LPFNDLLFUNC1)(DWORD,UINT);

// we Need all these as a minimum:
// 1 arg
// typedef DWORD (CALLBACK * LPFNDLLFUNC1)(DWORD);
typedef DWORD (CALLBACK * NoArgCallback)();
// DWORD WINAPI PrimoSDK_End(VOID);


typedef DWORD (CALLBACK * OneArgCallback)(DWORD);
/*
DWORD  PrimoSDK_Trace(DWORD dwTrace);
DWORD  PrimoSDK_CloseImage(DWORD dwHandle);
DWORD  PrimoSDK_ReleaseHandle(DWORD dwHandle);
*/ 
// typedef DWORD (CALLBACK1a * )(PDWORD);
typedef DWORD (CALLBACK * OneArgCallback2)(PDWORD);
/*
DWORD  PrimoSDK_Init(PDWORD pdwRelease);
DWORD  PrimoSDK_GetHandle(PDWORD pdwHandle);
*/ 
// 2
typedef DWORD (CALLBACK * TwoArgCallback)(DWORD, PBYTE);
// DWORD  PrimoSDK_AddFolder (DWORD dwHandle, PBYTE szFolder);
// typedef DWORD (CALLBACK2a)(DWORD, DWORD);
typedef DWORD (CALLBACK * TwoArgCallback2)(DWORD, DWORD);
// DWORD  PrimoSDK_VerifyImage(DWORD dwHandle, DWORD dwSpeed);
typedef DWORD (CALLBACK * TwoArgCallback3)(DWORD, PDWORD);
// 3
// typedef DWORD (CALLBACK3  )
typedef DWORD (CALLBACK * ThreeArgCallback)(DWORD, PDWORD, DWORD);

//   DWORD WINAPI PrimoSDK_UnitLock(DWORD dwHandle, PDWORD pdwUnit, DWORD dwFlags);
/*
DWORD  PrimoSDK_EraseMedium(DWORD dwHandle, PDWORD pdwUnit, DWORD dwFlags);
DWORD  PrimoSDK_UnitAIN(DWORD dwHandle, PDWORD pdwUnit, DWORD dwFlags);
DWORD  PrimoSDK_MoveMedium(DWORD dwHandle, PDWORD pdwUnit, DWORD dwFlags);
*/ 
// typedef DWORD (CALLBACK3b  )(DWORD, PBYTE, PBYTE);
typedef DWORD (CALLBACK * ThreeArgCallback2 )(DWORD, PBYTE, PBYTE);
// DWORD  PrimoSDK_AddFile(DWORD dwHandle, PBYTE szFileOnCD, PBYTE szSourceFile);

typedef DWORD (CALLBACK * ThreeArgCallback3 )(DWORD, DWORD, DWORD);

// 4
typedef DWORD (CALLBACK * FourArgCallback)(DWORD, PDWORD, DWORD, PBYTE);
// DWORD  PrimoSDK_UnitVxBlock(DWORD dwHandle, PDWORD pdwUnit, DWORD dwFlags, PBYTE szAppName);
typedef DWORD (CALLBACK * FourArgCallback2 )( DWORD, DWORD, DWORD, PDWORD);
// DWORD  PrimoSDK_WriteImage(DWORD dwHandle, DWORD dwFlags, DWORD dwSpeed, PDWORD pdwSize);
// 5
typedef DWORD (CALLBACK * FourArgCallback3 )( DWORD, DWORD, PDWORD, PDWORD);
// DWORD WINAPI PrimoSDK_RunningStatus(DWORD dwHandle, DWORD dwFlags,PDWORD pdwCurSector, PDWORD pdwTotSector);

typedef DWORD (CALLBACK *FourArgCallback4)( DWORD, PBYTE, DWORD, PDWORD);

// typedef DWORD (CALLBACK5  )( DWORD, PDWORD, PDWORD, PBYTE, PDWORD);
typedef DWORD (CALLBACK * FiveArgCallback)( DWORD, PDWORD, PDWORD, PBYTE, PDWORD);
// DWORD  PrimoSDK_UnitInfo(DWORD dwHandle, PDWORD pdwUnit, PDWORD pdwType, PBYTE szDescr, PDWORD pdwReady);

//typedef DWORD (CALLBACK5b  )( DWORD, PDWORD, PDWORD, PDWORD, PDWORD);
typedef DWORD (CALLBACK * FiveArgCallback2)( DWORD, PDWORD, PDWORD, PDWORD, PDWORD);
// DWORD  PrimoSDK_UnitSpeeds(DWORD dwHandle, PDWORD pdwUnit, PDWORD pdwCDSpeeds, PDWORD pdwDVDSpeeds, PDWORD pdwCapabilities);
typedef DWORD (CALLBACK * FiveArgCallback3 )( DWORD, PBYTE, DWORD, PDWORD, PDWORD);
// DWORD WINAPI PrimoSDK_NextExtractAudioBuffer(DWORD, PBYTE, DWORD, PDWORD, PDWORD);
// 6
//typedef DWORD (CALLBACK6  ) (DWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD);
typedef DWORD (CALLBACK * SixArgCallback) (DWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD);
// DWORD  PrimoSDK_UnitInfo2(DWORD dwHandle, PDWORD pdwUnit, PDWORD pdwTypes, PDWORD pdwClass, PDWORD pdwBusType, PDWORD pdwRFU);
// 7
// typedef DWORD (CALLBACK7  )( DWORD, PDWORD, PBYTE, DWORD, DWORD, DWORD, PBYTE);
typedef DWORD (CALLBACK * SevenArgCallback)( DWORD, PDWORD, PBYTE, DWORD, DWORD, DWORD, PBYTE);
// DWORD  PrimoSDK_NewImage(DWORD dwHandle, PDWORD pdwUnits, PBYTE szVolumeName, DWORD dwTrackToLoad, DWORD dwFlags, DWORD dwSwapThreshold, PBYTE szTemp);
typedef DWORD (CALLBACK * SevenArgCallback2)( DWORD, DWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD);
// DWORD WINAPI PrimoSDK_TrackInfo(DWORD, DWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD);
typedef DWORD (CALLBACK * SevenArgCallback3)( DWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD);
// DWORD WINAPI PrimoSDK_DiscInfo2(DWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD);

// 8
//typedef DWORD (CALLBACK8  )( DWORD, PDWORD,  PDWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD);
typedef DWORD (CALLBACK * EightArgCallback )( DWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD);
// DWORD  PrimoSDK_DiscInfo(DWORD dwHandle, PDWORD pdwUnit,  PDWORD pdwMediumType, PDWORD pdwMediumFormat, PDWORD pdwErasable, PDWORD pdwTracks, PDWORD pdwUsed, PDWORD pdwFree);

typedef DWORD (CALLBACK * EightArgCallback2 )(DWORD, PDWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);
// DWORD WINAPI PrimoSDK_ExtractAudioToBuffer(DWORD, PDWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);

typedef DWORD (CALLBACK * EightArgCallback3)(DWORD, PDWORD, PBYTE, PBYTE, PBYTE, PBYTE, PBYTE, PBYTE);

typedef DWORD (CALLBACK * NineArgCallback)(DWORD,PDWORD,DWORD,PDWORD,PDWORD,PDWORD,PDWORD,PDWORD,PDWORD);
#endif