/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
** Filename:
** Project:
** Description:
** Author:
** Created:
**/

#ifndef CONFIG_IMPL
#define CONFIG_IMPL
#endif
#include "Main.h"
#include "config.h"
#include "WinampAttributes.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "../nu/ns_wc.h"
#include "../nu/AutoLock.h"
#include "../nu/AutoCharFn.h"
#include "../Elevator/FileTypeRegistrar.h"
#include "main.hpp"
#include <shobjidl.h>

#if (_MSC_VER < 1500)
typedef struct tagBIND_OPTS3 : tagBIND_OPTS2
{
	HWND hwnd;
} BIND_OPTS3, * LPBIND_OPTS3;
#endif
static HRESULT CoCreateInstanceAsAdmin(HWND hwnd, REFCLSID rclsid, REFIID riid, void ** ppv)
{
	BIND_OPTS3 bo;
	WCHAR wszCLSID[50] = {0};
	WCHAR wszMonikerName[300] = {0};

	StringFromGUID2(rclsid, wszCLSID, sizeof(wszCLSID)/sizeof(wszCLSID[0])); 
	HRESULT hr = StringCchPrintfW(wszMonikerName, sizeof(wszMonikerName)/sizeof(wszMonikerName[0]), L"Elevation:Administrator!new:%s", wszCLSID);
	if (FAILED(hr))
		return hr;
	memset(&bo, 0, sizeof(bo));
	bo.cbStruct = sizeof(bo);
	bo.hwnd = hwnd;
	bo.dwClassContext  = CLSCTX_LOCAL_SERVER;
	return CoGetObject(wszMonikerName, &bo, riid, ppv);
}

static bool NeedElevation()
{
	return !IsUserAnAdmin();
}

static Nullsoft::Utility::LockGuard registrarGuard;
static const GUID ftr_guid = 
			{ 0x3B29AB5C, 0x52CB, 0x4a36, { 0x93,0x14,0xE3,0xFE,0xE0,0xBA,0x74,0x68 } };
static IFileTypeRegistrar *masterRegistrar;
static FileTypeRegistrar builtIn;
bool usingBuiltIn = false;
int GetRegistrar(IFileTypeRegistrar **registrar, BOOL use_fallback)
{
	Nullsoft::Utility::AutoLock autolock(registrarGuard);
	// attempt to re-get an elevated object as needed
	// i.e. if user selects no but wants to try again
	//		then we need to try going elevated again
	if (!masterRegistrar || usingBuiltIn)
	{
#if 1 // benski> on Vista, we need this object to run out-of-process so it can be elevated to Administrator
		// without elevating Winamp itself - see http://msdn2.microsoft.com/en-us/ms679687.aspx
		OSVERSIONINFO version = {0};
		version.dwOSVersionInfoSize = sizeof(version);
		if (!GetVersionEx(&version)) ZeroMemory(&version, sizeof(OSVERSIONINFO));
		if (version.dwMajorVersion >= 6 && NeedElevation()) // Vista
		{
			IFileTypeRegistrar *registrar = 0;
			HRESULT hr = CoCreateInstanceAsAdmin(0, ftr_guid, __uuidof(IFileTypeRegistrar), (void**)&registrar);
			if (SUCCEEDED(hr) && registrar)
			{
				if (masterRegistrar) masterRegistrar->Release();
				masterRegistrar = registrar;
				usingBuiltIn = false;
			}
			else
			{
				if (!use_fallback)
				{
					if (registrar) registrar->Release();
					if (masterRegistrar) masterRegistrar->Release();
					registrar = masterRegistrar = 0;
					usingBuiltIn = false;
					return 1;
				}
			}
		}
#if 0
		else /* benski> on earlier OS's just for testing purposes, we'll get rid of this totally once the COM stuff is worked out */
		{
			CoCreateInstance(ftr_guid, NULL, CLSCTX_LOCAL_SERVER, __uuidof(IFileTypeRegistrar), (void**)&masterRegistrar);
		}
#endif
#endif
	}

	if (!masterRegistrar) /* wasn't registered? we'll use our internal version (but it won't work well on Vista+) */
	{
		masterRegistrar = &builtIn;
		usingBuiltIn = true;
	}

	if (masterRegistrar)
	{
		*registrar = masterRegistrar;
		masterRegistrar->AddRef();
		return 0;
	}
	return 1;
}

void RemoveRegistrar()
{
	Nullsoft::Utility::AutoLock autolock(registrarGuard);
	if (masterRegistrar) masterRegistrar->Release();
	masterRegistrar = 0;
	usingBuiltIn = false;
}

int _r_i(char *name, int def)
{
	return GetPrivateProfileIntA(app_name, name, def, INI_FILEA);
}

#define RI(x) (( config_##x ) = _r_i(#x,( config_##x )))
#define RI_NP(x) (( x ) = _r_i(#x,( x )))
#define RB(x) (( config_##x ) = !!_r_i(#x,( config_##x )))
void _w_i(char *name, intptr_t d)
{
	char str[120] = {0};
	StringCchPrintfA(str, 120, "%d", d);
	WritePrivateProfileStringA(app_name, name, str, INI_FILEA);
}
#define WI(x) _w_i(#x,( config_##x ))
#define WI_NP(x) _w_i(#x,( x ))

