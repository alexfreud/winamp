#define SKIP_OVER
#include "main.h"
#include "api__in_cdda.h"

#include "CDPlay.h"
#include "DAEPlay.h"
#include "MCIPlay.h"
#include "WindacPlay.h"

#include "cddb.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "../nde/ndestring.h"
#include "../nu/ListView.h"
#ifndef IGNORE_API_GRACENOTE
#include "../primo/obj_primo.h"
#endif
#include "../Winamp/wa_ipc.h"
#include <api/service/waservicefactory.h>
#include <shlwapi.h>
#include <atlbase.h>
#include <strsafe.h>

#if 0
BOOL CALLBACK ripConfigProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

static int cachedev_used;
static MCIDEVICEID cachedev;
static wchar_t last_fn[8];
extern "C" __declspec(dllexport) int winampGetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *dest, int destlen)
{
	s_last_error = NULL;
	if (!_stricmp(data, "type"))
	{
		lstrcpynW(dest, L"0", destlen); //audio
		return 1;
	}
	else if (!_stricmp(data, "family"))
	{
		LPCWSTR e, p(NULL);
		DWORD lcid;
		e = PathFindExtensionW(fn);
		if (L'.' != *e) return 0;
		e++;
		lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
		if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"CDA", -1)) p = WASABI_API_LNGSTRINGW(IDS_FAMILY_STRING);
		if (p && S_OK == StringCchCopyW(dest, destlen, p)) return 1;
		return 0;
	}
	else if (!_stricmp(data, "ext_cdda"))
	{
		lstrcpynW(dest, L"1", destlen); //audio
		return 1;
	}

	// TODO determine if we even keep any of this...
	/*
	else if (!_stricmp(data, "cdda_config_text"))
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_RIPPING,dest,destlen);
		return 1;
	}
	else if (!_strnicmp(data, "cdda_cf_", 8))
	{
		HWND parent = (HWND)atoi(data + 8);
		dest[0] = 0;
		if (parent && IsWindow(parent))
		{
			parent = WASABI_API_CREATEDIALOGW(IDD_PREFS_CDRIP, parent, ripConfigProc);
			_itow((int)parent, dest, 10);
		}
		return 1;
	}
	*/

	if (!lstrcmpiW(PathFindExtensionW(fn), L".cda") && !_wcsnicmp(fn + 1, L":\\track", 7)) // stupid hack, converts x:\\trackXX.cda to cda://x,XX
	{
		static wchar_t fakebuf[128];
		StringCchPrintf(fakebuf, 128, L"cda://%c,%d", fn[0], _wtoi(PathFindFileNameW(fn) + 5));
		fn = fakebuf;
	}

	if (!_wcsnicmp(fn, L"cda://", 6)) // determine length of cd track via MCI
	{
		int track = lstrlenW(fn) > 8 ? _wtoi(fn + 8) : 0;
		int device = fn[6];
		if (device >= 'a' && device <= 'z') device += 'A' -'a';
		MCIDEVICEID dev2 = 0;

		if (cachedev_used) dev2 = cachedev;

		if (!_stricmp(data, "discid") || !_stricmp(data, "cddbid"))
		{
			dest[0] = 0;
#ifdef DISCID
			DiscId *disc = discid_new();
			wchar_t drive[4] = {device, L':'};

			if (!discid_read_sparse(disc, drive, 0)) {
				discid_free(disc);
				return 0;
			}

			if (!_stricmp(data, "cddbid"))
				lstrcpynW(dest, AutoWide(discid_get_freedb_id(disc)), destlen);
			else
				lstrcpynW(dest, AutoWide(discid_get_id(disc)), destlen);

			discid_free(disc);
#endif
		}
		else if (!_stricmp(data, "cdengine"))
		{
			dest[0] = 0;
			if (g_cdplay && device && g_cdplay->IsPlaying(device))
			{
				if (g_cdplay == daePlayer)
					lstrcpynW(dest, L"DAE", destlen);
				else if (g_cdplay == windacPlayer)
				{
					if (hDLL)
						lstrcpynW(dest, L"ASPI", destlen);
					else
						lstrcpynW(dest, L"SPTI", destlen);
				}
				else if (g_cdplay == mciPlayer)
					lstrcpynW(dest, L"MCI", destlen);
			}
			return 1;
		}
		else if (!_stricmp(data, "<begin>"))
		{
			if (!CDOpen(&cachedev, device, L"cache"))
				cachedev = 0;
			
			if (NULL != dest && destlen > 1)
			{
				dest[0] = (NULL != cachedev) ? L'1' : L'0';
				dest[1] = L'\0';
			}
			
			cachedev_used = 1;
			//OutputDebugString("begin device caching\n");
		}
		else if (!_stricmp(data, "<end>"))
		{
			if (cachedev_used && cachedev)
				CDClose(&cachedev);
			//OutputDebugString("end device caching\n");
			cachedev_used = 0;
			cachedev = 0;
		}
		else if (!_stricmp(data, "<eject>"))
		{
			if (!cachedev_used)
			{
				if (!CDOpen(&dev2, device, L"eject"))
					dev2 = 0;
			}
			if (dev2)
			{
				CDEject(dev2);
				if (dev2 != cachedev)
					CDClose(&dev2);
			}
		}
		else if (!_stricmp(data, "ntracks"))
		{
			if (!cachedev_used)
			{
				if (!CDOpen(&dev2, device, L"ntracks")) dev2 = 0;
			}
			if (dev2)
			{
				_itow(CDGetTracks(dev2), dest, 10);
				if (dev2 != cachedev) CDClose(&dev2);
			}
			else
			{
				if (NULL != dest && destlen > 1)
				{
					dest[0] = L'0';
					dest[1] = L'\0';
				}
			}
		}
		else if (!_stricmp(data, "tracktype"))
		{
			if (!cachedev_used)
			{
				if (!CDOpen(&dev2, device, L"tracktype")) dev2 = 0;
			}
			if (dev2)
			{
				MCI_STATUS_PARMS sMCIStatus;
				sMCIStatus.dwItem = MCI_CDA_STATUS_TYPE_TRACK;
				sMCIStatus.dwTrack = track;
				if (mciSendCommand(dev2, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, (DWORD_PTR) &sMCIStatus))
					WASABI_API_LNGSTRINGW_BUF(IDS_UNKNOWN,dest,destlen);
				else lstrcpynW(dest, (sMCIStatus.dwReturn != MCI_CDA_TRACK_AUDIO ? L"data" : L"audio"), destlen);

				if (dev2 != cachedev) CDClose(&dev2);
			}
		}
		else if (!_stricmp(data, "length"))
		{
			if (!cachedev_used)
			{
				if (!CDOpen(&dev2, device, L"length")) dev2 = 0;
			}
			if (dev2)
			{
				MCI_SET_PARMS sMCISet;
				sMCISet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
				MCISendCommand(dev2, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)(LPVOID) &sMCISet);
				_itow(CDGetTrackLength(dev2, track), dest, 10);
				sMCISet.dwTimeFormat = MCI_FORMAT_TMSF;
				MCISendCommand(dev2, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)(LPVOID) &sMCISet);

				if (dev2 != cachedev)
					CDClose(&dev2);
			}
		}
		else if (!_stricmp(data, "album") || !_stricmp(data, "artist") ||
				 !_stricmp(data, "year") || !_stricmp(data, "genre") ||
				 !_stricmp(data, "title") || !_stricmp(data, "comment") ||
				 !_stricmp(data, "tuid") || !_stricmp(data, "albumartist") ||
				 !_stricmp(data, "publisher") || !_stricmp(data, "disc") ||
				 !_stricmp(data, "conductor") || !_stricmp(data, "composer") ||
				 !_stricmp(data, "remixing") || !_stricmp(data, "isrc") ||
				 !_stricmp(data, "GracenoteFileID") || !_stricmp(data, "GracenoteExtData")
			)
		{
			int cached = 0;
			//cache our myps
			static DINFO myps;
			/*static unsigned int last_time;
			unsigned int now = GetTickCount();*/
			cached = !_wcsnicmp(fn, last_fn, 7);
			// TODO disabled as this is causing more access issues than it seems to help...
			/*if (cached)
			{
				if (now > last_time + 1000) cached = 0;
				if (now < last_time - 1000) cached = 0; // counter wrapped
			}*/

			if (!cached)
			{
				lstrcpynW(last_fn, fn, 8);

				memset(&myps, 0, sizeof(myps));
				if (!cachedev_used)
				{
					if (CDOpen(&dev2, device, L"extinfo"))
					{
						GetDiscID(dev2, &myps);
						CDClose(&dev2);
					}
				}
				else GetDiscID(dev2, &myps);
			}

			if (myps.CDDBID)
			{
				// try to get CDDB information and then revert to CD-Text if CDDB is not available
				if (GetCDDBInfo(&myps, device))
				{
					wchar_t cache[] = {L"cda://x"};
					cache[6] = device;
					PostMessage(line.hMainWindow, WM_WA_IPC, (WPARAM)cache, IPC_REFRESHPLCACHE);
				}
				else if (!cached && !myps.populated)
				{
					if (DoCDText(&myps, device))
					{
						wchar_t cache[] = {L"cda://x"};
						cache[6] = device;
						PostMessage(line.hMainWindow, WM_WA_IPC, (WPARAM)cache, IPC_REFRESHPLCACHE);
					}
				}

				if (!_stricmp(data, "album"))
				{
					if (myps.title)
						lstrcpynW(dest, myps.title, destlen);
					else
						dest[0]=0;
				}
				else if (!_stricmp(data, "artist") && track>0 && track<100)
				{
					if (myps.tracks[track-1].artist)
						lstrcpynW(dest, myps.tracks[track-1].artist, destlen);
					else if (myps.artist)
						lstrcpynW(dest, myps.artist, destlen);
					else
						dest[0]=0;
				}
				else if (!_stricmp(data, "albumartist"))
				{
					if (myps.artist)
						lstrcpynW(dest, myps.artist, destlen);
					else if (track == 0)
						WASABI_API_LNGSTRINGW_BUF(IDS_UNKNOWN_ARTIST,dest,destlen);
					else
						dest[0]=0;
				}
				else if (!_stricmp(data, "title") && track>0 && track<100)
				{
					if (myps.tracks[track-1].title)
						lstrcpynW(dest, myps.tracks[track-1].title, destlen);
					else
						StringCchPrintfW(dest, destlen, WASABI_API_LNGSTRINGW(IDS_TRACK_X), track);
				}
				else if (!_stricmp(data, "year"))
				{
					if (myps.year)
						lstrcpynW(dest, myps.year, destlen);
					else
						dest[0]=0;
				}
				else if (!_stricmp(data, "genre"))
				{
					if (myps.genre)
						lstrcpynW(dest, myps.genre, destlen);
					else
						dest[0]=0;
				}
				else if (!_stricmp(data, "comment"))
				{
					if (myps.notes)
						lstrcpynW(dest, myps.notes, destlen);
					else
						dest[0]=0;
				}
				else if (!_stricmp(data, "publisher"))
				{
					if (myps.label)
						lstrcpynW(dest, myps.label, destlen);
					else
						dest[0]=0;
				}
				else if (!_stricmp(data, "composer"))
				{
					if (track>0 && myps.tracks[track-1].composer)
						lstrcpynW(dest,  myps.tracks[track-1].composer, destlen);
					else if (myps.composer)
						lstrcpynW(dest,  myps.composer, destlen);
					else
						dest[0]=0;
				}
				else if (!_stricmp(data, "remixing"))
				{
					if (track>0 && myps.tracks[track-1].remixing)
						lstrcpynW(dest,  myps.tracks[track-1].remixing, destlen);
					else if (myps.composer)
						lstrcpynW(dest,  myps.remixing, destlen);
					else
						dest[0]=0;
				}
				else if (!_stricmp(data, "conductor"))
				{
					if (track>0 && myps.tracks[track-1].conductor)
						lstrcpynW(dest,  myps.tracks[track-1].conductor, destlen);
					else if (myps.conductor)
						lstrcpynW(dest,  myps.conductor, destlen);
					else
						dest[0]=0;
				}
				else if (!_stricmp(data, "isrc") && track>0 && track<100)
				{
					if (myps.tracks[track-1].isrc)
						lstrcpynW(dest, myps.tracks[track-1].isrc, destlen);
					else
						dest[0]=0;
				}
				else if (!_stricmp(data, "GracenoteFileID") && track>0 && track<100)
				{
					if (myps.tracks[track-1].tagID)
						lstrcpynW(dest, myps.tracks[track-1].tagID, destlen);
					else
						dest[0]=0;
				}
				else if (!_stricmp(data, "GracenoteExtData") && track>0 && track<100)
				{
					if (myps.tracks[track-1].extData)
						lstrcpynW(dest,  myps.tracks[track-1].extData, destlen);
					else
						dest[0]=0;
				}
				else if (!_stricmp(data, "tuid"))
				{
					if (myps.tuid)
						lstrcpynW(dest, myps.tuid, destlen);
					else
						dest[0]=0;
				}
				else if (!_stricmp(data, "disc"))
				{
					if (myps.numdiscs > 0)
						StringCchPrintf(dest, destlen, L"%u/%u", myps.discnum, myps.numdiscs);
					else if (myps.discnum)
						StringCchPrintf(dest, destlen, L"%u", myps.discnum);
					else
						dest[0] = 0;
				}
				else
				{
//					last_time = GetTickCount();
					return 0;// some error condition (such as track == 0) got us here
				}
			}
			else
			{
				if (!_stricmp(data, "title") && track>0 && track<100)
				{
					StringCchPrintfW(dest, destlen, WASABI_API_LNGSTRINGW(IDS_TRACK_X), track);
				}
				last_fn[0] = 0x00;
			}
//			last_time = GetTickCount();
		}
		else if (!_stricmp(data, "cdtype"))
		{
			dest[0] = 0;
			#ifndef IGNORE_API_GRACENOTE
			DWORD retCode = PRIMOSDK_OK + 1;
			obj_primo *primo=0;
			waServiceFactory *sf = line.service->service_getServiceByGuid(obj_primo::getServiceGuid());
			if (sf) primo = reinterpret_cast<obj_primo *>(sf->getInterface());
			if (primo)
			{
				DWORD unit = (DWORD)fn[6];
				DWORD mediumtype;
				DWORD mediumformat;
				DWORD erasable;
				DWORD tracks, used, free;
				retCode = primo->DiscInfo(&unit, &mediumtype, &mediumformat, &erasable, &tracks, &used, &free);
				if (retCode == PRIMOSDK_OK)
				{
					if (mediumformat > 0xf0 && mediumformat < 0xff) lstrcpynW(dest, L"DVD", destlen);
					else lstrcpynW(dest, L"CD", destlen);
					if (mediumtype == PRIMOSDK_BLANK || mediumtype == PRIMOSDK_COMPLIANTGOLD) StringCchCatW(dest, destlen, L"R");
					if (erasable) StringCchCatW(dest, destlen, L"W");
				}
				sf->releaseInterface(primo);
			}
			return (retCode == PRIMOSDK_OK);
			#else
			return 0;
			#endif
		}
		else if (!_stricmp(data, "cdtype2"))
		{
			dest[0] = 0;
			#ifndef IGNORE_API_GRACENOTE
			DWORD retCode = PRIMOSDK_OK + 1;
			obj_primo *primo=0;
			waServiceFactory *sf = line.service->service_getServiceByGuid(obj_primo::getServiceGuid());
			if (sf) primo = reinterpret_cast<obj_primo *>(sf->getInterface());
			if (primo)
			{
				DWORD unit = (DWORD)fn[6];
				DWORD mediumType;
				DWORD mediumFormat;
				DWORD erasable;
				DWORD tracks, used, free;
				DWORD medium = -1, protectedDVD = -1, mediumEx = -1;
				DWORD rfu;
				retCode = primo->DiscInfoEx(&unit, 1,  &mediumType, &mediumFormat, &erasable, &tracks, &used, &free);
				if (retCode == PRIMOSDK_OK)
				{
					primo->DiscInfo2(&unit, &medium, &protectedDVD, NULL, &mediumEx, &rfu);
					//format:  mediumType;mediumFormat;mediumEx;protectedDVD;erasable;tracks;used;free
					StringCchPrintfW(dest, destlen, L"%d;%d;%d;%d;%d;%d;%d;%d", mediumType, mediumFormat, mediumEx, protectedDVD, erasable, tracks, used, free);
				}
				sf->releaseInterface(primo);
			}
			return (retCode == PRIMOSDK_OK);
			#else
			return 0;
			#endif
		}
		else if (!_stricmp(data, "cdlengths"))
		{
			dest[0] = 0;
			#ifndef IGNORE_API_GRACENOTE
			obj_primo *primo=0;
			waServiceFactory *sf = line.service->service_getServiceByGuid(obj_primo::getServiceGuid());
			if (sf) primo = reinterpret_cast<obj_primo *>(sf->getInterface());
			if (primo)
			{
				DWORD unit = (DWORD)fn[6];
				DWORD mediumtype;
				DWORD mediumformat;
				DWORD erasable;
				DWORD tracks, used, free;
				if (primo->DiscInfo(&unit, &mediumtype, &mediumformat, &erasable, &tracks, &used, &free) == PRIMOSDK_OK)
				{
					StringCchPrintfW(dest, destlen, L"%d,%d", free, used);
				}
				sf->releaseInterface(primo);
			}
			#endif
		}
		else if (!_stricmp(data, "cdspeeds"))
		{
			dest[0] = 0;
			#ifndef IGNORE_API_GRACENOTE
			DWORD retCode = PRIMOSDK_OK + 1;
			obj_primo *primo=0;
			waServiceFactory *sf = line.service->service_getServiceByGuid(obj_primo::getServiceGuid());
			if (sf) primo = reinterpret_cast<obj_primo *>(sf->getInterface());
			if (primo)
			{
				DWORD unit = (DWORD)fn[6];
				DWORD cdspeeds[32];
				DWORD dvdspeeds[32];
				DWORD capabilities;
				retCode = primo->UnitSpeeds(&unit, (unsigned long *) & cdspeeds, (unsigned long *) & dvdspeeds, &capabilities);
				if (retCode == PRIMOSDK_OK)
				{
					wchar_t *p = dest;
					//reading speeds
					int i;
					for (i = 0;cdspeeds[i] != 0xFFFFFFFF;i++);
					i++;
					//CD-R speeds
					for (;cdspeeds[i] != 0xFFFFFFFF;i++)
					{
						StringCchPrintfW(p, destlen, L"%d", cdspeeds[i]);
						StringCchCatW(p, destlen, L"/");
						p += lstrlenW(p);
					}
					*p = ';';
					p++;
					i++;
					//CD-RW speeds
					for (;cdspeeds[i] != 0xFFFFFFFF;i++)
					{
						StringCchPrintfW(p, destlen, L"%d", cdspeeds[i]);
						StringCchCatW(p, destlen, L"/");
						p += lstrlenW(p);
					}
				}
				sf->releaseInterface(primo);
			}
			return (retCode == PRIMOSDK_OK);
			#else
			return 0;
			#endif
		}
		else if (!_stricmp(data, "cdinfo"))
		{
			dest[0] = 0;
			#ifndef IGNORE_API_GRACENOTE
			DWORD retCode = PRIMOSDK_OK + 1;
			obj_primo *primo=0;
			waServiceFactory *sf = line.service->service_getServiceByGuid(obj_primo::getServiceGuid());
			if (sf) primo = reinterpret_cast<obj_primo *>(sf->getInterface());
			if (primo)
			{
				DWORD unit = (DWORD)fn[6];
				DWORD type;
				BYTE szUnitDescr[64 + 1] = {0,}; // Unit Vendor, Model and FW. version

				retCode  = primo->UnitInfo(&unit, &type, szUnitDescr, NULL);
				if (retCode == PRIMOSDK_OK)
				{
					StringCchPrintfW(dest, destlen, L"%d;%s", type, (wchar_t *)AutoWide((char *)szUnitDescr));
				}
				sf->releaseInterface(primo);
			}
			return (retCode == PRIMOSDK_OK);
			#else
			return 0;
			#endif
		}
		else if (!_stricmp(data, "cdlock"))
		{
			dest[0] = 0;
			#ifndef IGNORE_API_GRACENOTE
			if (!m_veritas_handle)
			{
				waServiceFactory *sf = line.service->service_getServiceByGuid(obj_primo::getServiceGuid());
				if (sf) m_veritas_handle = reinterpret_cast<obj_primo *>(sf->getInterface());
			}

			if (!m_veritas_handle)
				return 0;

			m_nblock++;
			DWORD unit = (DWORD)fn[6];
			m_veritas_handle->UnitLock(&unit, PRIMOSDK_LOCK);
			lstrcpynW(dest, L"locked", destlen);
			return 1;
			#else
			return 0;
			#endif
		}
		else if (!_stricmp(data, "cdunlock"))
		{
			if (!m_nblock) return 0;

			#ifndef IGNORE_API_GRACENOTE
			if (!m_veritas_handle)
			{
				waServiceFactory *sf = line.service->service_getServiceByGuid(obj_primo::getServiceGuid());
				if (sf) m_veritas_handle = reinterpret_cast<obj_primo *>(sf->getInterface());
			}

			if (!m_veritas_handle)
				return 0;

			DWORD unit = (DWORD)fn[6];
			m_veritas_handle->UnitLock(&unit, PRIMOSDK_UNLOCK);
			m_nblock--;
			lstrcpynW(dest, L"unlocked", destlen);
			return 1;
			#else
			return 0;
			#endif
		}
		else if (!_stricmp(data, "track") && track)
		{
			StringCchPrintfW(dest, destlen, L"%d", track);
			return 1;
		}
		else if (!_stricmp(data, "bitrate") && track)
		{
			StringCchPrintfW(dest, destlen, L"%d", 1411/*200*/);
			return 1;
		}
		else
			return 0;

		return 1;
	}
	return 0;
}

