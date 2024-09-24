#ifndef ON2VPPLUGIN_H
#define ON2VPPLUGIN_H

#ifdef ON2VPPLUGIN_EXPORTS
#define ON2VPPLUGIN_API __declspec(dllexport) WINAPI
#else
#define ON2VPPLUGIN_API __declspec(dllimport) WINAPI
#endif


BOOL    ON2VPPLUGIN_API  ON2Info(DWORD fccType, DWORD fccHandler, ICINFO FAR * lpicinfo);
BOOL    ON2VPPLUGIN_API  ON2Install(DWORD fccType, DWORD fccHandler, LPARAM lParam, LPSTR szDesc, UINT wFlags);
BOOL    ON2VPPLUGIN_API  ON2Remove(DWORD fccType, DWORD fccHandler, UINT wFlags);
LRESULT ON2VPPLUGIN_API  ON2GetInfo(HIC hic, ICINFO FAR *picinfo, DWORD cb);
HIC     ON2VPPLUGIN_API  ON2Open(DWORD fccType, DWORD fccHandler, UINT wMode);
HIC     ON2VPPLUGIN_API  ON2OpenFunction(DWORD fccType, DWORD fccHandler, UINT wMode, FARPROC lpfnHandler);
LRESULT ON2VPPLUGIN_API  ON2Close(HIC hic);
LRESULT ON2VPPLUGIN_API  ON2SendMessage(HIC hic, UINT msg, DWORD dw1, DWORD dw2);;
LRESULT ON2VPPLUGIN_API  ON2QueryAbout(HIC hic) ;
LRESULT ON2VPPLUGIN_API  ON2About(HIC hic,  HWND hwnd) ;
LRESULT ON2VPPLUGIN_API  ON2QueryConfigure(HIC hic) ;
LRESULT ON2VPPLUGIN_API  ON2Configure(HIC hic,  HWND hwnd) ;
LRESULT ON2VPPLUGIN_API  ON2GetState(HIC hic,  LPVOID pv, DWORD cb) ;
LRESULT ON2VPPLUGIN_API  ON2SetState(HIC hic,  LPVOID pv, DWORD cb) ;
LRESULT ON2VPPLUGIN_API  ON2GetStateSize(HIC hic) ;
LRESULT ON2VPPLUGIN_API  ON2GetDefaultQuality(HIC hic) ;
LRESULT ON2VPPLUGIN_API  ON2GetDefaultKeyFrameRate(HIC hic) ;
LRESULT ON2VPPLUGIN_API  ON2CompressBegin(HIC hic,  BITMAPINFO *lpbiInput, BITMAPINFO *lpbiOutput) ;
LRESULT ON2VPPLUGIN_API  ON2CompressQuery(HIC hic,  BITMAPINFO *lpbiInput, BITMAPINFO *lpbiOutput) ;
LRESULT ON2VPPLUGIN_API  ON2CompressGetFormat(HIC hic,  BITMAPINFO *lpbiInput, BITMAPINFO *lpbiOutput) ;
LRESULT ON2VPPLUGIN_API  ON2CompressGetFormatSize(HIC hic,  BITMAPINFO *lpbi) ;
LRESULT ON2VPPLUGIN_API  ON2CompressGetSize(HIC hic,  BITMAPINFO *lpbiInput, BITMAPINFO *lpbiOutput) ;
LRESULT ON2VPPLUGIN_API  ON2CompressEnd(HIC hic) ;
LRESULT ON2VPPLUGIN_API  ON2DecompressBegin(HIC hic,  BITMAPINFO *lpbiInput, BITMAPINFO *lpbiOutput) ;
LRESULT ON2VPPLUGIN_API  ON2DecompressQuery(HIC hic,  BITMAPINFO *lpbiInput, BITMAPINFO *lpbiOutput) ;
LRESULT ON2VPPLUGIN_API  ON2DecompressGetFormat(HIC hic,  BITMAPINFO *lpbiInput, BITMAPINFO *lpbiOutput) ;
LRESULT ON2VPPLUGIN_API  ON2DecompressGetFormatSize(HIC hic,  BITMAPINFO *lpbi) ;
LRESULT ON2VPPLUGIN_API  ON2DecompressGetPalette(HIC hic,  BITMAPINFO *lpbiInput, BITMAPINFO *lpbiOutput) ;
LRESULT ON2VPPLUGIN_API  ON2DecompressSetPalette(HIC hic,  BITMAPINFO *lpbiPalette) ;
LRESULT ON2VPPLUGIN_API  ON2DecompressEnd(HIC hic) ;
LRESULT ON2VPPLUGIN_API  ON2DecompressExEnd(HIC hic) ;
LRESULT ON2VPPLUGIN_API  ON2DecompressEx(HIC hic,DWORD dwFlags,LPBITMAPINFOHEADER lpbiSrc,LPVOID lpSrc,int xSrc,int ySrc,int dxSrc,int dySrc,LPBITMAPINFOHEADER lpbiDst,LPVOID lpDst,int xDst,int yDst,int dxDst,int dyDst);
LRESULT ON2VPPLUGIN_API  ON2DecompressExBegin(HIC hic,DWORD dwFlags,LPBITMAPINFOHEADER lpbiSrc,LPVOID lpSrc,int xSrc,int ySrc,int dxSrc,int dySrc,LPBITMAPINFOHEADER lpbiDst,LPVOID lpDst,int xDst,int yDst,int dxDst,int dyDst);
LRESULT ON2VPPLUGIN_API  ON2DecompressExQuery(HIC hic,DWORD dwFlags,LPBITMAPINFOHEADER lpbiSrc,LPVOID lpSrc,int xSrc,int ySrc,int dxSrc,int dySrc,LPBITMAPINFOHEADER lpbiDst,LPVOID lpDst,int xDst,int yDst,int dxDst,int dyDst);
HIC     ON2VPPLUGIN_API  ON2Open(DWORD fccType, DWORD fccHandler, UINT wMode);
LRESULT ON2VPPLUGIN_API  ON2Close(HIC hic);
DWORD   ON2VPPLUGIN_API  ON2Decompress(HIC hic,DWORD dwFlags, LPBITMAPINFOHEADER  lpbiInput, LPVOID  lpInput, LPBITMAPINFOHEADER  lpbiOutput,LPVOID lpOutput);
DWORD   ON2VPPLUGIN_API  ON2Compress(HIC hic,DWORD dwFlags,LPBITMAPINFOHEADER lpbiOutput,LPVOID lpOutput,LPBITMAPINFOHEADER lpbiInput,LPVOID lpInput,LPDWORD lpckid,LPDWORD lpdwFlags,LONG lFrameNum,DWORD dwFrameSize,DWORD dwQuality,LPBITMAPINFOHEADER lpbiPrev,LPVOID lpPrev);
LRESULT ON2VPPLUGIN_API  ON2SendMessage(HIC hic, UINT msg, DWORD dw1, DWORD dw2);
LRESULT ON2VPPLUGIN_API  ON2SetReference(HIC hic, LPBITMAPINFO lpbiInput, char * buffer);
LRESULT ON2VPPLUGIN_API  ON2SetRecoveryFrame(HIC hic);
LRESULT ON2VPPLUGIN_API  ON2SetInternalSize(HIC hic,int wr, int ws, int hr, int hs );
LRESULT ON2VPPLUGIN_API  ON2SendMessage(HIC hic, UINT msg, DWORD dw1, DWORD dw2);
LRESULT ON2VPPLUGIN_API  ON2GetReference(HIC hic, LPBITMAPINFO lpbiInput, char * buffer);
LRESULT ON2VPPLUGIN_API  ON2SetCPUFree(HIC hic, int cpuFree);


#endif