void _r_s(char *name, char *data, int mlen)
{
	char utf8_data[2048] = {0};
	wchar_t utf16_data[2048] = {0};
	char buf[2048] = {0};
	StringCchCopyA(buf, 2048, data);
	GetPrivateProfileStringA(app_name, name, buf, utf8_data, 2048, INI_FILEA);
	MultiByteToWideCharSZ(CP_UTF8, 0, utf8_data, -1, utf16_data, 2048);
	WideCharToMultiByteSZ(CP_ACP, 0, utf16_data, -1, data, mlen, 0, 0);
}
#define RS(x) (_r_s(#x,config_##x,sizeof(config_##x)))
#define RS_NP(x) (_r_s(#x,x,sizeof(x)))

void _r_sW(const char *name, wchar_t *data, int mlen)
{
	char utf8_data[2048] = {0};
	char default_data[2048] = {0};
	WideCharToMultiByteSZ(config_utf8?CP_UTF8:CP_ACP, 0, data, -1, default_data, 2048,0,0);
	GetPrivateProfileStringA(app_name, name, default_data, utf8_data, 2048, INI_FILEA);
	MultiByteToWideCharSZ(config_utf8?CP_UTF8:CP_ACP, 0, utf8_data, -1, data, mlen);
}
#define RSW(x) (_r_sW(#x,config_##x,sizeof(config_##x)/sizeof(wchar_t)))
#define RSW_NP(x) (_r_sW(#x,x,sizeof(x)/sizeof(wchar_t)))

void _w_s(char *name, char *data)
{
	WritePrivateProfileStringA(app_name, name, AutoChar(AutoWide(data), CP_UTF8), INI_FILEA);
}
#define WS(x) (_w_s(#x,config_##x))
#define WS_NP(x) (_w_s(#x,x))

void _w_sW(const char *name, const wchar_t *data)
{
	WritePrivateProfileStringA(app_name, name, AutoChar(data, CP_UTF8), INI_FILEA); // TODO: don't want autowide here
}
#define WSW(x) (_w_sW(#x,config_##x))
#define WSW_NP(x) (_w_sW(#x,x))


void config_write(int i)
{
	config_pilp = PlayList_getPosition();
	//GetCurrentDirectoryW(MAX_PATH, config_cwd);
	if (i && plneedsave)
	{
		// changed 5.64 to only save if there was a change as this can save a
		// decent amount of time on closing with a large unmodified playlist.
		savem3ufn(M3U_FILE, 0, 1);
		savem3ufn(OLD_M3U_FILE, 0, 1);
	}

	config_utf8=1;
	WI(utf8);

	if (i != 1) // write mostly unused stuff, like proxy, plugin names, etc
	{
		WS(defext);
		WSW(titlefmt);
		WI(proxy80);
		WS(proxy);
		WSW(visplugin_name);
		WSW(dspplugin_name);
		WI(check_ft_startup);
		WI(updated_ft_startup);
		WI(visplugin_num);
		WI(pe_fontsize);
		WI(pe_direction);
		WI(visplugin_priority);
		WI(visplugin_autoexec);
		WI(dspplugin_num);
		WI(sticon);
		WI(splash);
		WI(taskbar);
		WI(dropaotfs);
		WI(nomwheel);
		WI(ascb_new);
		WI(ttips);
		WI(riol);
		WI(minst);
		WI(whichicon);
		WI(whichicon2);
		WI(addtolist);
		WI(snap);
		WI(snaplen);
		WI(parent);
		WI(hilite);
		WI(disvis);
		WI(rofiob);
		WI(shownumsinpl);
		WI(keeponscreen);
		WI(eqdsize);
		WI(usecursors);
		WI(fixtitles);
		WI(priority);
		WI(shuffle_morph_rate);
		WI(useexttitles);
		WI(bifont);
		WI(bifont_alt);
		WI(dotitlenum);
		WI(dotasknum);
		WI(plscrollsize);
		WI(plmw2xscroll);
		WI(inet_mode);
		WI(ospb);
		WI(embedwnd_freesize);
		WI(no_visseh);
	}
	WI(newverchk);
	WI(newverchk2);
	WI(newverchk3);
	WI(newverchk_rc);
	WI(user_consent_join_channels);
	// write everything else
	{
		int config_prefs_last_page = prefs_last_page | (about_lastpage << 8);
		WI(prefs_last_page);
		prefs_last_page &= 255;

		_w_i("prefs_wx", prefs_rect.left);
		_w_i("prefs_wy", prefs_rect.top);

		_w_i("alt3_wx", alt3_rect.left);
		_w_i("alt3_wy", alt3_rect.top);

		_w_i("editinfo_wx", editinfo_rect.left);
		_w_i("editinfo_wy", editinfo_rect.top);

		_w_i("ctrle_wx", ctrle_rect.left);
		_w_i("ctrle_wy", ctrle_rect.top);

		_w_i("about_wx", about_rect.left);
		_w_i("about_wy", about_rect.top);

		_w_i("loc_wx", loc_rect.left);
		_w_i("loc_wy", loc_rect.top);

		_w_i("time_wx", time_rect.left);
		_w_i("time_wy", time_rect.top);

		_w_i("load_wx", load_rect.left);
		_w_i("load_wy", load_rect.top);
	}
	WI(autoload_eq);
	WI(playlist_recyclebin);
	WI(use_eq);
	WI(eq_ws);
	WI(wx);
	WI(wy);
	WI(minimized);
	WI(aot);
	WI(shuffle);
	WI(repeat);
	WI(volume);
	WI(pan);
	WI(easymove);
	WI(dsize);
	WI(timeleftmode);
	WI(autoscrollname);
	WI(sa);
	WI(safire);
	WI(saref);
	WI(safalloff);
	WI(sa_peaks);
	WI(sa_peak_falloff);
	WI(eq_wx);
	WI(eq_wy);
	WI(eq_open);
	WI(mw_open);
	WI(pe_wx);
	WI(pe_wy);
	WI(pe_open);
	WI(pe_width);
	WI(pe_height);
	WI(pe_height_ws);

	WI(eq_limiter);
	WI(eq_type);
	WI(eq_frequencies);

	/*
	WI(si_wx);
	WI(si_wy);
	WI(si_width);
	WI(si_height);
	WI(si_autoshow);
	WI(si_autosize);
	WI(si_autohide);
	WI(si_open);
	*/

	WI(video_wx);
	WI(video_wy);
	WI(video_open);
	WI(video_width);
	WI(video_height);
	WI(video_ratio1);
	WI(video_ratio2);
	WI(video_useratio);
	WI(windowshade);
	WI(preamp);
	WI(pilp);
	WI(randskin);
	WSW(cwd);
	WSW(skin);
	WS(outname);
	WI(pladv);
	{
		char config_eq_data[256] = {0};
		StringCchPrintfA(config_eq_data, 256, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
			eq_tab[0], eq_tab[1], eq_tab[2], eq_tab[3], eq_tab[4],
			eq_tab[5], eq_tab[6], eq_tab[7], eq_tab[8], eq_tab[9]);
		WS(eq_data);
	}
	WI(video_vsync2);
	WI(video_aspectadj);
	WI(video_overlays);
	WI(video_gdiplus);
	WI(video_ddraw);
	WI(video_updsize);
	WI(video_autoopen);
	WI(video_autoclose);
	WI(video_noss);
	WI(video_logo);
	WI(video_osd);
	WI(video_yv12);
	WI(video_stopclose);
	WI(video_auto_fs);

	WI(playback_thread_priority);
	WI(audio_bits);
	WI(audio_mono);
	WI(audio_surround);
	WI(audio_dither);
	WI(replaygain);
	WI(replaygain_mode);
	WI(replaygain_source);
	WI(replaygain_preferred_only);
	_w_i("replaygain_non_rg_gain", (intptr_t)(config_replaygain_non_rg_gain.GetFloat()*1000.0f));
	_w_i("replaygain_preamp", (intptr_t)(config_replaygain_preamp.GetFloat()*1000.0f));

	//	WI(video_contrast);
	//	WI(video_brightness);
	WI(video_fliprgb);
	WI(video_remove_fs_on_stop);
	WI(wav_do_header);
	WI(wav_convert);
	WS(wav_ext);
	_w_sW("playlist_custom_font", playlist_custom_fontW);
	WI(custom_plfont);
	WI(no_registry);
	WI(last_classic_skin_page);
	WI(last_playback_page);
	WI(last_fileinfo_page);
	WSW(artwork_filter);
	WI(zeropadplnum);
	WI(wlz_menu);
	WI(accessibility_modalbeep);
	WI(accessibility_modalflash);

	WI(noml_ratings_prompt);
	WI(noml_ratings);
	WI(uid_ft);
}