static wchar_t m_eiw_lastdrive;
//#ifndef IGNORE_API_GRACENOTE
static DINFO setInfo;
//#endif

void ParseIntSlashInt(const wchar_t *string, int *part, int *parts);

extern "C" __declspec(dllexport) int winampSetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *val)
{
//#ifndef IGNORE_API_GRACENOTE
	s_last_error = NULL;
	if (!lstrcmpiW(PathFindExtensionW(fn), L".cda") && !_wcsnicmp(fn + 1, L":\\track", 7)) // stupid hack, converts x:\\trackXX.cda to cda://x,XX
	{
		static wchar_t fakebuf[128];
		StringCchPrintf(fakebuf, 128, L"cda://%c,%d", fn[0], _wtoi(PathFindFileNameW(fn) + 5));
		fn = fakebuf;
	}

	wchar_t drive = 0;
	int tracknum = -1;
	if (!ParseName(fn, drive, tracknum))
		return 0;

	if (drive < 'A' || drive > 'Z') return 0;

	if (drive != m_eiw_lastdrive)
	{
		setInfo.Reset();
		setInfo.populated=false;
		m_eiw_lastdrive = 0;
		MCIDEVICEID dev2 = 0;
		if (!CDOpen(&dev2, drive, L"setinfo")) dev2 = 0;

		if (dev2)
		{
			int ret = GetDiscID(dev2, &setInfo);
			CDClose(&dev2);
			if (!ret)
			{
				GetCDDBInfo(&setInfo, drive);
				m_eiw_lastdrive = drive;
			}
		}
	}

	if (!m_eiw_lastdrive)
		return 0;

	#define DO_TRACK(comparetag, field) if (tracknum > 0 && tracknum < 100 && !_stricmp(data, comparetag)) {\
		ndestring_release(setInfo.tracks[tracknum-1]. ## field);\
		setInfo.tracks[tracknum-1]. ## field=0;\
		if (val && *val) setInfo.tracks[tracknum-1]. ## field=ndestring_wcsdup(val);\
	}

	#define DO_DISC(comparetag, field) if (!_stricmp(data, comparetag)) {\
		ndestring_release(setInfo. ## field);\
		setInfo. ## field=0;\
		if (val && *val) setInfo. ## field=ndestring_wcsdup(val);\
	}

	if (tracknum > 0 && tracknum < 100 && !_stricmp(data, "artist"))
	{
		if (val && setInfo.tracks[tracknum-1].artist == 0 && setInfo.artist && !_wcsicmp(setInfo.artist, val))
			val=0;

		ndestring_release(setInfo.tracks[tracknum-1].artist);
		setInfo.tracks[tracknum-1].artist=0;
		if (val && *val) setInfo.tracks[tracknum-1].artist=ndestring_wcsdup(val);
	}
	else DO_TRACK("title", title)
	else DO_TRACK("composer", composer)
	else DO_TRACK("conductor", conductor)
	else DO_TRACK("GracenoteFileID", tagID)
	else DO_TRACK("GracenoteExtData", extData)
	else DO_DISC("album", title)
	else DO_DISC("albumartist", artist)
	else DO_DISC("genre", genre)
	else DO_DISC("year", year)
	else DO_DISC("comment", notes)
	else DO_DISC("publisher", label)
	else DO_DISC("tuid", tuid)
	else DO_DISC("composer", composer)
	else DO_DISC("conductor", conductor)
	else if (!_stricmp(data, "disc"))
		ParseIntSlashInt(val , &setInfo.discnum, &setInfo.numdiscs);
	else
//#endif
		return 0;
	return 1;
}

