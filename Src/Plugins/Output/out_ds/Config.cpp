#include "res_wa2/resource.h"
#include "Config.h"
#include "../Winamp/out.h"
#include <api.h>

DS2config::DS2config()
{
	sr = 44100;
	bps = 16;
	nch = 2;
	wnd = 0;
	create_primary = 0;
	error[0] = 0;
	mixing = MIXING_DEFAULT;
	volmode = 0;
	logfades = 0;
	//logvol_min=100;
	chan_mask = 0;
	ms = DEFAULT_BUFFER;
	preb = DEFAULT_PREBUFFER;
	memset(&guid, 0, sizeof(guid));
	sil_db = 0;
	delayed_shutdown = 1;
	prim_override = 0;
	_p_bps = _p_nch = _p_sr = 0;
	use_cpu_management = 0;
	refresh = 10;
	coop = 1;
#ifdef DS2_HAVE_PITCH
	have_pitch = 0;
#endif
}

extern HINSTANCE cfg_orig_dll;
void DS2config::SetErrorCodeMsgA(const TCHAR *msg, DWORD code)
{
	if (code)
	{
		TCHAR boo[512] = {0}, buf[512] = {0};
#ifdef UNICODE
		wsprintf(boo, WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_CODE_08X,buf,512),msg,code);
#else
		wsprintf(boo,WASABI_API_LNGSTRING_BUF(/*cfg_orig_dll,*/IDS_ERROR_CODE_08X,buf,512),msg,code);
#endif
		SetError(boo);
	}
	else SetError(msg);
}

void DS2config::SetError(LPCTSTR n_error) {lstrcpyn(error, n_error, 255);}