void config_read(int i)
{
	wchar_t programname[MAX_PATH] = {0};
	GetModuleFileNameW(hMainInstance, programname, MAX_PATH);
	//	regmimetype(L"interface/x-winamp-skin", programname, L".wsz", 0); // hack cause wa3 is gheeeeeey

	RI(utf8);
	RI(eq_ws);
	RI(updated_ft_startup);
	// fiddle things so if running an existing install then we force restore file assoc off
	// when we're running on Vista / Win7+ to resolve complaints from users on those OSes.
	// vista
	if(config_updated_ft_startup)
	{
		RI(check_ft_startup);
	}
	else
	{
		OSVERSIONINFO version = {0};
		version.dwOSVersionInfoSize = sizeof(version);
		if (!GetVersionEx(&version)) ZeroMemory(&version, sizeof(OSVERSIONINFO));
		if (version.dwMajorVersion >= 6) // Vista
		{
			config_updated_ft_startup = 0;
		}
		else
		{
			RI(check_ft_startup);
		}
	}
	config_updated_ft_startup = 1;
	
	RS(browserbrand);
	RI(inet_mode);
	RI(pe_fontsize);
	RI(pe_direction);
	RI(ospb);
	RI(visplugin_num);
	RI(visplugin_priority);
	RI(visplugin_autoexec);
	RI(dspplugin_num);
	RI(sticon);
	RI(splash);
	RI(taskbar);
	RI(dropaotfs);
	RI(nomwheel);
	RI(ascb_new);
	RI(ttips);
	RI(keeponscreen);
	RI(riol);
	RI(whichicon);
	RI(whichicon2);
	RI(addtolist);
	RI(snap);
	RI(snaplen);
	RI(parent);
	RI(hilite);
	RI(disvis);
	RI(minst);
	RI(eqdsize);
	RI(pladv);
	RI(bifont);
	RI(bifont_alt);
	RI(autoload_eq);
	RI(playlist_recyclebin);
	RI(use_eq);
	RI(wx);
	RI(wy);
	RI(minimized);
	RI(aot);
	RI(shuffle);
	RI(repeat);
	RI(volume);
	RI(pan);
	RI(easymove);
	RI(dsize);
	RI(timeleftmode);
	RI(autoscrollname);
	RI(sa);
	RI(safire);
	RI(saref);
	RI(safalloff);
	RI(sa_peaks);
	RI(sa_peak_falloff);
	RI(eq_wx);
	RI(eq_wy);
	RI(eq_open);
	RI(mw_open);
	RI(pe_wx);
	RI(pe_wy);
	RI(pe_open);
	RI(pe_width);
	RI(pe_height);
	RI(pe_height_ws);

	RB(eq_limiter);
	RI(eq_type);
	RI(eq_frequencies);
	/*
	RI(si_wx);
	RI(si_wy);
	RI(si_height);
	RI(si_width);
	RI(si_autoshow);
	RI(si_autosize);
	RI(si_autohide);
	RI(si_open);
	*/

	RI(video_wx);
	RI(video_wy);
	RI(video_open);
	RI(video_width);
	RI(video_height);
	RI(video_ratio1);
	RI(video_ratio2);
	RI(video_useratio);
	RI(windowshade);
	RI(preamp);
	RI(pilp);
	RI(randskin);
	if (!*config_cwd) // don't read if the user has overridden it through paths.ini
		RSW(cwd);
	RSW(skin);

	RI_NP(prefs_last_page);
	about_lastpage = prefs_last_page >> 8;
	prefs_last_page &= 255;

	prefs_rect.left = _r_i("prefs_wx", -1);
	prefs_rect.top = _r_i("prefs_wy", -1);

	alt3_rect.left = _r_i("alt3_wx", -1);
	alt3_rect.top = _r_i("alt3_wy", -1);

	editinfo_rect.left = _r_i("editinfo_wx", -1);
	editinfo_rect.top = _r_i("editinfo_wy", -1);

	ctrle_rect.left = _r_i("ctrle_wx", -1);
	ctrle_rect.top = _r_i("ctrle_wy", -1);

	about_rect.left = _r_i("about_wx", -1);
	about_rect.top = _r_i("about_wy", -1);

	loc_rect.left = _r_i("loc_wx", -1);
	loc_rect.top = _r_i("loc_wy", -1);

	time_rect.left = _r_i("time_wx", -1);
	time_rect.top = _r_i("time_wy", -1);

	load_rect.left = _r_i("load_wx", -1);
	load_rect.top = _r_i("load_wy", -1);

	RI(rofiob);
	RI(shownumsinpl);
	RS(outname);
	RI(usecursors);
	RI(fixtitles);
	RI(priority);

	if (!_r_i("fixpriority", 0))
	{
		if (config_priority > 1)
		{
			config_priority++;
			WI(priority);
		}
		_w_i("fixpriority", 1);
	}

	RI(shuffle_morph_rate);
	RI(useexttitles);
	RI(newverchk);
	RI(newverchk2);
	RI(newverchk3);
	RI(newverchk_rc);
	RI(user_consent_join_channels);
	RI(embedwnd_freesize);
	RB(video_vsync2);
	RI(video_aspectadj);
	RB(video_overlays);
	RB(video_gdiplus);
	RB(video_ddraw);
	RI(video_updsize);
	RB(video_autoopen);
	RB(video_autoclose);
	RI(video_noss);
	RI(video_logo);
	RI(video_osd);
	RB(video_yv12);
	RI(video_stopclose);
	RB(video_auto_fs);//RI(video_auto_fs); plague> changed to RB
	RI(dotitlenum);
	RI(dotasknum);
	RI(plscrollsize);
	RI(plmw2xscroll);
	RI(playback_thread_priority);
	RI(audio_bits);
	RB(audio_mono);
	RB(audio_surround);
	RB(audio_dither);
	RB(replaygain);
	RI(replaygain_mode);
	RI(replaygain_source);
	RB(replaygain_preferred_only);
	int gain10 = _r_i("replaygain_non_rg_gain", (int)(config_replaygain_non_rg_gain.GetFloat()*1000.0f));	
	config_replaygain_non_rg_gain = (float)((float)gain10/1000.0f);
	gain10 = _r_i("replaygain_preamp", (int)(config_replaygain_preamp.GetFloat()*1000.0f));
	config_replaygain_preamp = (float)((float)gain10/1000.0f);

	RI(last_classic_skin_page);
	RI(last_playback_page);
	RI(last_fileinfo_page);
	RI(upd_mode);
	RSW(artwork_filter);

	RI(noml_ratings_prompt);
	RI(noml_ratings);

	RI(jtf_check);
	RI(block_img);

	//  RI(video_contrast);
	//  RI(video_brightness);
	RI(video_fliprgb);
	RI(video_remove_fs_on_stop);
	RI(wav_do_header);
	RI(wav_convert);
	RS(wav_ext);
	RI(no_visseh);
	_r_sW("playlist_custom_font", playlist_custom_fontW, sizeof(playlist_custom_fontW)/sizeof(*playlist_custom_fontW));
	if (!*playlist_custom_fontW) StringCbCopyW(playlist_custom_fontW, sizeof(playlist_custom_fontW), DEFAULT_FONT);
	WideCharToMultiByteSZ(CP_ACP, 0, playlist_custom_fontW, -1, playlist_custom_font, 128, 0, 0);
	RI(custom_plfont);
	RI(no_registry);
	RB(proxy80);
	RS(proxy);
	RS(defext);
	RSW(titlefmt);
	RSW(visplugin_name);
	RSW(dspplugin_name);
	RI(zeropadplnum);
	RI(wlz_menu);
	RB(accessibility_modalbeep);
	RB(accessibility_modalflash);

	{
		char eq_data[256] = {0, };
		int x;
		char *p = eq_data;
		RS_NP(eq_data);
		for (x = 0; x < 10; x ++)
		{
			int b = 0, s = 0;
			while (p && *p >= '0' && *p <= '9')
			{
				s = 1;b = b * 10 + *p++ -'0';
			}
			if (!s) break;
			p++;
			eq_tab[x] = min(63, max(b, 0));
		}
	}

	//SetCurrentDirectoryW(config_cwd);

	config_utf8=1;
	WI(utf8);


	if (!i)
	{
		if (!PathFileExistsW(M3U_FILE))
		{
			LoadPlaylistByExtension(OLD_M3U_FILE, L".m3u", 1, 1);
			savem3ufn(M3U_FILE, 0, 1);
		}
		else
			LoadPlaylistByExtension(M3U_FILE, L".m3u8", 1, 1);
		PlayList_setposition(config_pilp);
	}
}