extern "C" __declspec(dllexport) int winampWriteExtendedFileInfo()
{
	s_last_error = NULL;
	// write it out
	if (m_eiw_lastdrive)
	{
		//#ifndef IGNORE_API_GRACENOTE
		CddbCache_SetDisc(&setInfo, S_OK);
		StoreDINFO(setInfo.CDDBID, &setInfo);
		m_eiw_lastdrive = 0;
		last_fn[0]=0;
		if (cachedev_used)
		{
			CDClose(&cachedev);
			cachedev_used=0;
		}
		return 1;
		//#endif
	}
	return 0;
}

// return 1 if you want winamp to show it's own file info dialogue, 0 if you want to show your own (via In_Module.InfoBox)
// if returning 1, remember to implement winampGetExtendedFileInfo("formatinformation")!
extern "C" __declspec(dllexport) int winampUseUnifiedFileInfoDlg(const wchar_t * fn)
{
	return 1;
}

#define SET_IF(hwndDlg, id, data) if (data) SetDlgItemText(hwndDlg, id, data); else SetDlgItemText(hwndDlg, id, L"");
static void Fill(HWND hwndDlg, const DINFO *info)
{
	SET_IF(hwndDlg, IDC_TITLE, info->title);
	SET_IF(hwndDlg, IDC_ARTIST, info->artist);

	if (info->discnum)
		SetDlgItemInt(hwndDlg, IDC_DISC, info->discnum, FALSE);
	else
		SetDlgItemText(hwndDlg, IDC_DISCS, L"");

	if (info->numdiscs)
		SetDlgItemInt(hwndDlg, IDC_DISCS, info->numdiscs, FALSE);
	else
		SetDlgItemText(hwndDlg, IDC_DISCS, L"");

	SET_IF(hwndDlg, IDC_YEAR, info->year);
	SET_IF(hwndDlg, IDC_LABEL, info->label);
	SET_IF(hwndDlg, IDC_NOTES, info->notes);
	SET_IF(hwndDlg, IDC_GENRE, info->genre);

	SendDlgItemMessage(hwndDlg, IDC_TRACKLIST, LB_RESETCONTENT, 0, 0);

	#ifndef IGNORE_API_GRACENOTE
	for (int x = 0; x < info->ntracks; x ++)
	{
		wchar_t buf[1100] = {0};
		if (!info->tracks[x].title)
			StringCchPrintfW(buf, 1100, L"%d.", x+1);
		else if (info->tracks[x].artist && info->tracks[x].artist[0] && wcscmp(info->tracks[x].artist, info->artist))
			StringCchPrintfW(buf, 1100, L"%d. %s - %s", x+1, info->tracks[x].artist, info->tracks[x].title);
		else
			StringCchPrintfW(buf, 1100, L"%d. %s", x+1, info->tracks[x].title);
		SendDlgItemMessageW(hwndDlg, IDC_TRACKLIST, LB_ADDSTRING, 0, (LPARAM)buf);
	}
	#endif
}

