#include "NXFileObject.h"
#include "nu/ProgressTracker.h"
#include "nx/nxthread.h"
#include "nx/nxsleep.h"
#include "jnetlib/jnetlib.h"
#include "../nswasabi/AutoCharNX.h"
#include "nswasabi/ReferenceCounted.h"
#include "nu/MessageLoop.h"
#include <time.h>
#include <new>
#include "../../../WAT/WAT.h"

/* TODO: benski> test this with a server that does not return content-length.  I bet we could get it to work */

/* TODO: benski> on windows, we can use a single CreateFile HANDLE for both reading and writing
                 and use ReadFile(..., &overlapped) to maintain two separate file pointers
								 this should improve performance as they will share the same cache 
								 _might_ have to use async I/O to get it to work (but use it synchronously by waiting on the handle after making the call
								 */

#define HTTP_BUFFER_SIZE 65536

class NXFileObject_ProgressiveDownloader;

enum
{
	MESSAGE_KILL,
	MESSAGE_SEEK,
	MESSAGE_SIZE,
	MESSAGE_ERROR,
	MESSAGE_CLOSED,
	MESSAGE_CONNECTED,
};

char MessageString[6][10] =
{
	"Kill",
	"Seek",
	"Size",
	"Error",
	"Closed",
	"Connected"
};


struct seek_message_t : public nu::message_node_t
{
	uint64_t start;
	uint64_t end;
};

struct size_message_t : public nu::message_node_t
{
	uint64_t size;
};

struct error_message_t : public nu::message_node_t
{
	int error_code;
};

/* This class represents the thread that's actually downloading the content from the server */
class ProgressiveDownload 
{
public:
	ProgressiveDownload(ProgressTracker &progress_tracker, NXFileObject_ProgressiveDownloader &parent);
	~ProgressiveDownload();
	ns_error_t Initialize(nx_uri_t uri, jnl_http_t http, const char *user_agent, nx_uri_t temp_uri);
	
	void Seek(uint64_t start, uint64_t end);
	void Close();
private:
	/* These functions are called on the local thread */
	/* These functions run on the download thread */
	static nx_thread_return_t NXTHREADCALL _ProgressiveThread(nx_thread_parameter_t param) { return ((ProgressiveDownload *)param)->ProgressiveThread(); }
	nx_thread_return_t NXTHREADCALL ProgressiveThread();
	int Connect();
	void Internal_Write(const void *data, size_t data_len);
	int Wait(int milliseconds);
	ns_error_t SetupConnection(uint64_t start_position, uint64_t end_position);
	int DoRead(void *buffer, size_t bufferlen);
	void ProcessMessage(nu::message_node_t *message);
private:
	ProgressTracker &progress_tracker;
	NXFileObject_ProgressiveDownloader &parent;

	nx_uri_t temp_filename, url;
	FILE *progressive_file_write;	
	jnl_http_t http;
	char *user_agent;
	nx_thread_t download_thread;
	nu::MessageLoop message_loop;
	uint64_t file_size;
	int killswitch;
};

class NXFileObject_ProgressiveDownloader: public NXFileObject
{
public:
	NXFileObject_ProgressiveDownloader();
	~NXFileObject_ProgressiveDownloader();
	ns_error_t Initialize(nx_uri_t uri, jnl_http_t http, const char *user_agent);

	bool Available(uint64_t size, uint64_t *available);

	/* API used by ProgressiveDownload */
	void OnFileSize(uint64_t filesize);
	void OnConnected();
	void OnError(int error_code);
	void OnClosed();
private:
	/* NXFileObject implementation */
	ns_error_t Read(void *buffer, size_t bytes_requested, size_t *bytes_read);
	ns_error_t Write(const void *buffer, size_t bytes);
	ns_error_t Seek(uint64_t position);
	ns_error_t Tell(uint64_t *position);
	ns_error_t PeekByte(uint8_t *byte);
	ns_error_t Sync();
	ns_error_t Truncate();

	bool WaitForRead(uint64_t size);
	void ProcessMessage(nu::message_node_t *message);
	void Wait(unsigned int milliseconds);