int config_isregistered(wchar_t *ext)
{
	wchar_t b[256] = {0};
	DWORD rval = 0, s = sizeof(b);

	if (config_no_registry) return 0;

	if (!ext[0]) return 0;
	StringCchPrintfW(b, 256, L".%s", ext);

	OSVERSIONINFO version = {0};
	version.dwOSVersionInfoSize = sizeof(version);
	if (!GetVersionEx(&version)) ZeroMemory(&version, sizeof(OSVERSIONINFO));

	if (IsWin8()) // Windows 8
	{
		// QueryAppIsDefault(..) was deprecated in Windows 8 so this is an alternative
		// which seems to work though how long it will keep working is currently unknown
		IApplicationAssociationRegistration* pAAR = 0;
		HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration,
									  NULL, CLSCTX_INPROC,
									  __uuidof(IApplicationAssociationRegistration),
									  (void**)&pAAR);
		if (SUCCEEDED(hr) && pAAR)
		{
			LPWSTR app = 0;
			/*hr = */pAAR->QueryCurrentDefault(b,
											   AT_FILEEXTENSION,
											   AL_EFFECTIVE,
											   &app);
			pAAR->Release();

			if (IsPlaylistExtension(ext))
			{
				if(!lstrcmpiW(WINAMP_PLAYLISTW, app)){ rval = 1; }
			}
			else if (!_wcsicmp(ext, L"wsz") || !_wcsicmp(ext, L"wal") || !_wcsicmp(ext, L"wpz"))
			{
				if(!lstrcmpiW(WINAMP_SKINZIPW, app)){ rval = 1; }
			}
			else if (!_wcsicmp(ext, L"wlz"))
			{
				if(!lstrcmpiW(WINAMP_LANGZIPW, app)){ rval = 1; }
			}
			else
			{
				wchar_t str[64] = {0};
				StringCchPrintfW(str, 64, L"%s%hs", WINAMP_FILEW, b);
				if(!lstrcmpiW(str, app)){ rval = 1; }
			}
			if (app) CoTaskMemFree(app);
		}
	}
	else if (version.dwMajorVersion >= 6) // Vista
	{
		IApplicationAssociationRegistration* pAAR = NULL;
		HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration,
									  NULL, CLSCTX_INPROC,
									  __uuidof(IApplicationAssociationRegistration),
									  (void**)&pAAR);
		if (SUCCEEDED(hr) && pAAR)
		{
			BOOL hasExt=FALSE;
			hr = pAAR->QueryAppIsDefault(b,
										 AT_FILEEXTENSION,
										 AL_EFFECTIVE,
										 AutoWide(app_name),
										 &hasExt);
			pAAR->Release();
			if (SUCCEEDED(hr))
			{
				rval=!!hasExt;
			}
		}
	}
	else
	{
		HKEY key = NULL;
		if (RegOpenKeyW(HKEY_CLASSES_ROOT, b, &key) != ERROR_SUCCESS) return 0;

		DWORD vt = 0;
		if (RegQueryValueExW(key, NULL, 0, &vt, (LPBYTE)b, &s) == ERROR_SUCCESS)
		{
			if (vt != REG_SZ || (wcsncmp(b, WINAMP_FILEW, wcslen(WINAMP_FILEW)) &&
								 wcscmp(b, WINAMP_PLAYLISTW) &&
								 wcscmp(b, WINAMP_SKINZIPW) &&
								 wcscmp(b, WINAMP_LANGZIPW))) rval = 0;
			else rval = 1;
		}
		else rval = 0;
		RegCloseKey(key);
	}

	return rval;
}

