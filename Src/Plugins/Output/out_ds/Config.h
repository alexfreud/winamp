#ifndef NULLSOFT_OUT_DS_CONFIG_H
#define NULLSOFT_OUT_DS_CONFIG_H

#include <windows.h>
#include "ds_main.h"

class DS2config	//config struct to pass to DS2::create(); if create messes up, this struct also returns error message
{
public:
	enum
	{
		DEFAULT_BUFFER = 1500,
		DEFAULT_PREBUFFER = 500,
	};
private:
	size_t sr, bps, nch;
	HWND wnd;
	bool create_primary;
	DWORD chan_mask;
	UINT ms;
	UINT preb;
	bool use_cpu_management;
	int volmode;
	int logvol_min;
	bool logfades;
	GUID guid;
	bool delayed_shutdown, prim_override;
	UINT _p_bps, _p_nch, _p_sr;
	float sil_db;
	UINT mixing;
	UINT refresh;
	UINT coop;
#ifdef DS2_HAVE_PITCH
	bool have_pitch;
#endif
	TCHAR error[256];
	void SetError(LPCTSTR n_error);
	void SetErrorCodeMsgA(const TCHAR *msg, DWORD code);
public:
	enum {
		MIXING_DEFAULT = 0,
		MIXING_FORCE_HARDWARE = 1,
		MIXING_FORCE_SOFTWARE = 2
	};
	DS2config();
	inline const TCHAR *GetError() { return error[0] ? error : 0;}
	void _inline SetPCM(UINT _sr, UINT _bps, UINT _nch) {sr = _sr;bps = _bps;nch = _nch;}
	void _inline SetWindow(HWND w) {wnd = w;}
	void _inline SetCreatePrimary(bool b) {create_primary = b;}
	void _inline SetChanMask(DWORD c) {chan_mask = c;}
	void _inline SetDeviceGUID(const GUID& g) {guid = g;}
	void _inline ResetDevice() {memset(&guid, 0, sizeof(guid));}
	void _inline SetBuffer(UINT _ms = DEFAULT_BUFFER, UINT _preb = DEFAULT_PREBUFFER) {ms = _ms;preb = _preb;}
	void _inline SetSilence(float s) {sil_db = s;} // s is in negative dB
	void _inline SetDelayedShutdown(bool d) {delayed_shutdown = d;} //disable for AC97 shit, NOTE: this is global, not per-DS2 instance
	void _inline SetImmediateShutdown(bool d) {delayed_shutdown = !d;}
	void _inline SetPrimaryOverride(bool b) {prim_override = b;}
	void _inline SetPrimaryOverrideFormat(UINT _sr, UINT _bps, UINT _nch) {_p_bps = _bps;_p_nch = _nch;_p_sr = _sr;}
	void _inline SetVolMode(int mode, int logmin = 100, bool _logfades = 0) {volmode = mode;logvol_min = logmin;logfades = _logfades;}
	void _inline SetMixing(UINT m) {mixing = m;}
	void _inline SetCpuManagement(bool b) {use_cpu_management = b;}
	void _inline SetRefresh(UINT n) {refresh = n;}
	void _inline SetCoop(UINT n) {coop = n;}
#ifdef DS2_HAVE_PITCH
	void _inline SetHavePitch(bool b) {have_pitch = b;}
#endif
	friend class DS2;
};

#endif