	ProgressiveDownload download;
	ProgressTracker progress_tracker;
	FILE *progressive_file_read;
	bool end_of_file;
	bool connected;
	int error_code;
	nu::MessageLoop message_loop;
	bool closed;
	bool need_seek; // if set to true, we need to fseek(position)
};

ProgressiveDownload::ProgressiveDownload(ProgressTracker &progress_tracker, NXFileObject_ProgressiveDownloader &parent) : progress_tracker(progress_tracker), parent(parent)
{
	killswitch=0;
	url=0;
	temp_filename=0;
	progressive_file_write=0;
	http=0;
	user_agent=0;
	download_thread=0;
	file_size=0;
}

ProgressiveDownload::~ProgressiveDownload()
{	
	if (download_thread)
	{
		Close();
		NXThreadJoin(download_thread, 0);
	}

	// TODO: flush messages
	if (progressive_file_write)
		fclose(progressive_file_write);
	NXURIRelease(temp_filename);
	NXURIRelease(url);
	if (http)
		jnl_http_release(http);
	free(user_agent);
}

void ProgressiveDownload::Close()
{
	nu::message_node_t *message = message_loop.AllocateMessage();
	message->message = MESSAGE_KILL;
	message_loop.PostMessage(message);
}

void ProgressiveDownload::Seek(uint64_t start, uint64_t end)
{
	seek_message_t *message = (seek_message_t *)message_loop.AllocateMessage();
	message->message = MESSAGE_SEEK;
	message->start = start;
	message->end = end;
	message_loop.PostMessage(message);
}

ns_error_t ProgressiveDownload::Initialize(nx_uri_t url, jnl_http_t http, const char *user_agent, nx_uri_t temp_filename)
{
	this->url = NXURIRetain(url);
	this->temp_filename = NXURIRetain(temp_filename);
	if (user_agent)
		this->user_agent = strdup(user_agent);
	this->http = jnl_http_retain(http);
	progressive_file_write = NXFile_fopen(temp_filename, nx_file_FILE_readwrite_binary);
	if (progressive_file_write == 0)
		return NErr_FailedCreate;

	return NXThreadCreate(&download_thread, _ProgressiveThread, this);
}

void ProgressiveDownload::ProcessMessage(nu::message_node_t *message)
{
	switch(message->message)
	{
	case MESSAGE_KILL:
		killswitch=1;
		break;
	case MESSAGE_SEEK:
		{
			seek_message_t *seek_message = (seek_message_t *)message;

			char buffer[HTTP_BUFFER_SIZE] = {0};

			/* empty out the jnetlib buffer.  that might let us be able to avoid this seek  */
			DoRead(buffer, sizeof(buffer));
			
			uint64_t new_start, new_end;
			if (!progress_tracker.Valid(seek_message->start, seek_message->end)  /* double check that we actually need to seek */
				&& !progress_tracker.Seek(seek_message->start, seek_message->end, &new_start, &new_end))
			{
				int ret = SetupConnection(new_start, new_end);
				if (ret == NErr_Success)
					ret = Connect();
				if (ret != NErr_Success)
				{
					parent.OnError(ret);
					killswitch=1;
					break;
				}

				_fseeki64(progressive_file_write, new_start, SEEK_SET);
			}
			else
				parent.OnConnected();
		}
		break;
	}

	message_loop.FreeMessage(message);
}

int ProgressiveDownload::Wait(int milliseconds)
{
	for (;;)
	{
		if (killswitch)
			return 1;

		nu::message_node_t *message = message_loop.PeekMessage(milliseconds);
		if (message)
			ProcessMessage(message);
		else
			break;
	}	

	nu::message_node_t *message = message_loop.PeekMessage(milliseconds);
	if (message)
		ProcessMessage(message);
	
	return killswitch;		
}

ns_error_t ProgressiveDownload::SetupConnection(uint64_t start_position, uint64_t end_position)
{
	if (!http)
		http = jnl_http_create(HTTP_BUFFER_SIZE, 0);

	if (!http)
		return NErr_FailedCreate;

	jnl_http_reset_headers(http);
	if (user_agent)
		jnl_http_addheadervalue(http, "User-Agent", user_agent);

	if (start_position && start_position != (uint64_t)-1)
	{
		if (end_position == (uint64_t)-1)
		{
			char temp[128] = {0};
			sprintf(temp, "Range: bytes=%llu-", start_position);
			jnl_http_addheader(http, temp);
		}
		else
		{
			char temp[128] = {0};
			sprintf(temp, "Range: bytes=%llu-%llu", start_position, end_position);
			jnl_http_addheader(http, temp);
		}
	}
	
	jnl_http_addheader(http, "Connection: Close"); // TODO: change if we ever want a persistent connection and downloading in chunks
	jnl_http_connect(http, AutoCharUTF8(url), 1, "GET");

	return NErr_Success;
}