#ifndef IGNORE_API_GRACENOTE
struct LookupData
{
	LookupData(HWND hwndDlg)
	{
		hwnd=hwndDlg;
		dlgItem=0;
		device=0;
		tracknum=0;
		use=false;
		disc=0;
		memset(szTOC, 0, sizeof(szTOC));
	}
	~LookupData()
	{
		if (disc)
			disc->Release();
	}
	HWND hwnd;
	int dlgItem;
	wchar_t szTOC[2048];
	char device;
	int tracknum;
	bool use;
	ICddbDisc *disc;
	DINFO info;
};

static HRESULT CALLBACK Cddb_LookupCallback(HRESULT result, ICddbDisc *pDisc, DWORD *pdwAutoCloseDelay, ULONG_PTR user)
{
	LookupData *data = (LookupData *)user;

	if (S_OK == result)
	{
		data->info.Reset();
		data->info.populated=false;
		if (data->disc)
			data->disc->Release();
		data->disc=pDisc;
		data->disc->AddRef();
		GetDiscInfo(pDisc, &data->info);
		Fill(data->hwnd, &data->info);

		ICddbCacheManager* pCache;
		HRESULT hr = Cddb_GetICacheManger((void**)&pCache);
		if (SUCCEEDED(hr))
		{
			pCache->StoreDiscByToc(data->szTOC, data->disc);
			pCache->Release();
		}
	}
	else
	{
		*pdwAutoCloseDelay = AUTOCLOSE_NEVER;
	}

	return S_OK;
}