bool allowed_extension(wchar_t *ext)
{
	const wchar_t* blocked[] = {L"exe", L"dll",
								L"jpg", L"jpeg", L"gif", L"bmp",
								L"png", L"tif", L"tiff"};
	// ensure we block at least 'exe' and 'dll', and images if not disabled
	for (size_t i = 0; i < (config_block_img ? ARRAYSIZE(blocked) : 2); i++)
	{
		if (!_wcsicmp(ext, blocked[i])) return false;
	}
	return true;
}

void config_register(wchar_t *ext, int reg)
{
	const wchar_t *which_str = WINAMP_FILEW;
	if (config_no_registry) return ;
	if (!ext[0]) return ;
	if (!allowed_extension(ext)) return ; //windows=gay

	IFileTypeRegistrar *registrar=0;
	if (GetRegistrar(&registrar, true) == 0 && registrar)
	{
		wchar_t family_str[256] = {0};

		if (IsPlaylistExtension(ext))
			which_str = WINAMP_PLAYLISTW;
		else if (!_wcsicmp(ext, L"wsz") || !_wcsicmp(ext, L"wal") || !_wcsicmp(ext, L"wpz"))
			which_str = WINAMP_SKINZIPW;
		else if (!_wcsicmp(ext, L"wlz"))
			which_str = WINAMP_LANGZIPW;
		else
		{
			StringCchPrintfW(family_str, 256, L"%s.%s", WINAMP_FILEW, ext);

			wchar_t family_description[256] = {0};
			if (in_get_extended_fileinfoW(family_str, L"family", family_description, 255) && family_description[0])
			{
				which_str=family_str;
				if (reg)
					config_setup_filetype(family_str, family_description, 0);
			}
		}

		if (reg && !_wcsicmp(ext, L"asx"))
		{
			wchar_t programname[MAX_PATH] = {0};
			GetModuleFileNameW(hMainInstance, programname, MAX_PATH);
			regmimetype(L"video/x-asx", programname, L".asx", 0);
			regmimetype(L"video/asx", programname, L".asx", 0);
			regmimetype(L"video/x-ms-asf", programname, L".asx", 0);
		}
		if (reg && !_wcsicmp(ext, L"wal"))
		{
			wchar_t programname[MAX_PATH] = {0};
			GetModuleFileNameW(hMainInstance, programname, MAX_PATH);
			regmimetype(L"interface/x-winamp3-skin", programname, L".wal", 0);
		}
		if (reg && !_wcsicmp(ext, L"wsz"))
		{
			wchar_t programname[MAX_PATH] = {0};
			GetModuleFileNameW(hMainInstance, programname, MAX_PATH);
			regmimetype(L"interface/x-winamp-skin", programname, L".wsz", 0);
		}
		if (reg && !_wcsicmp(ext, L"wlz"))
		{
			wchar_t programname[MAX_PATH] = {0};
			GetModuleFileNameW(hMainInstance, programname, MAX_PATH);
			regmimetype(L"interface/x-winamp-lang", programname, L".wlz", 0);
		}
		if (reg && !_wcsicmp(ext, L"pls"))
		{
			wchar_t programname[MAX_PATH] = {0};
			GetModuleFileNameW(hMainInstance, programname, MAX_PATH);
			regmimetype(L"audio/x-scpls", programname, L".pls", 0);
			regmimetype(L"audio/scpls", programname, L".pls", 0);
		}
		if (reg && !_wcsicmp(ext, L"wma"))
		{
			wchar_t programname[MAX_PATH] = {0};
			GetModuleFileNameW(hMainInstance, programname, MAX_PATH);
			regmimetype(L"audio/x-ms-wma", programname, L".wma", 1);
			regmimetype(L"application/x-msdownload", programname, L".wma", 1);
		}
		if (reg && !_wcsicmp(ext, L"m3u"))
		{
			wchar_t programname[MAX_PATH] = {0};
			GetModuleFileNameW(hMainInstance, programname, MAX_PATH);
			regmimetype(L"audio/x-mpegurl", programname, L".m3u", 0);
			regmimetype(L"audio/mpegurl", programname, L".m3u", 0);
		}
		if (reg && !_wcsicmp(ext, L"mp3"))
		{
			wchar_t programname[MAX_PATH] = {0};
			GetModuleFileNameW(hMainInstance, programname, MAX_PATH);
			regmimetype(L"audio/x-mpeg", programname, L".mp3", 1);
			regmimetype(L"audio/x-mp3", programname, L".mp3", 1);
			regmimetype(L"audio/x-mpg", programname, L".mp3", 1);
			regmimetype(L"audio/mp3", programname, L".mp3", 1);
			regmimetype(L"audio/mpg", programname, L".mp3", 1);
			regmimetype(L"audio/mpeg", programname, L".mp3", 1);
		}

		wchar_t b[128] = {0};
		StringCchPrintfW(b, 128, L".%s", ext);
		CharLowerBuffW(b, ARRAYSIZE(b));
		if (reg)
			registrar->RegisterType(b, which_str, AutoWide(app_name));
		else
		{
			// avoid removing WINAMP_FILEW as this will break some parts
			// this will generally happen from 3rd party plugins where
			// no %family% will be correctly returned in the relevant
			if(_wcsicmp(which_str, WINAMP_FILEW))
			{
				registrar->UnregisterType(b, which_str, AutoWide(app_name), IsPlaylistExtension(ext));
			}
		}

		registrar->Release();
	}
}