int ProgressiveDownload::Connect()
{
	// TODO: configurable timeout
		/* wait for connection */
#ifdef _DEBUG
	const int timeout = 15000;
#else
	const int timeout = 15;
#endif
	time_t start_time = time(0);

	int http_status = jnl_http_get_status(http);
	while (http_status == HTTPGET_STATUS_CONNECTING || http_status == HTTPGET_STATUS_READING_HEADERS)
	{
		if (Wait(55) != 0)
			return NErr_Interrupted;

		int ret = jnl_http_run(http);
		if (ret == HTTPGET_RUN_ERROR)
			return NErr_ConnectionFailed;
		if (start_time + timeout < time(0))
			return NErr_TimedOut;

		http_status = jnl_http_get_status(http);
	} 
	
	if (http_status == HTTPGET_STATUS_ERROR)
	{
		switch(jnl_http_getreplycode(http))
		{
			case 400:
				return NErr_BadRequest;
			case 401:
				// TODO: deal with this specially
				return NErr_Unauthorized;
			case 403:
				// TODO: deal with this specially?
				return NErr_Forbidden;
			case 404:
				return NErr_NotFound;
			case 405:
				return NErr_BadMethod;
			case 406:
				return NErr_NotAcceptable;
			case 407:
				// TODO: deal with this specially
				return NErr_ProxyAuthenticationRequired;
			case 408:
				return NErr_RequestTimeout;
			case 409:
				return NErr_Conflict;
			case 410:
				return NErr_Gone;
			case 500:
				return NErr_InternalServerError;
			case 503:
				return NErr_ServiceUnavailable;
			default:
				return NErr_ConnectionFailed;
		}
	}
	else
	{
		if (!file_size)
		{
			// TODO: check range header for actual size
			file_size = jnl_http_content_length(http);
			parent.OnFileSize(file_size);
		}
		parent.OnConnected();
		return NErr_Success;
	}	
}

void ProgressiveDownload::Internal_Write(const void *data, size_t data_len)
{
	size_t bytes_written = fwrite(data, 1, data_len, progressive_file_write);
	fflush(progressive_file_write);
	progress_tracker.Write(bytes_written);
}

int ProgressiveDownload::DoRead(void *buffer, size_t bufferlen)
{
	int ret = jnl_http_run(http);
	size_t bytes_received;
	do
	{
		ret = jnl_http_run(http);
		bytes_received = jnl_http_get_bytes(http, buffer, bufferlen);
		if (bytes_received)
		{
			Internal_Write(buffer, bytes_received);
		}
			/* TODO: benski> should we limit the number of times through this loop?
			I'm worried that if data comes in fast enough we might get stuck in this for a long time  */
	} while (bytes_received == bufferlen);
	return ret;
}

nx_thread_return_t ProgressiveDownload::ProgressiveThread()
{
	ns_error_t ret;

	if (!http)
	{
		ret = SetupConnection(0, (uint64_t)-1);
		if (ret != NErr_Success)
		{
			parent.OnError(ret);
			parent.OnClosed();
			return 0;
		}
	}


	ret = Connect();
	if (ret != NErr_Success)
	{
		parent.OnError(ret);
	}
	else
	{	
		for (;;)
		{
			if (Wait(10) == 1)
				break; // killed!

			char buffer[HTTP_BUFFER_SIZE] = {0};
			int ret = DoRead(buffer, sizeof(buffer));
			if (ret == -1)
				break;
			else if (ret == HTTPGET_RUN_CONNECTION_CLOSED)
			{
				if (jnl_http_bytes_available(http) == 0)
				{
					if (progress_tracker.Valid(0, file_size))
					{
						// file is completely downloaded. let's gtfo
						fclose(progressive_file_write);
						progressive_file_write=0;
						break;
					}

					// if we're not completely full then we need to sit around for a potential MESSAGE_SEEK
					//while (Wait(100) == 0)
					{
						// nop
					}
				}
			}
		}
	}

	parent.OnClosed();
	return 0;
}

 /* ------------------ */