static HRESULT CALLBACK Cddb_EditCallback(HRESULT result, ICddbDisc *pDisc, DWORD *pdwAutoCloseDelay, ULONG_PTR user)
{
	LookupData *data = (LookupData *)user;

	if (FAILED(result))
	{
		*pdwAutoCloseDelay = AUTOCLOSE_NEVER;
		return S_OK;
	}

	if (SUCCEEDED(result))
	{
		HRESULT hr(S_OK);
		ICDDBControl  *pControl;
		CDDBUIFlags uiFlags = UI_EDITMODE;

		if (SUCCEEDED(hr)) hr = Cddb_GetIControl((void**)&pControl);
		if (SUCCEEDED(hr))
		{
			if (!pDisc)
			{
				uiFlags = UI_SUBMITNEW;
				hr = pControl->GetSubmitDisc(data->szTOC, 0, 0, &pDisc);
				if (FAILED(hr)) pDisc = NULL;
			}
			else
				pDisc->AddRef();

			if (pDisc)
			{
				HWND parent = GetParent(data->hwnd);
				Cddb_DisplayDiscInfo(pDisc, &uiFlags, parent);
				if (uiFlags & UI_DATA_CHANGED)
				{
					ICddbCacheManager* pCache;
					hr = Cddb_GetICacheManger((void**)&pCache);
					if (SUCCEEDED(hr))
					{
						pCache->StoreDiscByToc(data->szTOC, pDisc);
						pCache->Release();
					}

					data->info.Reset();
					data->info.populated=false;
					if (data->disc)
						data->disc->Release();
					data->disc=pDisc;
					data->disc->AddRef();
					GetDiscInfo(pDisc, &data->info);
					Fill(data->hwnd, &data->info);
				}
				pDisc->Release();
			}
			pControl->Release();
		}
	}
	return S_OK;
}

