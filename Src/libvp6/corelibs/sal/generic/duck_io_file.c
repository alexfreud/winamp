/***********************************************\
??? duck_io.c
\***********************************************/

#include <stdio.h> 
#include <string.h>
#include <fcntl.h>

#if defined(_WIN32)
    #include <io.h>
#endif

#include "duck_io_file.h"
#include "duck_hfb.h"
#include "duck_io.h"
#include "duck_mem.h"

#include <assert.h>

#if defined (__cplusplus)
extern "C" {
#endif


#if defined (__cplusplus)
}
#endif



//int read_count;

#define MAKE_FOUR_CC(b1, b2, b3, b4 ) \
        ((b4 << 24) | (b3 << 16) | (b2 << 8) | (b1 << 0))

typedef struct  
{
    unsigned long   scheme;     /* contains FILE */
    #if defined(_WIN32)
        int             fileDescriptor;    /* used to be the handle */    
    #else
        FILE*           fileDescriptor;    /* used to be the handle */    
    #endif
    char            fname[512];
	int64_t         fileLength;
    char            errorString[256];
    SAL_ERR         lastErrorCode;

} FileScheme_t;


/* Add the decoration to avoid name collisions in the case where we want to include */
/* a forking version of duck_io that calls duck_io_file or duck_io_http             */
/* The user of this service should not need to compile and link all three files     */
/* duck_io (abstract forker) duck_io_file and duck_io_http.                         */
#define DECORATE(x) x##_file


/* Sets an error code and message */
static int SetError(int handle, SAL_ERR code, const char* appendMsg)
{
  	FileScheme_t* fileObj = (FileScheme_t *) handle;

    const char* decode = SalErrText(code);
    if (decode)
    {
        strcpy(fileObj->errorString, decode);
        fileObj->lastErrorCode = code;
        if (appendMsg)
        {
            strcat(fileObj->errorString, " : ");
            strcat(fileObj->errorString, appendMsg);
        }
        return code;
    }

    return SAL_ERROR; // DEFAULT ERROR
}


int DECORATE(duck_open)(const char *name, unsigned long userData)
{
    FileScheme_t* const fileObj = (FileScheme_t *) duck_calloc(1,sizeof(FileScheme_t), DMEM_GENERAL);
    const ReOpen_t* const openData = (ReOpen_t*) userData;

    fileObj->scheme = MAKE_FOUR_CC('f','i','l','e');

    assert(name);
    assert(strlen(name) < sizeof(fileObj->fname));
    strcpy(fileObj->fname,name);
    
    #if defined(_WIN32)
        fileObj->fileDescriptor = _open(name, _O_RDONLY | _O_BINARY);
       
        if(fileObj->fileDescriptor == -1)
            return SetError((int)fileObj, SAL_ERR_FILE_OPEN_FAILED, 0);        
    #else
        fileObj->fileDescriptor = fopen(name, "rb");
       
        if(fileObj->fileDescriptor == 0)
            return SetError((int)fileObj, SAL_ERR_FILE_OPEN_FAILED, 0);;
    #endif
    
    fileObj->fileLength = duck_seek((int) fileObj, 0, SEEK_END);

    duck_seek((int) fileObj, 0, SEEK_SET);

    if(openData)
        duck_seek_file((int) fileObj, openData->offset, SEEK_SET);

    return (int)fileObj;

}



void DECORATE(duck_close)(int handle)
{
    FileScheme_t* fileObj = (FileScheme_t *) handle;

    #if defined(_WIN32)
        _close(fileObj->fileDescriptor);
    #else
         fclose(fileObj->fileDescriptor);
    #endif

    if (fileObj)
    {
        duck_free(fileObj);
        fileObj = 0;
    }

}



int DECORATE(duck_read_blocking)(int handle,unsigned char *buffer,int bytes)
{
    return DECORATE(duck_read)(handle, buffer, bytes);
}




int DECORATE(duck_read)(int handle,unsigned char *buffer,int bytes)
{
    int x;
    FileScheme_t* fileObj = (FileScheme_t *) handle;

    if (buffer == NULL){
        duck_seek_file((int ) fileObj->fileDescriptor,bytes,SEEK_CUR);
        return bytes;
    }

    if (bytes == 0)
        return 0;
    
    #if defined(_WIN32)
        x = _read(fileObj->fileDescriptor, buffer, (unsigned int) bytes);
        if (x == -1L)
        {
            assert(0);
        }
    #else
        x = fread(buffer, sizeof(char) , (size_t) bytes, fileObj->fileDescriptor);
        if (x < bytes)
        {
            assert(x == bytes);
        }
    #endif
    
    
    return x ;
}


int64_t DECORATE(duck_seek)(int handle,   int64_t offset,  int origin)
{
#if defined(_WIN32)
    int64_t tellNo = 0;
    FileScheme_t* fileObj = (FileScheme_t *) handle;

    _lseeki64(fileObj->fileDescriptor,offset,origin);

    tellNo = _telli64(fileObj->fileDescriptor);

    return tellNo ;
#else
    int64_t tellNo = 0;
    FileScheme_t* fileObj = 0;
    assert(sizeof(off_t) == sizeof(int64_t));
    fileObj = (FileScheme_t *) handle;

    fseeko(fileObj->fileDescriptor,(off_t) offset,origin);

    tellNo = (int64_t) ftello(fileObj->fileDescriptor);

    return tellNo ;
#endif
    
}


int DECORATE(duck_name)(int handle, char fname[], size_t maxLen)
{
    FileScheme_t* fileObj = (FileScheme_t *) handle;

    if (strlen(fileObj->fname) < maxLen)
        strcpy(fname, fileObj->fname);
    else
        return -1;

    return 0;
}

int64_t DECORATE(duck_available_data)(int handle)
{
	FileScheme_t* fileObj = (FileScheme_t *) handle;
#if defined(_WIN32)    
    return fileObj->fileLength - _telli64(fileObj->fileDescriptor);
#else
    return fileObj->fileLength - (int64_t) ftello(fileObj->fileDescriptor);
#endif
}