NXFileObject_ProgressiveDownloader::NXFileObject_ProgressiveDownloader() : download(progress_tracker, *this)
{
	progressive_file_read=0;
	end_of_file=false;
	connected=false;
	error_code=NErr_Success;
	closed = false;
	need_seek=false;
	position=0;
}



NXFileObject_ProgressiveDownloader::~NXFileObject_ProgressiveDownloader()
{	
	download.Close();
	while (!closed)
		Wait(10);
	if (progressive_file_read)
		fclose(progressive_file_read);
}

void NXFileObject_ProgressiveDownloader::OnConnected()
{
	nu::message_node_t *message = message_loop.AllocateMessage();
	message->message = MESSAGE_CONNECTED;
	message_loop.PostMessage(message);
}

void NXFileObject_ProgressiveDownloader::OnError(int error_code)
{
	error_message_t *message = (error_message_t *)message_loop.AllocateMessage();
	message->message = MESSAGE_ERROR;
	message->error_code = error_code;
	message_loop.PostMessage(message);
}

void NXFileObject_ProgressiveDownloader::OnFileSize(uint64_t size)
{
	size_message_t *message = (size_message_t *)message_loop.AllocateMessage();
	message->message = MESSAGE_SIZE;
	message->size = size;
	message_loop.PostMessage(message);
}

void NXFileObject_ProgressiveDownloader::OnClosed()
{
	nu::message_node_t *message = message_loop.AllocateMessage();
	message->message = MESSAGE_CLOSED;
	message_loop.PostMessage(message);
}

ns_error_t NXFileObject_ProgressiveDownloader::Initialize(nx_uri_t uri, jnl_http_t http, const char *user_agent)
{
	ReferenceCountedNXURI temp_uri;
	NXURICreateTemp(&temp_uri);
	ns_error_t ret = download.Initialize(uri, http, user_agent, temp_uri);
	if (ret != NErr_Success)
	{
		closed=true;
		return ret;
	}

	progressive_file_read = NXFile_fopen(temp_uri, nx_file_FILE_read_binary);

	for (;;)
	{
		Wait(10);
		if (error_code != NErr_Success)
			return error_code;

		if (connected)
			break;
	}
	return NErr_Success;
}

void NXFileObject_ProgressiveDownloader::ProcessMessage(nu::message_node_t *message)
{
	switch(message->message)
	{
	case MESSAGE_ERROR:
		{
			error_message_t *seek_message = (error_message_t *)message;
			error_code = seek_message->error_code;
		}
		break;
	case MESSAGE_CONNECTED:
		connected = true;
		break;
	case MESSAGE_SIZE:
		{
			size_message_t *seek_message = (size_message_t *)message;
			region.end = seek_message->size;
		}
		break;
	case MESSAGE_CLOSED:
		closed=true;
		break;
	}

	message_loop.FreeMessage(message);
}

void NXFileObject_ProgressiveDownloader::Wait(unsigned int milliseconds)
{
	for (;;)
	{
		nu::message_node_t *message = message_loop.PeekMessage(milliseconds);
		if (message)
			ProcessMessage(message);
		else
			break;
	}	

	nu::message_node_t *message = message_loop.PeekMessage(milliseconds);
	if (message)
		ProcessMessage(message);
}

bool NXFileObject_ProgressiveDownloader::WaitForRead(uint64_t size)
{
	if (progress_tracker.Valid(position, position+size))
		return true;

	if (need_seek)
	{
		// give it just a little bit of time to avoid constant reseeks when the download thread is just barely keeping up
		Wait(10);
		if (progress_tracker.Valid(position, position+size))
			return true;		

		connected=false;
		error_code=NErr_Success;
		download.Seek(position, (uint64_t)position+size);
		
		for (;;)
		{
			Wait(10);
			if (error_code != NErr_Success)
				return false;
			
			if (connected)
				break;
		}
	}

	while (!progress_tracker.Valid(position, position+size))
	{
		Wait(10);
	}

	return true;
}


