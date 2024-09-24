/***********************************************\
??? duck_io.c
\***********************************************/
#ifdef _WIN32
#pragma warning(push,3)
#endif

#include "duck_io_http.h"
#include "duck_mem.h"
#include "on2_timer.h"
#include "circlebuffer.h"
#include "duck_io.h"
#include <errno.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <assert.h>
extern "C" {
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
}
#endif

#include <sstream>
#include <limits.h>


#define DEFAULT_CBSIZE 512 * 1024

#define DECORATE(x) x##_http



int duck_sal_fill(void * handle, bool blocking, size_t maxFill);


//#include "debug.h" // This can be removed

#ifdef _WIN32
#pragma warning(pop)
#pragma warning(disable:4706) /* assignment in conditional expression */
#pragma warning(disable:4514) /* matt made me do it */
    const char i64Fmt[] = "%I64d";
#else
    const char i64Fmt[] = "%lld";
    int WSAGetLastError()
    {
        return -1;
    }
#endif

#define MAKE_FOUR_CC(b1, b2, b3, b4 ) \
        ((b4 << 24) | (b3 << 16) | (b2 << 8) | (b1 << 0))

typedef struct _DuckHttp_temp 
{
    unsigned long scheme;
    std::string         url;
    std::string         urlExtended;

#if defined(_WIN32)
    SOCKET              socket;
#else
    int                    socket;
#endif

    SAL_ERR         lastErrorCode;
    std::ostringstream   errorString;
    
    

    /* milliseconds to wait before disconnecting from server */
    unsigned int        timeOut;
    
    /* Assuming 2k is the max size of an http header */
    char                httpHeader[2 * 1024];
 
    int64_t             contentLen; // Size of the movie
    int                 headerSize; // Size of the http header
    int                 xtraData;   // Amout of movie data recv'd from the http header recv
    int64_t             totalRead;  // Position in file
    CircleBuffer_t      cBuffer;    
    int                 cbSize;     // Circular buffer size

} DuckHttp_t;



/* Returns size of movie as parsed from the http header */
int64_t duck_content_len(void*  handle)
{
    DuckHttp_t* httpObj = (DuckHttp_t *) handle;
    
    if (httpObj == 0)
        return -1;
        
    return httpObj->contentLen;
}



void* duck_get_buffer(void *handle)
{
     DuckHttp_t* httpObj = (DuckHttp_t *) handle;
     
     return (void *) &httpObj->cBuffer;
}



/* Checks to see if any errors occured in recv */
int isDataAvailable(int bytesReceived)
{
  	(void ) bytesReceived; // for warning supression

    #ifdef WIN32
    if (WSAGetLastError() == 10035) // No data is available right now, try again later
        return 0;
    #else
    
    int x = errno;
    
    if(x == EAGAIN) 
    {
        return 0;
    }
    else if (x == 0)
    {
        assert(bytesReceived != -1);
        return 1;
    }
    else
    {
        assert(0);
    }
    #endif
    
    return 1;
}

/* Sets an error code and message */
static int SetError(DuckHttp_t* httpObj, SAL_ERR code, const char* appendMsg)
{
    const char* decode = SalErrText(code);
    if (decode)
    {
        httpObj->errorString.str("");
        httpObj->errorString << decode;
        httpObj->lastErrorCode = code;
        if (appendMsg)
            httpObj->errorString << " : " << appendMsg;
        return code;
    }

    return SAL_ERROR; // Default error
}


static int64_t http_atoi(const char* str)
{
    int64_t temp = 0;
    size_t len = 0;

    while(str[len] >= '0' && str[len] <= '9')
        len++;

    for(size_t i = 0; i < len; i++)
        temp = temp * 10 + (str[i] - '0');

    return temp;
}




