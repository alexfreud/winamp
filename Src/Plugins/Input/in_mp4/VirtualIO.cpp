#include "main.h"
#include "VirtualIO.h"
#include "api__in_mp4.h"
#include "api/service/waservicefactory.h"
#include "../../..\Components\wac_network\wac_network_http_receiver_api.h"
#include "../nu/AutoChar.h"
#include "../nu/ProgressTracker.h"

#include <assert.h>
#include <strsafe.h>

#define HTTP_BUFFER_SIZE 65536
// {C0A565DC-0CFE-405a-A27C-468B0C8A3A5C}
static const GUID internetConfigGroupGUID =
{
	0xc0a565dc, 0xcfe, 0x405a, { 0xa2, 0x7c, 0x46, 0x8b, 0xc, 0x8a, 0x3a, 0x5c }
};

static void SetUserAgent(api_httpreceiver *http)
{
	char agent[256] = {0};
	StringCchPrintfA(agent, 256, "User-Agent: %S/%S", WASABI_API_APP->main_getAppName(), WASABI_API_APP->main_getVersionNumString());
	http->addheader(agent);
}

static api_httpreceiver *SetupConnection(const char *url, uint64_t start_position, uint64_t end_position)
{
	api_httpreceiver *http = 0;
	waServiceFactory *sf = mod.service->service_getServiceByGuid(httpreceiverGUID);
	if (sf) http = (api_httpreceiver *)sf->getInterface();

	if (!http)
		return http;

	int use_proxy = 1;
	bool proxy80 = AGAVE_API_CONFIG->GetBool(internetConfigGroupGUID, L"proxy80", false);
	if (proxy80 && strstr(url, ":") && (!strstr(url, ":80/") && strstr(url, ":80") != (url + strlen(url) - 3)))
		use_proxy = 0;

	const wchar_t *proxy = use_proxy?AGAVE_API_CONFIG->GetString(internetConfigGroupGUID, L"proxy", 0):0;
	http->open(API_DNS_AUTODNS, HTTP_BUFFER_SIZE, (proxy && proxy[0]) ? (const char *)AutoChar(proxy) : NULL);
	if (start_position && start_position != (uint64_t)-1)
	{
		if (end_position == (uint64_t)-1)
		{
			char temp[128] = {0};
			StringCchPrintfA(temp, 128, "Range: bytes=%I64u-", start_position);
			http->addheader(temp);
		}
		else
		{
			char temp[128] = {0};
			StringCchPrintfA(temp, 128, "Range: bytes=%I64u-%I64u", start_position, end_position);
			http->addheader(temp);
		}
	}
	SetUserAgent(http);
	http->connect(url);
	return http;
}

static DWORD CALLBACK ProgressiveThread(LPVOID param);

static __int64 Seek64(HANDLE hf, __int64 distance, DWORD MoveMethod)
{
	LARGE_INTEGER li;

	li.QuadPart = distance;

	li.LowPart = SetFilePointer (hf, li.LowPart, &li.HighPart, MoveMethod);

	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
	{
		li.QuadPart = -1;
	}

	return li.QuadPart;
}

int bufferCount;
static void Buffering(int bufStatus, const wchar_t *displayString)
{
	if (bufStatus < 0 || bufStatus > 100)
		return;

	char tempdata[75*2] = {0, };

	int csa = mod.SAGetMode();
	if (csa & 1)
	{
		for (int x = 0; x < bufStatus*75 / 100; x ++)
			tempdata[x] = x * 16 / 75;
	}
	else if (csa&2)
	{
		int offs = (csa & 1) ? 75 : 0;
		int x = 0;
		while (x < bufStatus*75 / 100)
		{
			tempdata[offs + x++] = -6 + x * 14 / 75;
		}
		while (x < 75)
		{
			tempdata[offs + x++] = 0;
		}
	}
	else if (csa == 4)
	{
		tempdata[0] = tempdata[1] = (bufStatus * 127 / 100);
	}
	if (csa)	mod.SAAdd(tempdata, ++bufferCount, (csa == 3) ? 0x80000003 : csa);

	/*
	TODO
	wchar_t temp[64] = {0};
	StringCchPrintf(temp, 64, L"%s: %d%%",displayString, bufStatus);
	SetStatus(temp);
	*/
	//SetVideoStatusText(temp); // TODO: find a way to set the old status back
	//	videoOutput->notifyBufferState(static_cast<int>(bufStatus*2.55f));
}