void regmimetype(const wchar_t *mtype, const wchar_t *programname, const wchar_t *ext, int nsonly)
{
	if (config_no_registry)
		return ;

	IFileTypeRegistrar *registrar=0;
	if (GetRegistrar(&registrar, true) == 0 && registrar)
	{
		registrar->RegisterMIMEType(mtype, programname, ext, nsonly);
		registrar->Release();
	}
}

void trimPathW(wchar_t *pathStart)
{
	PathRemoveBlanksW(pathStart);
	PathRemoveBackslashW(pathStart);
}

void config_setinifile(wchar_t *inifile)
{
	lstrcpynW(INI_FILE, inifile, sizeof(INI_FILE) / sizeof(INI_FILE[0]));
}

void config_setinidir(const wchar_t *inidir)
{
	if (SUCCEEDED(StringCchCopyW(CONFIGDIR, MAX_PATH, inidir)))
		trimPathW(CONFIGDIR);
	else
		CONFIGDIR[0] = 0;
}

void config_setm3udir(const wchar_t *m3udir)
{
	if (SUCCEEDED(StringCchCopyW(M3UDIR, MAX_PATH, m3udir)))
		trimPathW(M3UDIR);
	else
		M3UDIR[0] = 0;
}

void config_setm3ubase(const wchar_t *m3ubase)
{
	if (SUCCEEDED(StringCchCopyW(M3UBASE, MAX_PATH, m3ubase)))
		trimPathW(M3UBASE);
	else
		M3UBASE[0] = 0;
}