bool SubmitEdit(ICddbDisc *pDisc)
{
	//CDDBUIFlags uiFlags = UI_EDITMODE;
	ICDDBControl  *pControl;

	HRESULT hr = Cddb_GetIControl((void**)&pControl);
	if (SUCCEEDED(hr)) 
	{

		long val;
		pControl->SubmitDisc(pDisc, 0, &val);
		pControl->Release();
		if (val == 0)
			return true;
	}
	return false;
}
#endif

#define SEND_DISC(field, val) SendMessage(hwndParent,WM_USER, (WPARAM)field,(LPARAM)(val?val:L""));
#define SEND_DISC_OR_TRACK(field, disc_val, track_val) { if (track_val) SendMessage(hwndParent,WM_USER, (WPARAM)field,(LPARAM)track_val); else if (disc_val) SendMessage(hwndParent,WM_USER, (WPARAM)field,(LPARAM)disc_val); else SendMessage(hwndParent,WM_USER, (WPARAM)field,(LPARAM)L"");}
#define SEND_TRACK(field, val) SendMessage(hwndParent,WM_USER, (WPARAM)field,(LPARAM)(val?val:L""));

#ifndef IGNORE_API_GRACENOTE
static void NotifyParent_MusicID(HWND hwndParent, const LookupData *data)
{
	DINFO disc = data->info;
	TRACKINFO dummy;
	TRACKINFO &track = data->tracknum?disc.tracks[data->tracknum-1]:dummy;

	SEND_DISC(L"album", disc.title);
	SEND_DISC(L"albumartist", disc.artist);
	SEND_DISC_OR_TRACK(L"artist", disc.artist, track.artist);
	SEND_DISC(L"tuid", disc.tuid);
	SEND_DISC(L"year", disc.year);
	SEND_DISC(L"genre", disc.genre);
	SEND_DISC(L"publisher", disc.label);
	SEND_DISC(L"comment", disc.notes);
	SEND_DISC_OR_TRACK(L"conductor", disc.conductor, track.conductor);
	SEND_DISC_OR_TRACK(L"composer", disc.composer, track.composer);

	wchar_t disc_temp[64] = {0};
	if (disc.numdiscs)
		StringCchPrintfW(disc_temp, 64, L"%d/%d", disc.discnum, disc.numdiscs);
	else if (disc.discnum)
		StringCchPrintfW(disc_temp, 64, L"%d", disc.discnum);
	else
		disc_temp[0]=0;
	SEND_DISC(L"disc", disc_temp);

	SEND_TRACK(L"title", track.title);
	SEND_TRACK(L"GracenoteFileID", track.tagID);
	SEND_TRACK(L"GracenoteExtData", track.extData);
}