ns_error_t NXFileObject_ProgressiveDownloader::Read(void *buffer, size_t bytes_requested, size_t *bytes_read)
{
	if (end_of_file || position >= (region.end - region.start))
		return NErr_EndOfFile;

	// don't allow a read past the end of the file as this will confuse progress_tracker (which doesn't know/care about the file length)
	if ((position + bytes_requested) > region.end)
		bytes_requested = (size_t)(region.end - position);

	if (WaitForRead((uint64_t)bytes_requested) == false)
	{
		*bytes_read = 0;
		return error_code;
	}

	if (need_seek)
	{
		_fseeki64(progressive_file_read, position, SEEK_SET);
		need_seek=false;
	}

	/* TODO: benski> if r < bytes_requested, then we need to flush the buffer.
	on windows, we can use fflush(progressive_file_read)
	on other platforms it's not guaranteed! */
	size_t r = fread(buffer, 1, bytes_requested, progressive_file_read);
	this->position += r;
	*bytes_read = r;
	return NErr_Success;		
}

ns_error_t NXFileObject_ProgressiveDownloader::Seek(uint64_t new_position)
{
	if (new_position >= (region.end - region.start))
	{
		this->position = region.end - region.start;
		end_of_file=true;
	}
	else
	{
		if (new_position == position)
			return NErr_Success;
		position = new_position;
		need_seek=true;
		
		end_of_file=false;
	}
	return NErr_Success;
}

ns_error_t NXFileObject_ProgressiveDownloader::Tell(uint64_t *position)
{
	if (end_of_file)
		*position = region.end - region.start;
	else
		*position = this->position - region.start;
	return NErr_Success;
}

ns_error_t NXFileObject_ProgressiveDownloader::PeekByte(uint8_t *byte)
{
	if (position == region.end)
		return NErr_EndOfFile;

	// make sure we have enough room
	if (WaitForRead((uint64_t)1) == false)
		return error_code;

	if (need_seek)
	{
		_fseeki64(progressive_file_read, position, SEEK_SET);		
		need_seek=false;
	}

	int read_byte = fgetc(progressive_file_read);
	if (read_byte != EOF)
		ungetc(read_byte, progressive_file_read);
	else
	{
		/* TODO: benski> if we hit the point, then we actually need to flush the buffer.
		on some platforms, fflush(progressive_file_read) will do that, but it's not guaranteed! */
		return NErr_EndOfFile;
	}

	*byte = (uint8_t)read_byte;
	return NErr_Success;
}

ns_error_t NXFileObject_ProgressiveDownloader::Sync()
{
	return NErr_NotImplemented;
}

ns_error_t NXFileObject_ProgressiveDownloader::Truncate()
{
	return NErr_NotImplemented;
}

ns_error_t NXFileObject_ProgressiveDownloader::Write(const void *buffer, size_t bytes)
{
	return NErr_NotImplemented;
}

bool NXFileObject_ProgressiveDownloader::Available(uint64_t size, uint64_t *available)
{
	uint64_t end = position+size;
	if (end > region.end)
		end = region.end;
	if (position == region.end)
	{
		if (available)
			*available=0;
		return true;
	}
	return progress_tracker.Valid(position, end, available);
}

ns_error_t NXFileOpenProgressiveDownloader(nx_file_t *out_file, nx_uri_t filename, nx_file_FILE_flags_t flags, jnl_http_t http, const char *user_agent)
{
	NXFileObject_ProgressiveDownloader *file_object = new (std::nothrow) NXFileObject_ProgressiveDownloader;
	if (!file_object)
		return NErr_OutOfMemory;

	ns_error_t ret = file_object->Initialize(filename, http, user_agent);
	if (ret != NErr_Success)
	{
		delete file_object;
		return ret;
	}

	*out_file = (nx_file_t)file_object;
	return NErr_Success;
}

ns_error_t NXFileProgressiveDownloaderAvailable(nx_file_t _f, uint64_t size, uint64_t *available)
{
	if (!_f)
		return NErr_BadParameter;

	NXFileObject_ProgressiveDownloader *f = (NXFileObject_ProgressiveDownloader *)_f;
	if (f->Available(size, available))
		return NErr_True;
	else
		return NErr_False;
}