#pragma once
#include "foundation/foundation.h"
#include "nx/nxuri.h"

#include "service/types.h"
class cb_filelock : public Wasabi2::Dispatchable
{
protected:
	cb_filelock() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~cb_filelock() {}
public:
	int Interrupt() { return FileLockCallback_Interrupt(); }
	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual int WASABICALL FileLockCallback_Interrupt()=0;
};

// {AC2E21C6-7C66-47F6-8C99-267D6CAA1942}
static const GUID file_lock_service_guid = 
{ 0xac2e21c6, 0x7c66, 0x47f6, { 0x8c, 0x99, 0x26, 0x7d, 0x6c, 0xaa, 0x19, 0x42 } };


class api_filelock : public Wasabi2::Dispatchable
{
protected:
	api_filelock() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~api_filelock() {}
public:
	static GUID GetServiceType() { return SVC_TYPE_UNIQUE; }
	static GUID GetServiceGUID() { return file_lock_service_guid; }
	int WaitForRead(nx_uri_t filename) { return FileLock_WaitForRead(filename); }
	int WaitForReadInterruptable(nx_uri_t filename, cb_filelock *callback) { return FileLock_WaitForReadInterruptable(filename, callback); }
	int WaitForWrite(nx_uri_t filename) { return FileLock_WaitForWrite(filename); }
	int WaitForWriteInterruptable(nx_uri_t filename, cb_filelock *callback) { return FileLock_WaitForWriteInterruptable(filename, callback); }
	int UnlockFile(nx_uri_t filename) { return FileLock_UnlockFile(filename); }
	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual int WASABICALL FileLock_WaitForRead(nx_uri_t filename)=0;
	virtual int WASABICALL FileLock_WaitForReadInterruptable(nx_uri_t filename, cb_filelock *callback)=0;
	virtual int WASABICALL FileLock_WaitForWrite(nx_uri_t filename)=0;
	virtual int WASABICALL FileLock_WaitForWriteInterruptable(nx_uri_t filename, cb_filelock *callback)=0;
	virtual int WASABICALL FileLock_UnlockFile(nx_uri_t filename)=0;
};