/* Parses url for a parameter */
inline bool get_url_parameter(const std::string url, std::string& parameter)
{
    
    std::string temp = url;
    size_t strPos;
    
    strPos = temp.find(parameter.c_str());
    
    if (strPos == std::string::npos)
    {
        return false;
    }
    else 
    {
       temp = temp.substr(strPos + parameter.length() + 1);
       
       size_t i = 0;
       
       for(i = 0; i < temp.length(); i++)
       {
            if (temp[i] == '&')
                temp[i] = '\0';
       }
       
       parameter = temp.c_str();
       
       return true;
    }
    
}


#if !defined(__cplusplus)
#error
#endif

char* duck_init_http(char *url)
{
    std::string strTime = "timeout";
    std::string strBuff = "buffer";

    DuckHttp_t* httpObj = new (DuckHttp_t);
    assert(httpObj);
    
    httpObj->scheme = MAKE_FOUR_CC('h','t','t','p');
    
    httpObj->urlExtended = url;

    if (get_url_parameter(url, strTime))
        sscanf(strTime.c_str(),"%d", &httpObj->timeOut); 
    else
        httpObj->timeOut = INT_MAX; // Wait "forever" is default

    if (get_url_parameter(url, strBuff))
    {
        sscanf(strBuff.c_str(), "%d", &httpObj->cbSize);    
        // Convert kilobytes into bytes
        httpObj->cbSize *= 1024;
    }
    else
        httpObj->cbSize = DEFAULT_CBSIZE;
    
    for(size_t i = 0; i < strlen(url); i++)
    {
        if (url[i] == '?')
            url[i] = '\0';
    }
    
    httpObj->url = url;
    httpObj->contentLen = 0;

#ifdef _WIN32

    WSADATA             wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData))
    {
        std::ostringstream temp;
        temp << " : " << WSAGetLastError();
        SetError(httpObj,SAL_ERR_WSASTARTUP,temp.str().c_str());
        return 0;
    }
#endif

    return (char *) httpObj;
}






/* Might need to reduce timeout after initial buffering */
/*------------------------------------------------------*/
void duck_http_timeout(int handle, unsigned long milliseconds)
{
        DuckHttp_t* httpObj = (DuckHttp_t *) handle;
        httpObj->timeOut = milliseconds;
}