static INT_PTR CALLBACK MusicID_Proc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			LookupData *data = new LookupData(hwndDlg);
			wchar_t *filename = (wchar_t *)lParam;
			if (ParseName(AutoChar(filename), data->device, data->tracknum)) // TODO: remove AutoChar here, I'm just being lazy
			{
				MCIDEVICEID d = 0;
				if (CDOpen(&d, data->device, L"MusicID_Dlg"))
				{
					GetDiscID(d, &data->info);
					CDClose(&d);

					if (Cddb_CalculateTOC(&data->info, data->szTOC, sizeof(data->szTOC)/sizeof(wchar_t)))
					{
						ICddbCacheManager *pCache;
						if (SUCCEEDED(Cddb_GetICacheManger((void**)&pCache)))
						{
							if (SUCCEEDED(pCache->FetchDiscByToc(data->szTOC, &data->disc)))
							{
								GetDiscInfo(data->disc, &data->info);
								Fill(hwndDlg, &data->info);
							}
							pCache->Release();
						}
					}
				}
			}
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)data);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			{
				LookupData *data = (LookupData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				if (data->use && data->disc)
				{
					StoreDisc(data->info.CDDBID, data->disc);
					CddbCache_SetDisc(&data->info, S_OK);
				}
			}
			break;
		case IDC_LOOKUP:
			{
				LookupData *data = (LookupData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				if (data)
				{
					data->dlgItem = LOWORD(wParam);
					HWND parent = GetParent(hwndDlg);
					UINT flags = CDDB_NOCACHE | CDDB_UI_MODAL | CDDB_UI_MULTIPLE | CDDB_UI_RESULT_MODAL;
					HRESULT hr = Cddb_DoLookup(data->szTOC, parent, Cddb_LookupCallback, flags, (ULONG_PTR)data);
					if (FAILED(hr)) Cddb_DisplayResultDlg(parent, hr, AUTOCLOSE_NEVER, flags);
				}
			}
			break;
		case IDC_USE:
			{
				LookupData *data = (LookupData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				data->use=true;
				NotifyParent_MusicID(GetParent(hwndDlg), data);
			}
			break;
		case IDC_EDIT_GRACENOTE:
			{
				LookupData *data = (LookupData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				data->dlgItem = LOWORD(wParam);
				HWND parent = GetParent(hwndDlg);
				UINT flags = CDDB_UI_MODAL | CDDB_UI_MULTIPLE | CDDB_UI_RESULT_MODAL;
				HRESULT hr = Cddb_DoLookup(data->szTOC, parent, Cddb_EditCallback, flags, (ULONG_PTR)data);
				if (FAILED(hr)) Cddb_DisplayResultDlg(parent, hr, AUTOCLOSE_NEVER, flags);
			}
			break;
		case IDC_SUBMIT:
			{
				LookupData *data = (LookupData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				if (data && data->disc)
					SubmitEdit(data->disc);
			}
			break;
		}
		break;
	case WM_DESTROY:
		{
			LookupData *data = (LookupData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			delete data;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)0);
		}
		break;
	}
	return 0;
}
#endif

struct CDTextData
{
public:
	CDTextData()
	{
		device=0;
		tracknum=0;
		use=false;
	}
	DINFO info;
	wchar_t device;
	int tracknum;
	bool use;
};

static void Send_CDText(HWND hwndParent, const CDTextData *data)
{
	DINFO disc = data->info;
	TRACKINFO dummy;
	TRACKINFO &track = data->tracknum ? disc.tracks[data->tracknum-1] : dummy;
	SEND_DISC(L"album", disc.title);
	SEND_DISC(L"albumartist", disc.artist);
	SEND_DISC_OR_TRACK(L"artist", disc.artist, track.artist);
	SEND_DISC_OR_TRACK(L"composer", disc.composer, track.composer);
	SEND_TRACK(L"title", track.title);
	SEND_DISC_OR_TRACK(L"genre", disc.genre, track.genre);
}

static void FillDialog_CDText(HWND hwndDlg, const DINFO &info)
{
	SET_IF(hwndDlg, IDC_ARTIST, info.artist);
	SET_IF(hwndDlg, IDC_ALBUM, info.title);
	SET_IF(hwndDlg, IDC_COMPOSER, info.composer);

	W_ListView listview(GetDlgItem(hwndDlg, IDC_TRACKS));

	listview.Clear();

	for (int i=0;i<info.ntracks;i++)
	{
		const TRACKINFO &track = info.tracks[i];
		wchar_t num[64] = {0};
		StringCchPrintfW(num, 64, L"%d", i+1);
		int index = listview.AppendItem(num, 0);
		if (track.artist)
			listview.SetItemText(index, 1, track.artist);
		if (track.title)
			listview.SetItemText(index, 2, track.title);
		if (track.genre)
			listview.SetItemText(index, 3, track.genre);
		if (track.composer)
			listview.SetItemText(index, 4, track.composer);
	}
}

static INT_PTR CALLBACK CDText_Proc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			W_ListView listview;
			listview.setwnd(GetDlgItem(hwndDlg, IDC_TRACKS));

			listview.AddCol(WASABI_API_LNGSTRINGW(IDS_TRACK), 50);
			listview.AddCol(WASABI_API_LNGSTRINGW(IDS_ARTIST), 150);
			listview.AddCol(WASABI_API_LNGSTRINGW(IDS_TITLE), 150);
			// TODO
			listview.AddCol(L"Genre"/*WASABI_API_LNGSTRINGW(IDS_GENRE)*/, 150);
			listview.AddCol(WASABI_API_LNGSTRINGW(IDS_COMPOSER), 150);

			CDTextData *data = new CDTextData;
			wchar_t *filename = (wchar_t *)lParam;
			if (ParseName(filename, data->device, data->tracknum))
			{
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)data);
			}
			else
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);

			// this is slow if there's no CD Text
			PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_LOOKUP, BN_CLICKED), (LPARAM)GetDlgItem(hwndDlg, IDC_LOOKUP));
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			{
				CDTextData *data = (CDTextData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				if (data && data->use)
					StoreCDText(data->info.CDDBID, data->device);
			}
			break;
		case IDC_LOOKUP:
			{
				CDTextData *data = (CDTextData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				if (data && DoCDText(&data->info, data->device))
				{
					FillDialog_CDText(hwndDlg, data->info);
				}
			}
			break;
		case IDC_USE:
			{
				CDTextData *data = (CDTextData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				if (data)
				{
					data->use=true;
					Send_CDText(GetParent(hwndDlg), data);
				}
			}
			break;
		}
		break;
	case WM_DESTROY:
		{
			CDTextData *data = (CDTextData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			delete data;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)0);
		}
		break;
	}
	return 0;
}

