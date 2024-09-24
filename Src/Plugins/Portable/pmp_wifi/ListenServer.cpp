#include "api.h"
#include "..\..\..\replicant\jnetlib\multicastlisten.h"
#include "jnetlib/util.h"
#include "InfoDownloader.h"
#include "nu/AutoChar.h"
#include <vector>
#include "nu/AutoLock.h"

namespace Wasabi2
{
	#include "ssdp/cb_ssdp.h"
}

#include <windows.h>

static nu::LockGuard connections_guard;

struct Connection
{
	Connection()
	{
		usn        = 0;
		location   = 0;
		device     = 0;
		downloader = 0;
	}

	Wasabi2::nx_string_t usn;
	Wasabi2::nx_uri_t location;
	WifiDevice *device;
	InfoDownloader *downloader;
};

typedef std::vector<Connection> Connections;
static Connections connections;
static volatile int killswitch;

static Connection &FindUSN(Wasabi2::nx_string_t usn)
{
	for (size_t i=0;i<connections.size();i++)
	{
		if (!Wasabi2::NXStringKeywordCompare(connections[i].usn, usn))
		{
			return connections[i];
		}
	}

	Connection dummy;
	dummy.usn = NXStringRetain(usn);
	connections.push_back(dummy);

	return connections[connections.size()-1];
}

class WAFAListener : public Wasabi2::cb_ssdp
{
	void WASABICALL SSDPCallback_OnServiceConnected(Wasabi2::nx_uri_t location, Wasabi2::nx_string_t type, Wasabi2::nx_string_t usn);
	void WASABICALL SSDPCallback_OnServiceDisconnected(Wasabi2::nx_string_t usn);
};

void WAFAListener::SSDPCallback_OnServiceConnected(Wasabi2::nx_uri_t location, Wasabi2::nx_string_t type, Wasabi2::nx_string_t usn)
{
	if (!Wasabi2::NXStringKeywordCompareWithCString(type, "urn:nullsoft.com:device:Android:1"))
	{
		nu::AutoLock auto_lock(connections_guard);
		Connection &connection = FindUSN(usn);
		connection.location = Wasabi2::NXURIRetain(location);

		uint64_t id = _wcstoui64(usn->string, 0, 16);
		AutoChar cached_location(location->string);
		InfoDownloader *downloader = new InfoDownloader(cached_location, id, usn);
		connection.downloader = downloader;
		WAC_API_DOWNLOADMANAGER->DownloadEx(cached_location, downloader, api_downloadManager::DOWNLOADEX_CALLBACK);
	}		
}

void WAFAListener::SSDPCallback_OnServiceDisconnected(Wasabi2::nx_string_t usn)
{
	nu::AutoLock auto_lock(connections_guard);
	Connection &connection = FindUSN(usn);
	if (connection.device)
	{
		connection.device->OnDisconnect();
		connection.device->Release();
		connection.device = 0;
	}

	if (connection.downloader)
	{
		connection.downloader->Cancel();
		connection.downloader->Release();
		connection.downloader = 0;
	}
}

void OnInfoDownloadDone(InfoDownloader *_info)
{
	nu::AutoLock auto_lock(connections_guard);
	Connection &connection = FindUSN(_info->usn);
	WifiDevice *device = 0;
	if (connection.downloader && connection.downloader->Done(&device))
	{
		connection.device = device;
		connection.downloader->Release();
		connection.downloader = 0;
	}
	else
	{
		if (connection.location)
		{
			// requeue download request, TODO: but might want to wait a bit
			AutoChar cached_location(connection.location->string);
			WAC_API_DOWNLOADMANAGER->DownloadEx(cached_location, connection.downloader, api_downloadManager::DOWNLOADEX_CALLBACK);
		}
	}
}

static WAFAListener wafa_ssdp_callback;

void StartListenServer()
{
	if (REPLICANT_API_SSDP)
	{
		REPLICANT_API_SSDP->RegisterCallback(&wafa_ssdp_callback);
	}
}

void StopListenServer()
{
	nu::AutoLock auto_lock(connections_guard);
	for (size_t i=0;i<connections.size();i++)
	{
		NXStringRelease(connections[i].usn);
		NXURIRelease(connections[i].location);

		if (connections[i].downloader)
		{
			connections[i].downloader->Cancel();
			connections[i].downloader->Release();
		}

		if (connections[i].device)
			connections[i].device->Release();
	}

	connections.clear();
}