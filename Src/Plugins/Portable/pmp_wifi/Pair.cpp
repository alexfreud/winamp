#include "Pair.h"
#include "api.h"
#include "../../..\Components\wac_network\wac_network_http_receiver_api.h"
#include "main.h"
#include "WifiDevice.h"
#include <strsafe.h>

PairDownloader::PairDownloader(WifiDevice *device) : device(device)
{
	device->AddRef();
}

PairDownloader::~PairDownloader()
{
}

void PairDownloader::OnInit(DownloadToken token)
{
	api_httpreceiver *jnet = WAC_API_DOWNLOADMANAGER->GetReceiver(token);
	if (jnet)
	{
		jnet->AddHeaderValue("X-Winamp-ID", winamp_id_str);
		jnet->AddHeaderValue("X-Winamp-Name", winamp_name);
	}
}

void PairDownloader::OnData(DownloadToken token, void *data, size_t datalen)
{
}

void PairDownloader::OnCancel(DownloadToken token)
{
	device->OnConnectionFailed();
	device->Release();
	Release();
}

void PairDownloader::OnError(DownloadToken token, int error)
{
	api_httpreceiver *jnet = WAC_API_DOWNLOADMANAGER->GetReceiver(token);
	if (jnet)
	{
		jnet->getreplycode();
	}
	device->OnConnectionFailed();
	device->Release();
	Release();
}

void PairDownloader::OnFinish(DownloadToken token)
{
	api_httpreceiver *jnet = WAC_API_DOWNLOADMANAGER->GetReceiver(token);
	if (jnet)
	{
		if (jnet->getreplycode() == 202)
		{
			device->OnPaired();
			device->Release();
			Release();
			return;
		}
	}
	device->OnConnectionFailed();
	device->Release();
	Release();
}

bool IsPaired(uint64_t id)
{
	wchar_t pair_name[64] = {0};
	StringCbPrintfW(pair_name, sizeof(pair_name), L"%016I64x", id);
	if (GetPrivateProfileInt(L"pairs", pair_name, 0, inifile) == 0)
		return false;

	return true;
}

void SetPaired(uint64_t id, bool status)
{
	wchar_t pair_name[64] = {0};
	StringCbPrintfW(pair_name, sizeof(pair_name), L"%016I64x", id);
	if (status)
		WritePrivateProfileString(L"pairs", pair_name, L"1", inifile);
	else
		WritePrivateProfileString(L"pairs", pair_name, L"0", inifile);
	
}