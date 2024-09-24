
#if !defined(_circlebuffer_h)
#define _circlebuffer_h

#include <stdlib.h>

#if defined(__cplusplus)
extern "C" {
#endif


#if defined(_WIN32)
	typedef __int64 int64_t;
#elif defined(__POWERPC) || defined(__APPLE)
    #include <ppc/types.h>
#else
    typedef long long int64_t;
#endif

#if !defined(_WIN32)
#pragma bool on
#endif



typedef unsigned char CircleRecord_t;



typedef void (*FuncLock_t)()   ;

/* assume that assert, alerts, messages to go off before this ever is allowed to fill */
/*------------------------------------------------------------------------------------*/
typedef struct CircleBuf_tt
{
	size_t				head;           /* points to start of usable data in buffer */
	size_t 				count;
	size_t				bufSize;
	int64_t				bytesConsumed;
	size_t				recordSize;
	size_t				userData;       /* might store actual recordsize */
	int					balance;
	CircleRecord_t*		buffer;         /* 10 seconds of 16 bit stereo nice quality */
	unsigned char* 		maxChunk;
	size_t				maxChunkLen;
	int 				percent;        /* level where buffer considered stable */
	int					wrapped;        /* non-zero if data has wrapped at least once */
	int                 muted;
	
	FuncLock_t          lock;           /* in case there could be competition for any members */
	FuncLock_t          unlock;         /* in case there could be competition for any members */
	
	int 				starvedBytes;    /* how many bytes we had to "conjure up" because we were empty (debug) */
	int 				starvedRequests;  /* how many request we honored when we have been in a starved state (debug) */
	
} CircleBuffer_t;


void testCircleBuffer(void);
void destroyCircleBuffer(CircleBuffer_t* cb);
int initCircleBuffer(CircleBuffer_t* cb, size_t size, int percent, size_t maxChunk, FuncLock_t lock, FuncLock_t unlock);
int addToCircleBuffer(CircleBuffer_t* cb, void* data, size_t count);
int readFromCircleBuffer(CircleBuffer_t* cb, void* dest, size_t count);
int accessCircleBuffer(CircleBuffer_t* cb, void* dest, size_t count);
void FreeWrapless(const CircleBuffer_t* cb, void* handle, size_t* sizeWrapless);
int resetCircleBuffer(CircleBuffer_t* cb);
int RewindBuffer(CircleBuffer_t* cb, int64_t len);
int ForwardBuffer(CircleBuffer_t* cb, int64_t len);



void CircleReport(const CircleBuffer_t* cb, const char* title);



int CirclePercent(CircleBuffer_t* cb);

int CircleAtLevel(CircleBuffer_t* cb);

int CircleOverLevel(CircleBuffer_t* cb);



typedef enum {
        CB_NOERR        = 0,    /* OK */
        CB_FULL         = -1,   /* Buffer overflow */
        CB_MAX_LEVEL    = -2,   /* Buffer is over target full level (percent) */
        CB_MIN_LEVEL    = -3,   /* Buffer is under target min level (percent) */
        CB_EMPTY        = -4    /* Buffer is empty */
} CB_Err_t;
#if defined(__cplusplus)
}
#endif



#endif


