

#include "circlebuffer.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> /* memory copy */
#include "duck_mem.h"


/* this is just a debugging trick so that we can "see" the free space */
void* circleread_memcpy(void* dst, void* src, int64_t count);
void* circleread_memcpy(void* dst, void* src, int64_t count)
{
	return duck_memcpy64(dst, src, count);
}



void CircleReport(const CircleBuffer_t* cb, const char* title)
{
    printf("-----(%s)------\n", title);
    printf("max Size cb = %ld\n", cb->bufSize);
    printf("fills at = %ld\n", cb->bufSize * cb->percent / 100 );
    printf("Current amount = %ld, level = %ld\n", cb->count, cb->count * 100 / cb->bufSize);
}




int ForwardBuffer(CircleBuffer_t* cb, int64_t len)
{
    if (len >= (int64_t)cb->count)
        return -1;
		
	if (  (cb->head + len) < cb->bufSize  )
	    cb->head += (int)len;
		
	else
	    cb->head = (int)len - (cb->bufSize - cb->head);
	
	cb->count -= (int)len;
		
	return 0;
}






int RewindBuffer(CircleBuffer_t* cb, int64_t len)
{
    if (len >= (int64_t)(cb->bufSize - cb->count) )
	    return -1; /* not enough history in buffer ! */

	if (cb->head <= (size_t)len)
	{
	    if (cb->wrapped == 0) 
		    return -1;
			
	    cb->head = cb->bufSize - ((int)len - cb->head); 
		cb->count += (int)len;
	    return 0;
	}
	else
	{
	    cb->head -= (int)len;
	    cb->count += (int)len;
	}
	
	return 0;
}




void destroyCircleBuffer(CircleBuffer_t* cb)
{
	assert(cb);
	if (cb->buffer)
		free(cb->buffer);
		
	if (cb->maxChunk)
		free(cb->maxChunk);
}


int resetCircleBuffer(CircleBuffer_t* cb)
{   
    cb->count = 0;
	cb->bytesConsumed = 0;
	cb->wrapped = 0;                
	cb->starvedBytes = 0;  
	cb->starvedRequests = 0;  
	
	return 0;
}



int initCircleBuffer(
    CircleBuffer_t* cb, 
    size_t countRecords, 
    int percent, 
    size_t maxChunk,
    FuncLock_t lock,
    FuncLock_t unlock
)
{
	assert(cb);
	
	cb->buffer = (unsigned char * ) calloc(1, countRecords);
	
	cb->maxChunk = (unsigned char *) calloc(1, maxChunk);
	cb->maxChunkLen = maxChunk;
	
	if (cb->buffer)
	{
		cb->head = cb->count = 0;
		cb->balance = 0;
		cb->bufSize = countRecords;
		cb->bytesConsumed = 0;
		cb->muted = false;
		cb->percent = percent;
		cb->wrapped = 0;
		cb->lock = lock;
		cb->unlock = unlock;
		return 0;
	}
	else
	{
		return -1; /* error */
	}
	
}



/* return zero if plenty of room and success */
/*-------------------------------------------*/
/* free space nested in the middle of the buffer is consider endSpace */
/* and when free space nested in middle, startSpace is considered to be zero */
/*---------------------------------------------------------------------------*/
int addToCircleBuffer(CircleBuffer_t* cb, void* data, size_t requestSpace)
{
	int64_t freeSpace;				/* count total free space in buffer */
	int64_t head = cb->head;			/* offset start of valid data */
	int64_t tail = (cb->head + cb->count) % cb->bufSize;			/* offest first free byte after valid data */
	int64_t endSpace;

	freeSpace = cb->bufSize - cb->count;
	
	/* if not enough room to do the add */
	/*----------------------------------*/
	if (requestSpace > freeSpace)
	{
	    assert(0);
		return CB_FULL;
	}

	
	endSpace = cb->bufSize - tail;
	
	if (tail >= head && requestSpace > endSpace)  /* additional data write will wrap  */
	{
			duck_memcpy64(&cb->buffer[tail], data, endSpace);
			duck_memcpy64(
            cb->buffer,
            (unsigned char *)data+endSpace,
            requestSpace - endSpace);
	}
	else /* existing data wrapped around from end of buffer through beginning of buffer. */
	{
		memcpy(&cb->buffer[tail], data, requestSpace);
	}
	
	cb->count 		+= requestSpace;
	cb->balance     += 1;

	return 0;   /* -1 will mean error,m  zero is OK  */
}

/* get info need so we can write direct as in memcpy into the circle buffer */
/*--------------------------------------------------------------------------*/
void FreeWrapless(const CircleBuffer_t* cb, void* handle, int64_t* sizeWrapless)
{
    int64_t tail = 			(cb->head + cb->count) % cb->bufSize;
    
    if ((cb->head + cb->count) < cb->bufSize)
    {
        *((void **) handle) = &cb->buffer[tail];
        *sizeWrapless =  (cb->bufSize -(cb->head + cb->count));
    }
    else
    {
        *((void **) handle) = &cb->buffer[tail];
        *sizeWrapless =  (cb->bufSize - cb->count);
    }
}