int DECORATE(duck_open)(const char* src, unsigned long userData)
{

    struct sockaddr_in  sa_in;
    struct sockaddr*    const psa = (struct sockaddr*)&sa_in;
    struct hostent*     host;
    std::ostringstream    buf1;
    unsigned short      port = 0;
    int                 input;
    char*               endOfHttpHeader = "\r\n\r\n";
    char*               strContentLen = "Content-Length: ";
    char*               strPos;
    on2Timer            start;
    (void)              userData;
    unsigned long        block = 1;
    DuckHttp_t*         httpObj;

    ReOpen_t*        reOpenData = (ReOpen_t*) userData;

    if(reOpenData == 0 || reOpenData->blocking || reOpenData->offset == 0)
        httpObj = (DuckHttp_t *) duck_init_http((char *) src);
    else
        httpObj = (DuckHttp_t *) src;
        
    
    if(!reOpenData) // if we're not doing a re-open
        httpObj->totalRead = 0;
    else
        httpObj->totalRead = reOpenData->offset;
        
    
    std::stringbuf path;
    std::stringbuf server;
    std::istringstream is(httpObj->url);

    is.ignore(strlen("http://"), '\0');  

    // Check if a port is specified
    for(size_t i = strlen("http://"); i < httpObj->url.length(); i++)
    {
        if(httpObj->url[i] == ':')
        {
            port = 1;
            break;
        } else if(httpObj->url[i] == '/') break;
    }

    if(port) 
    {
        std::stringbuf strPort;

        is.get(server,':');/* get the server */
        is.ignore(1, '\0');
        is.get(strPort, '/');
        port = (unsigned short)http_atoi(strPort.str().c_str());
    }
    else
    {
        is.get(server,'/');/* get the server */

        port = 80; // default http port
    }

    assert(server.str().length() > 0);

    is.ignore(1, '\0');      /* get the path */
    is.get(path, '\0');

    /* Wrap up the send message */
    buf1 << "GET " << "/" << path.str() << " HTTP/1.1 " << "\r\n";
    buf1 << "Host:" << server.str().c_str() << "\r\n" ;
    
    if (reOpenData)
    {
        char number[64];
        sprintf(number, i64Fmt, reOpenData->offset);
        buf1 << "Range: bytes=" << number << "-" ;
        buf1 << " \r\n" ;
    }
    buf1 << "\r\n";

    if ((httpObj->socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "duck_open: SAL_ERR_SOCKET_CREATE\n");
        SetError(httpObj,SAL_ERR_SOCKET_CREATE, 0);
        duck_exit_http((int)httpObj);
        return SAL_ERR_SOCKET_CREATE;
    }

    sa_in.sin_family      = AF_INET;
    sa_in.sin_port        = htons(port);

    if (!(host = gethostbyname(server.str().c_str())))
    {
        fprintf(stderr, "duck_open: SAL_ERR_RESOLVING_HOSTNAME\n");
        SetError(httpObj,SAL_ERR_RESOLVING_HOSTNAME,0);
        duck_exit_http((int)httpObj);
        return SAL_ERR_RESOLVING_HOSTNAME;
    }

    duck_memcpy(&sa_in.sin_addr, host->h_addr_list[0], sizeof(struct in_addr));

    if (connect(httpObj->socket, psa, sizeof(sa_in) ) != 0)
    {
        fprintf(stderr, "duck_open: SAL_ERR_SERVER_CONNECTION\n");
        SetError(httpObj,SAL_ERR_SERVER_CONNECTION,0);
        duck_exit_http((int)httpObj);
        return SAL_ERR_SERVER_CONNECTION;
    }
    
    /* connected */

    if (send(httpObj->socket, buf1.str().c_str(), strlen(buf1.str().c_str()), 0) < 0)
    {
        fprintf(stderr, "duck_open: SAL_ERR_SENDING_DATA\n");
        SetError(httpObj,SAL_ERR_SENDING_DATA,0);
        duck_exit_http((int)httpObj);
        return SAL_ERR_SENDING_DATA;
    }


    on2Timer_Init(&start);
    on2Timer_Start(&start);

    duck_memset(httpObj->httpHeader, 0, sizeof(httpObj->httpHeader));

    /* Get the HTTP header EMH 2-14-03 */
    /* Assuming we get all the header info in the 1st good recv */
    do
    {
        unsigned long delta = on2Timer_GetCurrentElapsedMilli(&start);

        if (delta > httpObj->timeOut)
        {
            return SetError(httpObj,SAL_ERR_CONNECTION_TIMEOUT,0);
        }   

        input = recv(httpObj->socket,  httpObj->httpHeader, sizeof(httpObj->httpHeader), 0);
    } while (!strstr(httpObj->httpHeader, endOfHttpHeader));

#ifdef _WIN32
    ioctlsocket(httpObj->socket, FIONBIO, &block); /* Set the socket to non-blocking */
#else
    if (ioctl(httpObj->socket, FIONBIO, &block)) /* Set the socket to non-blocking */
    {
        assert(0);
    }
#endif
    
    strPos = strstr(httpObj->httpHeader, endOfHttpHeader);
    
    strPos += strlen(endOfHttpHeader);

    httpObj->headerSize = (int)strPos - (int)httpObj->httpHeader;

    httpObj->xtraData = input - httpObj->headerSize; // Amount of GOOD data grabbed with the HTTP header

    if (strstr(httpObj->httpHeader, "404") && strstr(httpObj->httpHeader, "Not Found"))
    {
        fprintf(stderr, "duck_open: SAL_ERR_404_FILE_NOT_FOUND\n");
        SetError(httpObj,SAL_ERR_404_FILE_NOT_FOUND,0);
        duck_exit_http((int)httpObj);
        return SAL_ERR_404_FILE_NOT_FOUND;
    }
    
    strPos = strstr(httpObj->httpHeader, strContentLen);

    if (!strPos)
    {
        fprintf(stderr, "duck_open: SAL_ERR_PARSING_HTTP_HEADER\n");
        SetError(httpObj,SAL_ERR_PARSING_HTTP_HEADER,0);
        duck_exit_http((int)httpObj);
        return SAL_ERR_PARSING_HTTP_HEADER;
    }

    strPos += strlen(strContentLen);
    
    if((*strPos >= '0') && (*strPos <= '9'))
    {
        httpObj->contentLen = http_atoi(strPos);
    }
    else
    {
        fprintf(stderr, "duck_open: SAL_ERR_PARSING_CONTENT_LEN\n");
        SetError(httpObj,SAL_ERR_PARSING_CONTENT_LEN,0);
        duck_exit_http((int)httpObj);
        return SAL_ERR_PARSING_CONTENT_LEN;
    }


    int rv;
    
    rv = initCircleBuffer(&httpObj->cBuffer, httpObj->cbSize, 75,
            httpObj->cbSize/4, /* max chunk */  0, 0);

    if (rv < 0)
        assert(0);

    addToCircleBuffer(&httpObj->cBuffer,
                      httpObj->httpHeader + httpObj->headerSize,
                      (size_t) httpObj->xtraData);

    bool blocking = true;
/*
    // Block only if we're not doing a re-open
    userData ? blocking = false : blocking = true;
*/
    if(reOpenData)
        blocking = (reOpenData->blocking != 0);
   
    if (duck_sal_fill((void *) httpObj, blocking, 0) < 0)
    {
        fprintf(stderr, "duck_open: SAL_ERR_RECEIVING_DATA\n");
        duck_close_http((int)httpObj);
        return -1;
    }

    return (int) httpObj;
}