void init_config()
{
	GetModuleFileNameW(hMainInstance, PROGDIR, MAX_PATH);
	PathRemoveFileSpecW(PROGDIR);
	SetEnvironmentVariableW(L"WINAMP_PROGRAM_DIR", PROGDIR);
	wchar_t winamp_root[MAX_PATH] = {0};
	StringCchCopyW(winamp_root, MAX_PATH, PROGDIR);
	PathStripToRootW(winamp_root);
	SetEnvironmentVariableW(L"WINAMP_ROOT_DIR", winamp_root);
}

void setup_config(void)
{
	if (!CONFIGDIR[0])
		StringCchCopyW(CONFIGDIR, MAX_PATH, PROGDIR);

	if (!M3UDIR[0])
		StringCchCopyW(M3UDIR, MAX_PATH, CONFIGDIR);

	if (!M3UBASE[0])
		StringCchCopyW(M3UBASE, MAX_PATH, M3UDIR);

	CreateDirectoryW(M3UDIR, NULL);
	CreateDirectoryW(CONFIGDIR, NULL);

	// basic config files
	PathCombineW(OLD_M3U_FILE, M3UDIR, L"Winamp.m3u");
	PathCombineW(M3U_FILE, M3UDIR, L"Winamp.m3u8");
	PathCombineW(BOOKMARKFILE, CONFIGDIR, L"Winamp.bm");
	PathCombineW(BOOKMARKFILE8, CONFIGDIR, L"Winamp.bm8");
	// just make sure if a winamp.bm8 doesn't exist then
	// go make one from winamp.bm - implemented for 5.58+
	if(!PathFileExistsW(BOOKMARKFILE8))
	{
		CopyFileW(BOOKMARKFILE,BOOKMARKFILE8,FALSE);
	}
	PathCombineW(EQDIR1, CONFIGDIR, L"Winamp.q1");
	PathCombineW(EQDIR2, CONFIGDIR, L"Winamp.q2");
	wchar_t tempPath[MAX_PATH] = {0};
	GetTempPathW(MAX_PATH, tempPath);
	PathCombineW(TEMP_FILE, tempPath, L"Winamp.tmp");
	PathCombineW(DEMOMP3, M3UDIR, L"demo.mp3");
	PathCombineW(JSAPI2_INIFILE, CONFIGDIR, L"jsapisec.ini");
	PathCombineW(ML_INI_FILE, CONFIGDIR, L"Plugins\\gen_ml.ini");

	// override INI_FILE if specified
	if (INI_FILE[0])
	{
		if (PathIsFileSpecW(INI_FILE) || PathIsRelativeW(INI_FILE))
		{
			wchar_t temp[MAX_PATH] = {0};
			PathCombineW(temp, CONFIGDIR, INI_FILE);
			lstrcpynW(INI_FILE, temp, MAX_PATH);
		}
	}
	else
		PathCombineW(INI_FILE, CONFIGDIR, L"Winamp.ini");

	// maintain a ansi version of INI_FILE for some aspects
	StringCchCopyA(INI_FILEA, MAX_PATH, AutoCharFn(INI_FILE));

	RI(utf8);

	// skin and plugin directories
	PathCombineW(SKINDIR, PROGDIR, L"Skins");
	PathCombineW(PLUGINDIR, PROGDIR, L"Plugins");
	PathCombineW(SYSPLUGINDIR, PROGDIR, L"System");
	PathCombineW(SKINDIR, PROGDIR, L"Skins");
	PathCombineW(LANGDIR, PROGDIR, L"Lang");
	PathCombineW(VISDIR, PROGDIR, L"Plugins");
	PathCombineW(DSPDIR, PROGDIR, L"Plugins");

	// override skin/plugin directories from config
	{
		wchar_t bW[MAX_PATH] = {0}, *pW;

		bW[0]=0;
		_r_sW("PluginDir", bW, MAX_PATH);
		pW = bW;
		while (pW && (*pW == L' ' || *pW == L'\t')) pW++;
		if (pW && *pW)
		{
			StringCchCopyW(PLUGINDIR, MAX_PATH, bW);
		}

		ZeroMemory(bW, sizeof(bW));
		_r_sW("DSPDir", bW, MAX_PATH);
		pW = bW;
		while (pW && (*pW == L' ' || *pW == L'\t')) pW++;
		if (pW && *pW)
		{
			StringCchCopyW(DSPDIR, MAX_PATH, bW);
		}

		ZeroMemory(bW, sizeof(bW));
		_r_sW("VISDir", bW, MAX_PATH);
		pW = bW;
		while (pW && (*pW == L' ' || *pW == L'\t')) pW++;
		if (pW && *pW)
		{
			StringCchCopyW(VISDIR, MAX_PATH, bW);
		}

		ZeroMemory(bW, sizeof(bW));
		_r_sW("SkinDir", bW, MAX_PATH);
		pW = bW;
		while (pW && (*pW == L' ' || *pW == L'\t')) pW++;
		if (pW && *pW)
		{
			StringCchCopyW(SKINDIR, MAX_PATH, bW);
		}

		ZeroMemory(bW, sizeof(bW));
		_r_sW("LangDir", bW, MAX_PATH);
		pW = bW;
		while (pW && (*pW == L' ' || *pW == L'\t')) pW++;
		if (pW && *pW)
		{
			StringCchCopyW(LANGDIR, MAX_PATH, bW);
		}
	}

	// create a skin temp directory, too
	wchar_t buf[MAX_PATH] = {0};
	GetTempPathW(MAX_PATH, buf);
	GetTempFileNameW(buf, L"WAS", GetTickCount(), SKINTEMPDIR);

	// create a lang temp directory, too by
	// trying to use part of the wlz's name
	// so it's easier to see when debugging
	// e.g. W<lang>XXXX.tmp otherwise it'll
	// revert back to the older WLZXXXX.tmp
	config_load_langpack_var();
	if (wcstok(config_langpack, L"-"))
	{
		wchar_t *p = wcstok(NULL, L"-");
		if (p)
		{
			wchar_t buf2[4] = {0};
			StringCchPrintfW(buf2, 4, L"W%s", p);
			CharUpperBuffW(buf2, 4);
			GetTempFileNameW(buf, buf2, GetTickCount(), LANGTEMPDIR);
		}
		else
		{
			GetTempFileNameW(buf, L"WLZ", GetTickCount(), LANGTEMPDIR);
		}
	}
	else
	{
		GetTempFileNameW(buf, L"WLZ", GetTickCount(), LANGTEMPDIR);
	}

	RI(minst);
}