/* Please clone this sucker from readFromCircleBuffer */
int accessCircleBuffer(CircleBuffer_t* cb, void* handle1, size_t requestSize)
{
	int64_t head = 			cb->head;
	int64_t tail = 			(cb->head + cb->count) % cb->bufSize;
	void** handle = 		(void **) handle1;
	void*  dest = 			*handle;
		
	if (requestSize <= 0)
	{
		return requestSize;
	}

	if (cb->count < requestSize)
	{
		return -1;
	}
	
	
	if (tail > head)  /* the data does not wrap ! */
	{
			*handle = &cb->buffer[head];
	}
	else    /* the current data does wrap */
	{
		/* but our read does not wrap */
		if (head + requestSize < cb->bufSize)
		{
			*handle = &cb->buffer[head];
		}
		else if (head + requestSize == cb->bufSize)
		{
			*handle = &cb->buffer[head];
		}
		else  /* our read will wrap ! */
		{
			int64_t temp = cb->bufSize - head;
			dest = cb->maxChunk;
			
			assert(cb->maxChunkLen >= requestSize);
			
			circleread_memcpy(
                dest,
                &cb->buffer[head],
                temp);
			circleread_memcpy(
                ((unsigned char *) dest) + temp,
                cb->buffer,
                requestSize - temp);
            *handle = dest;
		}
	}
	
	cb->head = (cb->head + requestSize) % cb->bufSize;
	cb->count -= requestSize;

	cb->bytesConsumed += requestSize;
	cb->balance -= 1;
	
	return requestSize;  /* records (16 bit or maybe other in future) */
}






/* return count read , or -1 if not enough data */
/*----------------------------------------------*/
int readFromCircleBuffer(CircleBuffer_t* cb, void* dest, size_t requestSize)
{
	int64_t head = 			cb->head;
	int64_t tail = 			(cb->head + cb->count) % cb->bufSize;
		
	if (cb->count < requestSize)
	{
		requestSize = cb->count; /* Give them what we have */
	}
		
	if (requestSize <= 0)
	{
		return (int)requestSize;
	}
	
	if (tail > head)  /* the data does not wrap ! */
	{
			circleread_memcpy(dest, &cb->buffer[head], requestSize);
	}
	else    /* the current data does wrap */
	{
		/* but our read does not wrap */
		if (head + requestSize < cb->bufSize)
		{
			circleread_memcpy(dest, &cb->buffer[head], requestSize);
		}
		else if (head + requestSize == cb->bufSize)
		{
			circleread_memcpy(dest, &cb->buffer[head], requestSize);
			memset(&cb->buffer[head], 0,  (size_t)requestSize);   /* optional, debug */
		}
		else  /* our read will wrap ! */
		{
			int64_t temp = cb->bufSize - head;
			circleread_memcpy(
                dest,
                &cb->buffer[head],
                temp);
			circleread_memcpy(
                ((unsigned char *) dest) + temp,
                cb->buffer,
                requestSize - temp);
		}
	}
	
	cb->head = (cb->head + requestSize) % cb->bufSize;
	cb->count -= requestSize;

	cb->bytesConsumed += requestSize;
	cb->balance -= 1;
	
	return (int)requestSize;  /* records (16 bit or maybe other in future) */
}







void testCircleBuffer()
{
	CircleBuffer_t temp;
	size_t bufSize = 256;
	const size_t maxInput = 256*3;
	size_t count = 0;
	size_t t;
	int i;
	const int maxRandom = 32;
	size_t chunkOut = 30;
	
	CircleRecord_t data[256*3];
	
	initCircleBuffer(&temp, bufSize, 75, 256, 0, 0);
	
	/* who cares ... take the default seed value. */
	while (count < maxInput  || temp.count > chunkOut)
	{
		t = rand();
		
		/* for whatever reason this seems to be a 16 bit random number */
		t = t / ( 2 << (16 - 7) ) ;
		
		for(i = 0; i < (int)t; i++)
		{
			data[i] = (unsigned char ) ( count + i  );
		}
		
		if 	(	
				((temp.bufSize - temp.count) >= t*sizeof(short)+1)  &&
				(count < (maxInput - maxRandom))
			)  /* add 1 to keep buffer being completely filled */
		{
			int64_t tail = (temp.head + temp.count) % temp.bufSize;
			addToCircleBuffer(&temp, data, t);
			printf("Add to buffer count = %ld. head = %ld, tail = %ld\n",  t, temp.head, tail);
			count += t;
		}
		else  /* not enough space in buffer, try to empty some out */
		{
			int r;
			r = readFromCircleBuffer(&temp, data, chunkOut);
			
			if (r >= 0)
			{
				int64_t tail = (temp.head + temp.count) % temp.bufSize;
				for(i = 0; i < r; i++)
					printf("%ld  ", data[i]);

				printf("\nRead from buffer. head = %ld, tail = %ld\n", temp.head, tail);
			}
		}
		
		
	}    /* while we have not accumulated a large eough test ... */
	
}






int CirclePercent(CircleBuffer_t* cb)
{
   return  (int)(cb->count * 100 / cb->bufSize);
}

int CircleAtLevel(CircleBuffer_t* cb)
{  
    return (int)(cb->count * 100 / cb->bufSize) >= cb->percent;
}

int CircleOverLevel(CircleBuffer_t* cb)
{  
    return (int)(cb->count * 100 / cb->bufSize) > cb->percent;
}