void DECORATE(duck_close)(int handle)
{
    DuckHttp_t* httpObj = (DuckHttp_t *) handle;

#if defined(_WIN32)
    closesocket(httpObj->socket); // Close the old socket
#else
    close(httpObj->socket); 
#endif
    destroyCircleBuffer(&httpObj->cBuffer);

    duck_exit_http(handle);
}





void duck_exit_http(int handle)
{
    DuckHttp_t* httpObj = (DuckHttp_t *) handle;
    
    //delete httpObj->cBuffer;
    delete httpObj;

#ifdef _WIN32
    WSACleanup(); 
#endif
}




/* Read data off of the socket directly into the circular buffer */
inline int recv_circular (void* handle, size_t maxBytes)
{
    DuckHttp_t* httpObj = (DuckHttp_t *) handle;
    int bytesRead = 0;
    int totalRead = 0;


    size_t  tail = (httpObj->cBuffer.head + httpObj->cBuffer.count) % httpObj->cBuffer.bufSize;
    size_t head = httpObj->cBuffer.head;
    size_t freeSpace = httpObj->cBuffer.bufSize - httpObj->cBuffer.count;
    size_t endSpace = httpObj->cBuffer.bufSize - tail;
    size_t least;
        
    if (tail >= head && maxBytes > endSpace)  /* additional data write will wrap  */
    {
        /* try to fill to end of buffer */
        bytesRead = recv(httpObj->socket, (char*)httpObj->cBuffer.buffer + tail, (int)endSpace, 0); 

        if (bytesRead < 0)
        {
            if (isDataAvailable((int)bytesRead) == 0)
                return 0; // Try again later...
            else                    
                return (int)bytesRead; // Error
        }

        totalRead += bytesRead;
        httpObj->cBuffer.count += bytesRead;
        maxBytes -= bytesRead;

        freeSpace = httpObj->cBuffer.bufSize - httpObj->cBuffer.count;
        
        if((size_t)bytesRead < maxBytes && (size_t)bytesRead == endSpace) /* filled to end and more to read */
        {
            httpObj->cBuffer.wrapped = 1;
            least = (maxBytes < freeSpace) ? maxBytes : freeSpace;
            bytesRead = recv(httpObj->socket, (char *)httpObj->cBuffer.buffer, (int)least, 0);
            
            if (bytesRead < 0)
            {
                if (isDataAvailable((int)bytesRead) == 0)
                    return 0; // Try again later...
                else                    
                    return (int)bytesRead; // Error
            }
            
            totalRead += bytesRead;
            httpObj->cBuffer.count += bytesRead;
        }
    }
    else /* existing data wrapped around from end of buffer through beginning of buffer. */
    {
        if (tail < head)
            httpObj->cBuffer.wrapped = 1;
        least = (maxBytes < freeSpace) ? maxBytes : freeSpace;
        bytesRead = recv(httpObj->socket, (char*)httpObj->cBuffer.buffer + tail, (int)least, 0);
                
        if (bytesRead < 0)
        {
            if (isDataAvailable((int)bytesRead) == 0)
                return 0; // Try again later...
            else                    
                return (int)bytesRead; // Error
        }

        totalRead += bytesRead;
        httpObj->cBuffer.count += bytesRead;
    }
    return (int)totalRead;    
}