BOOL config_removedircontext(BOOL use_fallback)
{
	if (config_no_registry)
		return TRUE;

	IFileTypeRegistrar *registrar=0;
	if (GetRegistrar(&registrar, !use_fallback) == 0 && registrar)
	{
		registrar->RemoveDirectoryContext(WINAMP_PLAYW);
		registrar->RemoveDirectoryContext(WINAMP_ENQUEUEW);
		registrar->RemoveDirectoryContext(WINAMP_BOOKMARKW);
		registrar->Release();
		return TRUE;
	}
	return FALSE;
}

int config_iscdplayer(void)
{
	DWORD r = 0, s;
	HKEY mp3Key;
	char buf[MAX_PATH], buf2[MAX_PATH] = "\"";;
	if (!GetModuleFileNameA(hMainInstance, buf2 + 1, sizeof(buf2) - 8)) return 0;
	if (RegOpenKeyA(HKEY_CLASSES_ROOT, "AudioCD\\shell\\play\\command", &mp3Key) != ERROR_SUCCESS) return 0;
	StringCchCatA(buf2, MAX_PATH, "\" %1");
	s = sizeof(buf);
	if (RegQueryValueEx(mp3Key, NULL, 0, NULL, (LPBYTE)buf, &s) == ERROR_SUCCESS)
	{
		if (!lstrcmpiA(buf, buf2)) r = 1;
	}
	RegCloseKey(mp3Key);
	return r;
}

BOOL config_regcdplayer(int reg, int mode)
{
	if (config_no_registry) return TRUE;

	IFileTypeRegistrar *registrar=0;
	if (GetRegistrar(&registrar, !mode) == 0 && registrar)
	{
		wchar_t programName[MAX_PATH] = {0};
		if (GetModuleFileNameW(hMainInstance, programName, MAX_PATH))
		{
			if (reg)
				registrar->RegisterCDPlayer(programName);
			else
				registrar->UnregisterCDPlayer(programName);
		}
		registrar->Release();
		return TRUE;
	}
	return FALSE;
}

void config_load_langpack_var(void)
{
	RSW(langpack);
}

void config_save_langpack_var(void)
{
	WSW(langpack);
}

void config_agent_add(void)
{
	if (config_no_registry) return;

	IFileTypeRegistrar *registrar=0;
	if (GetRegistrar(&registrar, true) == 0 && registrar)
	{
		wchar_t exe_name[MAX_PATH + 2] = {0};
		GetModuleFileNameW(hMainInstance, exe_name, sizeof(exe_name)/sizeof(wchar_t));
		PathRemoveFileSpecW(exe_name);
		PathCombineW(exe_name, exe_name, L"winampa.exe");
		PathQuoteSpacesW(exe_name);

		registrar->AddAgent(exe_name);
		registrar->Release();
	}

	Lang_LocaliseAgentOnTheFly(TRUE);
}

void config_agent_remove(void)
{
	if (config_no_registry) return ;

	IFileTypeRegistrar *registrar=0;
	if (GetRegistrar(&registrar, true) == 0 && registrar)
	{
		HWND hwnd = FindWindowW(L"WinampAgentMain", NULL);
		if (hwnd)
		{
			SendMessageW(hwnd, WM_CLOSE, 0, 0);
		}
		registrar->RemoveAgent();
		registrar->Release();
	}

	Lang_LocaliseAgentOnTheFly(FALSE);
}

BOOL config_register_capability(wchar_t *ext, int mode)
{
	if (config_no_registry) return TRUE;

	IFileTypeRegistrar *registrar=0;
	if (GetRegistrar(&registrar, !mode) == 0 && registrar)
	{
		WCHAR szApplication[128] = {0}, szExtension[64] = {0}, szProgId[256] = {0};
		size_t required = MultiByteToWideCharSZ(CP_ACP, 0, app_name, -1, NULL, 0);
		if (required > ARRAYSIZE(szApplication)) 
			return TRUE;

		if (0 == MultiByteToWideCharSZ(CP_ACP, 0, app_name, -1, szApplication, ARRAYSIZE(szApplication)))
			return TRUE;

		LPWSTR cursor = szProgId;
		size_t remaining = ARRAYSIZE(szProgId);

		if (FAILED(StringCchCopyExW(cursor, remaining, WINAMP_FILEW, &cursor, &remaining, 0)))
			return TRUE;

		if (FAILED(StringCchCopyExW(cursor, remaining, L".", &cursor, &remaining, 0)))
			return TRUE;

		if (FAILED(StringCchCopyExW(cursor, remaining, ext/*szExtension*/, &cursor, &remaining, 0)))
			return TRUE;

		//CharLowerW(szExtension);

		registrar->RegisterCapability(szApplication, szProgId, szExtension);
		registrar->Release();
		return TRUE;
	}
	return FALSE;
}