// should return a child window of 513x271 pixels (341x164 in msvc dlg units), or return NULL for no tab.
// Fill in name (a buffer of namelen characters), this is the title of the tab (defaults to "Advanced").
// filename will be valid for the life of your window. n is the tab number. This function will first be
// called with n == 0, then n == 1 and so on until you return NULL (so you can add as many tabs as you like).
// The window you return will recieve WM_COMMAND, IDOK/IDCANCEL messages when the user clicks OK or Cancel.
// when the user edits a field which is duplicated in another pane, do a SendMessage(GetParent(hwnd),WM_USER,(WPARAM)L"fieldname",(LPARAM)L"newvalue");
// this will be broadcast to all panes (including yours) as a WM_USER.
extern "C" __declspec(dllexport) HWND winampAddUnifiedFileInfoPane(int n, const wchar_t * filename, HWND parent, wchar_t *name, size_t namelen)
{
	if (!lstrcmpiW(PathFindExtensionW(filename), L".cda") && !_wcsnicmp(filename + 1, L":\\track", 7)) // stupid hack, converts x:\\trackXX.cda to cda://x,XX
	{
		static wchar_t fakebuf[128];
		StringCchPrintf(fakebuf, 128, L"cda://%c,%d", filename[0], _wtoi(PathFindFileNameW(filename) + 5));
		filename = fakebuf;
	}

	#ifndef IGNORE_API_GRACENOTE
	switch (n)
	{
	case 0: // MusicID
		StringCchCopyW(name, namelen, L"MusicID"); // benski> this is purposefully not translatable
		return WASABI_API_CREATEDIALOGPARAMW(IDD_MUSICID,parent,MusicID_Proc,(LPARAM)_wcsdup(filename));
	case 1: // CD Text
		WASABI_API_LNGSTRINGW_BUF(IDS_CDTEXT,name, namelen);
		return WASABI_API_CREATEDIALOGPARAMW(IDD_CDTEXT,parent,CDText_Proc,(LPARAM)_wcsdup(filename));
	default:
		return 0;
	}
	#else
	switch (n)
	{
		case 0: // CD Text
		{
			//if (DoCDText(0, filename[6]))	// this is slow if there's no CD Text
			{
				WASABI_API_LNGSTRINGW_BUF(IDS_CDTEXT,name, namelen);
				return WASABI_API_CREATEDIALOGPARAMW(IDD_CDTEXT,parent,CDText_Proc,(LPARAM)_wcsdup(filename));
			}
			return 0;
		}
		default:
			return 0;
	}
	#endif
}