/* Re-charge the circular buffer */
int duck_sal_fill(void * handle, bool blocking, size_t maxFill)
{
    DuckHttp_t* httpObj = (DuckHttp_t *) handle;
    on2Timer    start;
    int bytesRead = 0;
    int totalRead = 0;

    int fillLevel = httpObj->cBuffer.bufSize * httpObj->cBuffer.percent / 100;
    int fillSpace = fillLevel - httpObj->cBuffer.count;

    if(maxFill)
    {
        // Charge twice as much as was read
        maxFill *=2; 
        // Take the lesser of the two
        fillSpace = ((int)maxFill < fillSpace) ?  maxFill : fillSpace;
    }

    on2Timer_Init ( &start );
    on2Timer_Start ( &start );

    while ((httpObj->cBuffer.count < (size_t)fillLevel) && ((int)httpObj->cBuffer.count < httpObj->contentLen))
    {
        unsigned long delta = on2Timer_GetCurrentElapsedMilli(&start);
        
        if (delta > httpObj->timeOut)
        {
            std::ostringstream temp;
            temp << "Bytes received = " << totalRead;
            //return -1;
            return SetError(httpObj, SAL_ERR_CONNECTION_TIMEOUT, temp.str().c_str());
        }
        bytesRead = recv_circular(handle, fillSpace);
        
        
        #if defined(__APPLE__) || defined(__POWERPC__)
        if (bytesRead == 0 && blocking) /* please give some time to the SOCKET thread / OS . */
                  usleep(1000*2);    
        #endif
        
		if (bytesRead < 0)
		{
			std::ostringstream temp;
			temp << " : WSAGetLastError = " << WSAGetLastError();
				SetError(httpObj,SAL_ERR_SERVER_CONNECTION,temp.str().c_str());
				return bytesRead;
		}

        totalRead += bytesRead;

        if (blocking == 0) /* we only want one recv done */
            return totalRead;

    }
    return totalRead;
}





int DECORATE(duck_read)(int handle,unsigned char *buffer, int bytes)
{
    DuckHttp_t* httpObj = (DuckHttp_t *) handle;
    int input;
 
    if (bytes < 0)
        return -1;

    assert(httpObj);
    assert(buffer);

    input = readFromCircleBuffer(&httpObj->cBuffer, buffer, bytes);

    if (input >= 1)
    {
        httpObj->totalRead += input;
    }

    bool blocking = false;

    if (duck_sal_fill((void *)handle, blocking, bytes) < 0)
    {
        return -1; // The socket probably disconnected
    }    

    return input;
}






int DECORATE(duck_read_blocking)(int handle,unsigned char *buffer, int bytes)
{
    DuckHttp_t* httpObj = (DuckHttp_t *) handle;
    int input;
    int amountRead = 0;
 
    if (bytes < 0)
        return -1;

    assert(httpObj);
    assert(buffer);

    while (amountRead < bytes)
    {
        input = readFromCircleBuffer(
                    &httpObj->cBuffer,
                    buffer + amountRead,
                    bytes - amountRead);
        
        if (input < 0)
            return input;
        else
        {
            amountRead += input;
            httpObj->totalRead += input;
        }
        
        bool blocking = false;
        
        if (duck_sal_fill((void *)handle, blocking, bytes) < 0)
        {
            return -1; // The socket probably disconnected
        }    
    }
    return amountRead;
}