class ProgressiveReader 
{
public:
	ProgressiveReader(const char *url, HANDLE killswitch) : killswitch(killswitch)
	{
		thread_abort = CreateEvent(NULL, FALSE, FALSE, NULL);
		download_thread = 0;
		progressive_file_read = 0;
		progressive_file_write = 0;

		content_length=0;
		current_position=0;
		stream_disconnected=false;
		connected=false;
		end_of_file=false;

		wchar_t temppath[MAX_PATH-14] = {0}; // MAX_PATH-14 'cause MSDN said so
		GetTempPathW(MAX_PATH-14, temppath);
		GetTempFileNameW(temppath, L"wdl", 0, filename);
		this->url = _strdup(url);
		http = SetupConnection(url, 0, (uint64_t)-1);

		progressive_file_read = CreateFileW(filename, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
		progressive_file_write = CreateFileW(filename, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
		download_thread = CreateThread(0, 0, ProgressiveThread, this, 0, 0);

		while (!connected && !stream_disconnected && WaitForSingleObject(killswitch, 55) == WAIT_TIMEOUT)
		{
			// nop
		}
		Buffer();
	}

	~ProgressiveReader()
	{
		if (download_thread)
		{
			SetEvent(thread_abort);
			WaitForSingleObject(download_thread, INFINITE);
			CloseHandle(download_thread);
		}

		if (thread_abort)
		{
			CloseHandle(thread_abort);
		}

		CloseHandle(progressive_file_read);
		CloseHandle(progressive_file_write);
		DeleteFile(filename);

		if (http)
		{
			waServiceFactory *sf = mod.service->service_getServiceByGuid(httpreceiverGUID);
			if (sf) http = (api_httpreceiver *)sf->releaseInterface(http);
			http=0;
		}
	}

	void Buffer()
	{
		bufferCount=0;
		for (int i=0;i<101;i++)
		{
			Buffering(i, L"Buffering: ");
			WaitForSingleObject(killswitch, 55);
		}
	}

	void OnFinish()
	{
		stream_disconnected=true;
	}


	bool WaitForPosition(uint64_t position, uint64_t size)
	{
		do
		{
			bool valid = progress_tracker.Valid(position, position+size);
			if (valid)
				return true;
			else
			{
				if (position < current_position)
				{
					Reconnect(position, position+size);
				}
				else
				{
					Buffer();
				}
			}
		} while (WaitForSingleObject(killswitch, 0) == WAIT_TIMEOUT);
		return false;
	}

	size_t Read(void *buffer, size_t size)
	{
		if (WaitForPosition(current_position, (uint64_t)size) == false)
			return 0;

		DWORD bytes_read=0;
		ReadFile(progressive_file_read, buffer, size, &bytes_read, NULL);
		current_position += bytes_read;
		return bytes_read;		
	}

	uint64_t GetFileLength()
	{
		return content_length;
	}

	void Reconnect(uint64_t position, uint64_t end)
	{
		SetEvent(thread_abort);
		WaitForSingleObject(download_thread, INFINITE);
		ResetEvent(thread_abort);

		uint64_t new_start, new_end;
		progress_tracker.Seek(position, end, &new_start, &new_end);

		CloseHandle(download_thread);
		stream_disconnected=false;
		connected=false;
		if (http)
		{
			waServiceFactory *sf = mod.service->service_getServiceByGuid(httpreceiverGUID);
			if (sf) http = (api_httpreceiver *)sf->releaseInterface(http);
			http=0;
		}

		http = SetupConnection(url, new_start, new_end);
		Seek64(progressive_file_write, new_start, SEEK_SET);
		download_thread = CreateThread(0, 0, ProgressiveThread, this, 0, 0);
		while (!connected && !stream_disconnected && WaitForSingleObject(killswitch, 55) == WAIT_TIMEOUT)
		{
			// nop
		}
		Buffer();
	}

	int SetPosition(uint64_t position)
	{
		if (position == content_length)
		{
			end_of_file=true;
		}
		else
		{
			if (!progress_tracker.Valid(position, position))
			{
				Reconnect(position, (uint64_t)-1);
			}
			current_position = Seek64(progressive_file_read, position, SEEK_SET);
			end_of_file=false;
		}
		return 0;
	}

	int GetPosition(uint64_t *position)
	{
		if (end_of_file)
			*position = content_length;
		else
			*position = current_position;
		return 0;
	}

	int EndOfFile()
	{
		return !!stream_disconnected;
	}

	int Close()
	{
		SetEvent(thread_abort);
		while (!stream_disconnected && WaitForSingleObject(killswitch, 55) == WAIT_TIMEOUT)
		{
			// nop
		}
		return 0;
	}

	/* API used by download thread */
	void Write(const void *data, size_t data_len)
	{
		DWORD bytes_written = 0;
		WriteFile(progressive_file_write, data, data_len, &bytes_written, 0);
		progress_tracker.Write(data_len);
	}

	int Wait(int milliseconds)
	{
		HANDLE handles[] = {killswitch, thread_abort};
		int ret = WaitForMultipleObjects(2, handles, FALSE, milliseconds);
		if (ret == WAIT_OBJECT_0+1)
			return 1;
		else if (ret == WAIT_TIMEOUT)
			return 0;
		else
			return -1;
	}

	int DoRead(void *buffer, size_t bufferlen)
	{
		int ret = http->run();
		int bytes_received;
		do
		{
			ret = http->run();
			bytes_received= http->get_bytes(buffer, bufferlen);
			if (bytes_received)
				Write(buffer, bytes_received);
		} while (bytes_received);
		return ret;
	}

	int Connect()
	{
		do
		{
			int ret = http->run();
			if (ret == -1) // connection failed
				return ret;

			// ---- check our reply code ----
			int replycode = http->getreplycode();
			switch (replycode)
			{
			case 0:
			case 100:
				break;
			case 200:
			case 206:
				{

					const char *content_length_header = http->getheader("Content-Length");
					if (content_length_header)
					{
						uint64_t new_content_length = _strtoui64(content_length_header, 0, 10);
						//InterlockedExchange64((volatile LONGLONG *)&content_length, new_content_length);
						content_length = new_content_length; // TODO interlock on win32
					}
					connected=true;

					return 0;
				}
			default:
				return -1;
			}
		}
		while (Wait(55) == 0);
		return 0;
	}

private:
	uint64_t current_position;
	volatile uint64_t content_length;
	bool end_of_file;
	bool stream_disconnected;
	bool connected;
	char *url;
	wchar_t filename[MAX_PATH];
	HANDLE progressive_file_read, progressive_file_write;
	ProgressTracker progress_tracker;
	HANDLE killswitch;
	HANDLE download_thread;
	api_httpreceiver *http;
	HANDLE thread_abort;
};

static DWORD CALLBACK ProgressiveThread(LPVOID param)
{
	ProgressiveReader *reader = (ProgressiveReader *)param;

	if (reader->Connect() == 0)
	{
		int ret = 0;
		while (ret == 0)
		{
			ret=reader->Wait(10);
			if (ret >= 0)
			{
				char buffer[HTTP_BUFFER_SIZE] = {0};
				reader->DoRead(buffer, sizeof(buffer));
			}
		}
	}
	reader->OnFinish();

	return 0;
}

u_int64_t HTTPGetFileLength(void *user)
{
	ProgressiveReader *reader = (ProgressiveReader *)user;
	return reader->GetFileLength();
}

int HTTPSetPosition(void *user, u_int64_t position)
{
	ProgressiveReader *reader = (ProgressiveReader *)user;
	return reader->SetPosition(position);
}

int HTTPGetPosition(void *user, u_int64_t *position)
{
	ProgressiveReader *reader = (ProgressiveReader *)user;
	return reader->GetPosition(position);
}

size_t HTTPRead(void *user, void *buffer, size_t size)
{
	ProgressiveReader *reader = (ProgressiveReader *)user;
	return reader->Read(buffer, size);
}

size_t HTTPWrite(void *user, void *buffer, size_t size)
{
	return 1;
}

int HTTPEndOfFile(void *user)
{
	ProgressiveReader *reader = (ProgressiveReader *)user;
	return reader->EndOfFile();
}

int HTTPClose(void *user)
{
	ProgressiveReader *reader = (ProgressiveReader *)user;
	return reader->Close();
}

Virtual_IO HTTPIO =
{
	HTTPGetFileLength,
	HTTPSetPosition,
	HTTPGetPosition,
	HTTPRead,
	HTTPWrite,
	HTTPEndOfFile,
	HTTPClose,
};

void *CreateReader(const wchar_t *url, HANDLE killswitch)
{

	if ( WAC_API_DOWNLOADMANAGER )
	{
		return new ProgressiveReader(AutoChar(url), killswitch);
	}
	else
		return 0; 
}

void DestroyReader(void *user)
{
	ProgressiveReader *reader = (ProgressiveReader *)user;
	delete reader;
}

void StopReader(void *user)
{
	ProgressiveReader *reader = (ProgressiveReader *)user;
	reader->Close();
}

/* ----------------------------------- */

struct Win32_State
{
	Win32_State() 
	{
		memset(buffer, 0, sizeof(buffer));
		handle=0;
		endOfFile=false;
		position.QuadPart = 0;
		event = CreateEvent(NULL, TRUE, TRUE, NULL);
		read_offset=0;
		io_active=false;
	}
	~Win32_State()
	{
		if (handle && handle != INVALID_HANDLE_VALUE)
			CancelIo(handle);
		CloseHandle(event);
	}
	//	void *userData;
	HANDLE handle;
	bool endOfFile;
	LARGE_INTEGER position;
	HANDLE event;
	OVERLAPPED overlapped;
	DWORD read_offset;
	bool io_active;
	char buffer[16384];
};

static __int64 FileSize64(HANDLE file)
{
	LARGE_INTEGER position;
	position.QuadPart=0;
	position.LowPart = GetFileSize(file, (LPDWORD)&position.HighPart); 	

	if (position.LowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
		return INVALID_FILE_SIZE;
	else
		return position.QuadPart;
}

u_int64_t UnicodeGetFileLength(void *user)
{
	Win32_State *state = static_cast<Win32_State *>(user);
	assert(state->handle);
	return FileSize64(state->handle);
}



int UnicodeSetPosition(void *user, u_int64_t position)
{
	Win32_State *state = static_cast<Win32_State *>(user);
	assert(state->handle);
	__int64 diff = position - state->position.QuadPart;

	if ((diff+state->read_offset) >= sizeof(state->buffer)
		|| (diff+state->read_offset) < 0)
	{
		CancelIo(state->handle);
		state->io_active = 0;
		state->read_offset = 0;
	}
	else if (diff)
		state->read_offset += (DWORD)diff;

	state->position.QuadPart = position;
	state->endOfFile = false;
	return 0;
}

int UnicodeGetPosition(void *user, u_int64_t *position)
{
	Win32_State *state = static_cast<Win32_State *>(user);
	assert(state->handle);
	*position = state->position.QuadPart;
	return 0;
}

static void DoRead(Win32_State *state)
{
	WaitForSingleObject(state->event, INFINITE);
	state->overlapped.hEvent = state->event;
	state->overlapped.Offset = state->position.LowPart;
	state->overlapped.OffsetHigh = state->position.HighPart;
	state->read_offset = 0;
	ResetEvent(state->event);
	ReadFile(state->handle, state->buffer, sizeof(state->buffer), NULL, &state->overlapped);
	//int error = GetLastError();//ERROR_IO_PENDING = 997
	state->io_active=true;
}

size_t UnicodeRead(void *user, void *buffer, size_t size)
{
	Win32_State *state = static_cast<Win32_State *>(user);
	assert(state->handle);
	size_t totalRead=0;
	HANDLE file = state->handle;
	if (!state->io_active)
	{
		DoRead(state);
	}

	if (state->read_offset == sizeof(state->buffer))
	{
		DoRead(state);
	}

	while (size > (sizeof(state->buffer) - state->read_offset))
	{
		DWORD bytesRead=0;
		BOOL res = GetOverlappedResult(file, &state->overlapped, &bytesRead, TRUE);
		if ((res && bytesRead != sizeof(state->buffer))
			|| (!res && GetLastError() == ERROR_HANDLE_EOF))
		{
			state->endOfFile = true;
		}

		if (bytesRead > state->read_offset)
		{
			size_t bytesToCopy = bytesRead-state->read_offset;
			memcpy(buffer, state->buffer + state->read_offset, bytesToCopy);
			buffer=(uint8_t *)buffer + bytesToCopy;
			totalRead+=bytesToCopy;
			size-=bytesToCopy;


			if (state->endOfFile)
				return totalRead;

			state->position.QuadPart += bytesToCopy;
			DoRead(state);
		}
		else
			break;
	}

	while (1)
	{
		DWORD bytesRead=0;
		BOOL res = GetOverlappedResult(file, &state->overlapped, &bytesRead, FALSE);
		if ((res && bytesRead != sizeof(state->buffer))
			|| (!res && GetLastError() == ERROR_HANDLE_EOF))
		{
			state->endOfFile = true;
		}
		if (bytesRead >= (size + state->read_offset))
		{
			memcpy(buffer, state->buffer + state->read_offset, size);
			state->read_offset += size;
			totalRead+=size;
			state->position.QuadPart += size;
			break;
		}

		if (state->endOfFile)
			break;

		WaitForSingleObject(state->event, 10); // wait 10 milliseconds or when buffer is done, whichever is faster
	}

	return totalRead;
}

size_t UnicodeWrite(void *user, void *buffer, size_t size)
{
	Win32_State *state = static_cast<Win32_State *>(user);
	DWORD written = 0;
	assert(state->handle);
	WriteFile(state->handle, buffer, size, &written, NULL);
	return 0;
}

int UnicodeEndOfFile(void *user)
{
	Win32_State *state = static_cast<Win32_State *>(user);
	return state->endOfFile;
}

int UnicodeClose(void *user)
{
	Win32_State *state = static_cast<Win32_State *>(user);
	if (state->handle)
		CloseHandle(state->handle);

	state->handle=0;
	return 0;
}

Virtual_IO UnicodeIO =
{
	UnicodeGetFileLength,
	UnicodeSetPosition,
	UnicodeGetPosition,
	UnicodeRead,
	UnicodeWrite,
	UnicodeEndOfFile,
	UnicodeClose,
};


void *CreateUnicodeReader(const wchar_t *filename)
{
	HANDLE fileHandle = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
	if (fileHandle == INVALID_HANDLE_VALUE)
		return 0;

	Win32_State *state = new Win32_State;
	if (!state)
	{
		CloseHandle(fileHandle);
		return 0;
	}
	state->endOfFile = false;
	state->handle = fileHandle;
	return state;
}

void DestroyUnicodeReader(void *reader)
{
	if (reader) // need to check because of the cast
		delete (Win32_State *)reader;
}
