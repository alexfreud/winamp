/****************************************************************************

	Entry for "C" calls

****************************************************************************/
#ifndef WCOMPC_H
#define WCOMPC_H


#ifndef YUVINPUTBUFFERCONFIG
#define YUVINPUTBUFFERCONFIG
typedef struct
{
    int     YWidth;
    int     YHeight;
    int     YStride;

    int     UVWidth;
    int     UVHeight;
    int     UVStride;

    char *  YBuffer;
    char *  UBuffer;
    char *  VBuffer;

} YUV_INPUT_BUFFER_CONFIG;
#endif

#ifdef __cplusplus
extern "C" 
{
#endif

	void NewWC(void **wc);

	void DeleteWC(void **wc);

	int BeginCompressWC(
			void *wc,
			int ScreenWidth,
			int ScreenHeight,
			int Width,
			int Height,
			int XOffset,
			int YOffset);

	
	int CompressYUVWC(void *wc,
			YUV_INPUT_BUFFER_CONFIG *YuvInputData,
			unsigned char *OutputBufferPtr,
			unsigned char *ReconBufferPtr,
			int TargetSize);

	int CompressWC(void *wc,
			 unsigned char *InputData,
			 unsigned char *OutputBufferPtr,
			 unsigned char *ReconBufferPtr,
			 int TargetSize);

	int AnalyzeWC(void *wc,
			 unsigned char *InputData);


	void EndCompressWC(void *wc);


	int BeginDecompressWC(void *wc,
					  int ScreenWidth,
					  int ScreenHeight,
					  int Width,
					  int Height,
					  int XOffset,
					  int YOffset);

	int DecompressWC(void *wc,
			   unsigned char *InputBufferPtr,
			   unsigned char *OutputBufferPtr);


	void EndDecompressWC(void *wc);
#ifdef __cplusplus

	}
#endif

#endif