int64_t DECORATE(duck_seek)(int handle, int64_t offset,int origin)
{
    DuckHttp_t* httpObj = (DuckHttp_t *) handle;
    
    //printf("seeking to offset = %ld, origin = %ld\n", offset, origin);

    

    if (offset < 0)
    {
        fprintf(stderr, "Trying to seek backwards with offset = %d\n", offset);
        assert(0);
    }

    if (origin == SEEK_END)
    {
        fprintf(stderr, "SEEK_END is not supported\n", offset);
        assert(0);
    }

    
    if (origin == SEEK_SET)
    {
        if (    offset > httpObj->totalRead &&
                ForwardBuffer(&httpObj->cBuffer, (offset - httpObj->totalRead)) == 0 
                ) /* forward small jump */
        {
            // We've eaten away at the buffer so re-charge it.
            duck_sal_fill((void *)handle, false, (int)(offset - httpObj->totalRead)); 
            httpObj->totalRead = offset;
            return httpObj->totalRead;
        }
        
        else if (   offset < httpObj->totalRead && 
                    RewindBuffer(&httpObj->cBuffer, (httpObj->totalRead - offset)) == 0
                )  /* backwards small jump */
        {  
            httpObj->totalRead = offset;
            return httpObj->totalRead;
        }
        
        else 
            httpObj->totalRead = offset;
    }
    
    
    if (origin == SEEK_CUR)
    {
        if (!offset) // They just want the current pos
            return httpObj->totalRead;

           httpObj->totalRead += offset;

        if(ForwardBuffer(&httpObj->cBuffer, offset) == 0)
        {
            duck_sal_fill((void *)handle, false, (size_t)offset); // We've eaten away at the buffer so re-charge it.
            return httpObj->totalRead;
        }
 
    }

#if defined(_WIN32)
    closesocket(httpObj->socket); // Close the old socket
#else
        close(httpObj->socket);    
#endif

    destroyCircleBuffer(&httpObj->cBuffer);

    ReOpen_t openData;
    openData.offset = httpObj->totalRead;
    openData.blocking = 0; 

    // Reconnect to http server
    if( duck_open_http((char* )handle, (unsigned long)&openData) < 0) 
    {
        char err[256];
        SAL_ERR errCode;
        duck_sal_error_http((void*)handle, &errCode, err, sizeof(err));
        assert(0);
        return -1;
    }
    return httpObj->totalRead;
    
}






int64_t duck_tell(int handle)
{
    DuckHttp_t* httpObj = (DuckHttp_t *) handle;

    return httpObj->totalRead;
}





/* Return the amount of data in the circular buffer */
int duck_sal_buff_percent(void* handle)
{
    DuckHttp_t* httpObj = (DuckHttp_t *) handle;
    return 100 * httpObj->cBuffer.count / httpObj->cBuffer.bufSize;
}


int64_t DECORATE(duck_available_data)(int handle)
{
    DuckHttp_t* httpObj = (DuckHttp_t *) handle;
    return httpObj->cBuffer.count;
}



/* Checks the last error */
int DECORATE(duck_sal_error)(void* handle, SAL_ERR* lastErrorCode, char buffer[], size_t maxLen)
{
    DuckHttp_t* httpObj = (DuckHttp_t *) handle;

    *lastErrorCode = httpObj->lastErrorCode;

    if (httpObj->errorString.str().length() <= maxLen)
    {
        strcpy(buffer, httpObj->errorString.str().c_str());
        return 0;
    }
    else
        return -1;

}

int DECORATE(duck_name)(int handle, char url[], size_t maxLen)
{
    DuckHttp_t* httpObj = (DuckHttp_t *) handle;

    if (httpObj->urlExtended.length() <= maxLen)
        strcpy(url, httpObj->urlExtended.c_str());
    else
        return -1;

    return 0;
}
