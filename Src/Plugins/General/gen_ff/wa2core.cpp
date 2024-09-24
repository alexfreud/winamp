#include <precomp.h>
#include "wa2core.h"
#include "wa2frontend.h"
#include "../winamp/wa_ipc.h"
#include "wa2pledit.h"
#include "../nu/AutoWide.h"
#include "gen.h"
#include "../nu/ns_wc.h"
#include "../Agave/Language/api_language.h"
#include "../Winamp/in2.h"
#include "resource.h"
#include "../nu/AutoChar.h"
#include <shlwapi.h>



// {72409F84-BAF1-4448-8211-D84A30A1591A}
static const GUID eqConfigGroupGUID = 
{ 0x72409f84, 0xbaf1, 0x4448, { 0x82, 0x11, 0xd8, 0x4a, 0x30, 0xa1, 0x59, 0x1a } };

// -----------------------------------------------------------------------------------------------------------------
// Core implementation for wa2
// -----------------------------------------------------------------------------------------------------------------

#define TIMER_POLL 0x8971

Core *g_Core = NULL;
api_core *g_core = NULL;
int g_coreref = 0;

// this is called by the library when api->core_create is called

api_core *createCustomCoreApi()
{
	g_coreref++;
	if (g_core == NULL) { g_Core = new Core(); g_core = g_Core; }
	return g_core;
}

// this is called when api->core_destroy is called

void destroyCustomCoreApi(api_core *core)
{
	if (--g_coreref == 0)
	{
		delete g_Core;
		g_Core = NULL;
		g_core = NULL;
	}
}

Core::Core()
{
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(Agave::AgaveConfigGUID);
	if (sf)
		config = (Agave::api_config *)sf->getInterface();

	m_lastpeentry = -1;
	m_laststatus = -1;
	m_lastpos = -1;
	m_lastvol = -1;
	m_lastpan = -1;
	m_lasttitle = L"";
	m_lastsamplerate = -1;
	m_lastchan = -1;
	m_lastbitrate = -1;
	for (int i = 0;i < 10;i++)
		m_lasteqband[i] = 0xdead;
	m_lasteq = -1;
	m_lasteqauto = -1;
	m_lasteqpreamp = -1;
	m_lastfreqband = -1;
	// timerclient_setTimer(TIMER_POLL, 250);
	gotCallback(IPC_CB_MISC_TITLE, 1);
	gotCallback(IPC_CB_MISC_VOLUME, 1);
	gotCallback(IPC_CB_MISC_STATUS, 1);
	gotCallback(IPC_CB_MISC_EQ, 1);
	gotCallback(IPC_CB_MISC_INFO, 1);
	gotCallback(IPC_CB_MISC_TITLE_RATING, 1);
}

Core::~Core()
{
		waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(Agave::AgaveConfigGUID);
	if (sf)
		sf->releaseInterface(config);
	//timerclient_killTimer(TIMER_POLL);
}

void Core::addCallback(CoreCallback *cb)
{
	if (cb == NULL) return ;
	if (callbacks.haveItem(cb)) return ;
	callbacks.addItem(cb);
	cb->ccb_notify(CoreCallback::REGISTER);
}

void Core::delCallback(CoreCallback *cb)
{
	if (cb == NULL) return ;
	if (callbacks.haveItem(cb))
	{
		cb->ccb_notify(CoreCallback::DEREGISTER);
		callbacks.removeItem(cb);
	}
}

void Core::sendCoreCallback(int message, int param1, int param2)
{
	ReentryFilter filter(&rf, message);
	int l = callbacks.getNumItems();
	for (int i = 0; i < l;i++)
		callbacks[i]->ccb_notify(message, param1, param2);
}

int Core::getStatus()
{
	return m_laststatus;
}

void Core::gotCallback(int wParam, int forcecb)
{
	if(wParam == IPC_CB_MISC_TITLE_RATING){
		forcecb = 1;
	}
	switch (wParam)
	{
	case IPC_CB_MISC_TITLE:
	case IPC_CB_MISC_TITLE_RATING:
		{
			// check for title change
			wa2.invalidateCache();
			StringW cur_title=wa2.GetCurrentTitle();
			if (cur_title.isempty()) 
				cur_title = L"";

			StringW cur_file = wa2.GetCurrentFile();

			if (!m_lastfile.iscaseequal(cur_file) || forcecb)
			{
				m_lastfile.swap(cur_file);
				sendCoreCallback(CoreCallback::URLCHANGE, (intptr_t)m_lastfile.getValue());
			}

			if (!m_lasttitle.iscaseequal(cur_title) || forcecb)
			{
				m_lasttitle.swap(cur_title);
				//Martin> i dunno why we send a INFOCHANGE Callback here, but if we want to send this
				//			callback try to deliver the correct data 
				//sendCoreCallback((forcecb==2?CoreCallback::INFOCHANGE:CoreCallback::TITLECHANGE),
				//				 (int)m_lasttitle.getValue(),(wParam == IPC_CB_MISC_TITLE_RATING));

				if (forcecb==2) // this has led to INFOCHANGE Callback
				{
					sendCoreCallback(CoreCallback::INFOCHANGE, (intptr_t)getSongInfoText(), (wParam == IPC_CB_MISC_TITLE_RATING));
				}
				sendCoreCallback(CoreCallback::TITLECHANGE, (intptr_t)m_lasttitle.getValue(), (wParam == IPC_CB_MISC_TITLE_RATING));
			}

			int plentry = wa2.getCurPlaylistEntry();
			int curpeentry = plentry + 1;
			if (curpeentry != m_lastpeentry)
			{
				Wa2PlaylistEditor::_onNewCurrentIndex(curpeentry);
				m_lastpeentry = curpeentry;
			}
		}
		break;
	case IPC_CB_MISC_STATUS:
		{
			int cur_status;
			int resume = 0;
			if (wa2.isPaused()) cur_status = STATUS_PAUSE;
			else if (wa2.isPlaying())
			{
				if (m_laststatus == STATUS_PAUSE)
					resume = 1;
				else
				{
					m_lastsamplerate = -1;
					m_lastbitrate = -1;
					m_lastchan = -1;
				}
				cur_status = STATUS_PLAY;
			}
			else cur_status = STATUS_STOP;

			if (m_laststatus != cur_status || forcecb)
			{
				m_laststatus = cur_status;
				switch (cur_status)
				{
				case STATUS_PLAY: sendCoreCallback(resume ? CoreCallback::UNPAUSED : CoreCallback::STARTED); break;
				case STATUS_STOP:
					{
						m_lastsamplerate = 0;
						m_lastchan = 0;
						m_lastbitrate = 0;
						sendCoreCallback(CoreCallback::STOPPED);
						break;
					}
				case STATUS_PAUSE: sendCoreCallback(CoreCallback::PAUSED); break;
				}
			}
		}
		break;
	case IPC_CB_MISC_VOLUME:
		{
			// check for volume change
			int cur_vol = wa2.getVolume();
			if (m_lastvol != cur_vol || forcecb)
			{
				m_lastvol = cur_vol;
				sendCoreCallback(CoreCallback::VOLCHANGE, cur_vol);
			}
			int cur_pan = wa2.getPanning();
			if (m_lastpan != cur_pan || forcecb)
			{
				m_lastpan = cur_pan;
				sendCoreCallback(CoreCallback::PANCHANGE, cur_pan);
			}
		}
		break;
	case IPC_CB_MISC_EQ:
		{
			// check if the band frequencies changed (winamp vs ISO)
			int new_freqband = config->GetInt(eqConfigGroupGUID, L"frequencies", 0);
			if (m_lastfreqband != new_freqband)
			{
				bool sendCallback = m_lastfreqband != -1;
				
				m_lastfreqband = new_freqband; // set before the callback so we don't run into any issues if the callback triggers another EQ change message
				if (sendCallback)
					sendCoreCallback(CoreCallback::EQFREQCHANGE, new_freqband, 0);
			}

			// check for eq change
			for (int i = 0;i < 10;i++)
			{
				int cur_band = wa2.getEqData(WA2_EQDATA_FIRSTBAND + i);
				if (cur_band == 63 - ((m_lasteqband[i] + 127) / 4)) cur_band = m_lasteqband[i];
				else cur_band = (63 - cur_band) * 4 - 127;
				if (m_lasteqband[i] != cur_band || forcecb)
				{
					m_lasteqband[i] = cur_band;
					sendCoreCallback(CoreCallback::EQBANDCHANGE, i, cur_band);
				}
			}
			int cur_eq = wa2.getEqData(WA2_EQDATA_ENABLED);
			if (m_lasteq != cur_eq)
			{
				m_lasteq = cur_eq;
				sendCoreCallback(CoreCallback::EQSTATUSCHANGE, cur_eq);
			}
			int cur_eqauto = wa2.getEqData(WA2_EQDATA_AUTO);
			if (m_lasteqauto != cur_eqauto || forcecb)
			{
				m_lasteqauto = cur_eqauto;
				sendCoreCallback(CoreCallback::EQAUTOCHANGE, cur_eqauto);
			}
			int cur_eqpreamp = wa2.getEqData(WA2_EQDATA_PREAMP);
			if (cur_eqpreamp == 63 - ((cur_eqpreamp + 127) / 4)) { /*cur_eqpreamp = cur_eqpreamp;*/ }
			else cur_eqpreamp = (63 - cur_eqpreamp) * 4 - 127;
			if (m_lasteqpreamp != cur_eqpreamp || forcecb)
			{
				m_lasteqpreamp = cur_eqpreamp;
				sendCoreCallback(CoreCallback::EQPREAMPCHANGE, cur_eqpreamp);
			}
		}
		break;

	case IPC_CB_MISC_INFO:
		{
			int realrate = wa2.getSamplerate();
			int brate = wa2.getBitrate();
			int ch = wa2.getChannels();
			int any = 0;
			if (realrate != m_lastsamplerate || forcecb)
			{
				m_lastsamplerate = realrate;
				sendCoreCallback(CoreCallback::SAMPLERATECHANGE, realrate);
				any = 1;
			}
			if (brate != m_lastbitrate || forcecb)
			{
				m_lastbitrate = brate;
				sendCoreCallback(CoreCallback::BITRATECHANGE, m_lastbitrate);
				any = 1;
			}
			if (ch != m_lastchan || forcecb)
			{
				m_lastchan = ch;
				sendCoreCallback(CoreCallback::CHANNELSCHANGE, m_lastchan);
				any = 1;
			}
			//DebugString("Got IPC_CB_MISC_INFO callback, numchans = %d, samplerate = %d, bitrate = %d\n", m_lastchan, m_lastsamplerate, m_lastbitrate);
			if (any)
			{
				StringW txt = getSongInfoText();
				sendCoreCallback(CoreCallback::INFOCHANGE, (intptr_t)txt.getValue());
			}
			break;
		}
	}
}

StringW infotext;

const wchar_t *Core::getSongInfoText()
{
	infotext = L"";
	int srate = wa2.getSamplerate();
	int brate = wa2.getBitrate();
	int ch = wa2.getChannels();

	if (srate == 0 && brate == 0)
	{
		return L"";
	}

	infotext = StringPrintfW(L"%dkbps", brate);

	if (ch == 1)
	{
		infotext += L" mono";
	}
	else if (ch == 2)
	{
		infotext += L" stereo";
	}
	else if (ch > 2)
	{
		infotext += StringPrintfW(L" %d Channels", ch);
	}

	infotext += StringPrintfW(L" %.1fkHz", (float)srate/1000.0f);

	if (wa2.isPlayingVideo())
	{
		infotext.prepend(L"Video ");
	}
	return infotext;
}

StringW infotextTranslated;

const wchar_t *Core::getSongInfoTextTranslated()
{
	infotextTranslated = L"";
	int srate = wa2.getSamplerate();
	int brate = wa2.getBitrate();
	int ch = wa2.getChannels();

	if (srate == 0 && brate == 0)
	{
		return L"";
	}

	infotextTranslated = StringPrintfW(L"%d%s", brate,WASABI_API_LNGSTRINGW(IDS_KBPS));

	if (ch == 1)
	{
		infotextTranslated += WASABI_API_LNGSTRINGW(IDS_MONO);
	}
	else if (ch == 2)
	{
		infotextTranslated += WASABI_API_LNGSTRINGW(IDS_STEREO);
	}
	else if (ch > 2)
	{
		infotextTranslated += StringPrintfW(WASABI_API_LNGSTRINGW(IDS_X_CHANNELS), ch);
	}

	infotextTranslated += StringPrintfW(L" %.1f", (float)srate/1000.0f);
	infotextTranslated += WASABI_API_LNGSTRINGW(IDS_KHZ);// (doing this in the StringPrintfW(..) above causes a crash even in a seperate buffer)

	if (wa2.isPlayingVideo())
	{
		infotextTranslated.prepend(StringPrintfW(L"%s ",WASABI_API_LNGSTRINGW(IDS_VIDEO)));
	}

	return infotextTranslated;
}

void Core::userButton(int button)
{
	int mod = Std::keyDown(VK_SHIFT) ? WA2_USERBUTTONMOD_SHIFT : (Std::keyDown(VK_SHIFT) ? WA2_USERBUTTONMOD_CTRL : WA2_USERBUTTONMOD_NONE);
	switch (button)
	{
	case UserButton::PLAY: wa2.userButton(WA2_USERBUTTON_PLAY, mod); break;
	case UserButton::STOP: wa2.userButton(WA2_USERBUTTON_STOP, mod); break;
	case UserButton::PAUSE: wa2.userButton(WA2_USERBUTTON_PAUSE, mod); break;
	case UserButton::NEXT: wa2.userButton(WA2_USERBUTTON_NEXT, mod); break;
	case UserButton::PREV: wa2.userButton(WA2_USERBUTTON_PREV, mod); break;
	}
}

void Core::setVolume(int vol)
{
	wa2.setVolume(vol);
}

int Core::getVolume()
{
	return wa2.getVolume();
}

void Core::setPosition(int ms)
{
	wa2.seekTo(ms);
	sendCoreCallback(CoreCallback::SEEKED, ms);
}

int Core::getPosition()
{
	if (m_laststatus == STATUS_STOP) return -1;
	return wa2.getPosition();
}

int Core::getLength()
{
	if (m_laststatus == STATUS_STOP) return -1;
	return wa2.getLength();
}

void Core::setPanning(int p)
{
	if (p > 127) p = 127;
	wa2.setPanning(p);
}

int Core::getPanning()
{
	return wa2.getPanning();
}

void Core::setShuffle(int shuffle)
{
	wa2.setShuffle(shuffle);
}

int Core::getShuffle()
{
	return wa2.getShuffle();
}

void Core::setRepeat(int repeat)
{
	static int myset = 0;
	if (!myset)
	{
		myset = 1;
		int manadv = 0;
		int rep = 0;
		if (repeat == 0)
		{
			rep = 0;
			manadv = 0;
		}
		else if (repeat > 0)
		{
			rep = 1;
			manadv = 0;
		}
		else if (repeat < 0)
		{
			rep = 1;
			manadv = 1;
		}
		if (!!wa2.getRepeat() != !!rep) wa2.setRepeat(rep);
		if (!!wa2.getManualPlaylistAdvance() != !!manadv) wa2.setManualPlaylistAdvance(manadv);
		myset = 0;
	}
}

int Core::getRepeat()
{
	int manadv = wa2.getManualPlaylistAdvance();
	int rep = wa2.getRepeat();
	int _v = (rep && manadv) ? -1 : rep;
	return _v;
}

int Core::getSamplerate(int wa2_getinfo)
{
	return wa2.getInfo(WA2_GETINFO_SAMPLERATE);
}

int Core::getBitrate(int wa2_getinfo)
{
	return wa2.getInfo(WA2_GETINFO_BITRATE);
}

int Core::getChannels(int wa2_getinfo)
{
	return wa2.getInfo(WA2_GETINFO_CHANNELS);
}

int Core::getEqBand(int band)
{
	//  int v = 63-wa2.getEqData(WA2_EQDATA_FIRSTBAND+band) * 4 - 127;
	//  v = MIN(-127, v);
	//  v = MAX(127, v);
	return m_lasteqband[band]; //(v);
}

void Core::setEqBand(int band, int val)
{
	val = MIN(127, MAX( -127, val));
	m_lasteqband[band] = val;
	int v = 63 - ((val + 127) / 4);
	v = MIN(63, v);
	v = MAX(0, v);
	wa2.setEqData(WA2_EQDATA_FIRSTBAND + band, v);
	sendCoreCallback(CoreCallback::EQBANDCHANGE, band, val);
}

int Core::getEQStatus()
{
	return wa2.getEqData(WA2_EQDATA_ENABLED);
}

void Core::setEQStatus(int enable)
{
	wa2.setEqData(WA2_EQDATA_ENABLED, enable);
}

int Core::getEQAuto()
{
	return wa2.getEqData(WA2_EQDATA_AUTO);
}

void Core::setEQAuto(int enable)
{
	wa2.setEqData(WA2_EQDATA_AUTO, enable);
}

int Core::getEQPreamp()
{
	//  int v = 63-wa2.getEqData(WA2_EQDATA_PREAMP) * 4 - 127;
	//  v = MIN(-127, v);
	//  v = MAX(127, v);
	return m_lasteqpreamp;
}

void Core::setEQPreamp(int val)
{
	val = MIN(127, MAX( -127, val));
	m_lasteqpreamp = val;
	int v = 63 - ((val + 127) / 4);
	v = MIN(63, v);
	v = MAX(0, v);
	wa2.setEqData(WA2_EQDATA_PREAMP, v);
	sendCoreCallback(CoreCallback::EQPREAMPCHANGE, val);
}

const wchar_t *Core::getTitle()
{
	return m_lasttitle; // faster to use our cached ver
	//  int plentry = wa2.getCurPlaylistEntry();
	//  if (plentry == -1) return NULL;
	//  return wa2.getTitle(plentry);
}

void Core::setTitle(const wchar_t * new_title)
{
	StringW title = new_title;
	if(title.isempty()) 
		title = L"";
	m_lasttitle = title;
}

const wchar_t *Core::getPlaystring()
{
	m_playstring = wa2.GetCurrentFile();
	if (m_playstring.getValue() == NULL) m_playstring = L"";
	return m_playstring;
}

int Core::getCurPlaylistEntry()
{
	return wa2.getCurPlaylistEntry();
}

// -----------------------------------------------------------------------------------------------------------------
// api_core implementation
// -----------------------------------------------------------------------------------------------------------------

// this pointer is set automagically by ApiInit
api_core *coreApi = NULL;

const wchar_t *Core::core_getSupportedExtensions()
{
	return L"mp3\x0";
}

const wchar_t *Core::core_getExtSupportedExtensions()
{
	return L"mp3\x0";
}

CoreToken Core::core_create()
{
	return 0;
}

int Core::core_free(CoreToken core)
{
	return 0;
}

int Core::core_setNextFile(CoreToken core, const wchar_t *playstring)
{
	return 0;
}

int Core::core_getStatus(CoreToken core)
{
	switch (getStatus())
	{
	case STATUS_STOP : return 0;
	case STATUS_PLAY : return 1;
	case STATUS_PAUSE : return -1;
	}
	return 0; // dunno, stopped i guess...
}

const wchar_t *Core::core_getCurrent(CoreToken core)
{
	return getPlaystring();
}

int Core::core_getCurPlaybackNumber(CoreToken core)
{
	return getCurPlaylistEntry();
}

int Core::core_getPosition(CoreToken core)
{
	return getPosition();
}

int Core::core_getWritePosition(CoreToken core)
{
	return getPosition();
}

int Core::core_setPosition(CoreToken core, int ms)
{
	setPosition(ms);
	return 1;
}

int Core::core_getLength(CoreToken core)
{
	return getLength();
}

int Core::core_getPluginData(const wchar_t *playstring, const wchar_t *name, wchar_t *data, int data_len, int data_type)
{
	return 0;
}

unsigned int Core::core_getVolume(CoreToken core)
{
	return getVolume();
}

void Core::core_setVolume(CoreToken core, unsigned int vol)
{
	setVolume(vol);
}

int Core::core_getPan(CoreToken core)
{
	return getPanning();
}

void Core::core_setPan(CoreToken core, int val)
{
	setPanning(val);
}

void Core::core_addCallback(CoreToken core, CoreCallback *cb)
{
	addCallback(cb);
}

void Core::core_delCallback(CoreToken core, CoreCallback *cb)
{
	delCallback(cb);
}

int Core::core_getVisData(CoreToken core, void *dataptr, int sizedataptr)
{
	int ret = 75*2;
	// added this to attempt to cope with some poor / old input plug-ins
	// which keep cropping up in the plug-in crash reports from users...
	try
	{
		// todo
		if (wa2.export_sa_get)
		{
			if (sizedataptr >= (75*2 + 8))
				wa2.export_sa_get((char *)dataptr);
			else
			{
				char data[75*2 + 8];
				char *p = wa2.export_sa_get(data);
				if (p) memcpy(dataptr, p, min(2*75, sizedataptr));
			}
		}
		else if (wa2.export_sa_get_deprecated)
		{
			char *p = wa2.export_sa_get_deprecated();
			if (p) memcpy(dataptr, p, min(2*75, sizedataptr));
		}
	}
	catch(...)
	{
		ret = 0;
	}
	return ret;
}

int Core::core_getLeftVuMeter(CoreToken core)
{
	if (wa2.export_vu_get)
	{
		int vu = wa2.export_vu_get(0);
		if (vu != -1)
			return vu;
	}
	 if (wa2.export_sa_get)
	{
		char data[75*2 + 8] = {0};
		char *p = (char *)wa2.export_sa_get(data);
		if (!p) return 0;

		int m = 0;
		for (int i = 75;i < 150;i++) // this is very gay but workish
			m = max(abs(m), p[i]);

		return MIN(255, m*16);
	}
	else if (wa2.export_sa_get_deprecated)
	{
		char *p = (char *)wa2.export_sa_get_deprecated();
		if (!p) return 0;

		int m = 0;
		for (int i = 75;i < 150;i++) // this is very gay but workish
			m = max(abs(m), p[i]);

		return MIN(255, m*16);
	}
	else return 0;

}

int Core::core_getRightVuMeter(CoreToken core)
{
	if (wa2.export_vu_get)
	{
		int vu = wa2.export_vu_get(1);
		if (vu == -1)
			vu = wa2.export_vu_get(0);
		if (vu != -1)
			return vu;
	}

	return core_getLeftVuMeter(core);
}

int Core::core_registerSequencer(CoreToken core, ItemSequencer *seq)
{
	return 0;
}

int Core::core_deregisterSequencer(CoreToken core, ItemSequencer *seq)
{
	return 0;
}

void Core::core_userButton(CoreToken core, int button)
{
	userButton(button);
}

int Core::core_getEqStatus(CoreToken core)
{
	return getEQStatus();
}

void Core::core_setEqStatus(CoreToken core, int enable)
{
	setEQStatus(enable);
}

int Core::core_getEqPreamp(CoreToken core)
{
	return getEQPreamp();
}

void Core::core_setEqPreamp(CoreToken core, int pre)
{
	setEQPreamp(pre);
}

int Core::core_getEqBand(CoreToken core, int band)
{
	return getEqBand(band);
}

void Core::core_setEqBand(CoreToken core, int band, int val)
{
	setEqBand(band, val);
}

int Core::core_getEqAuto(CoreToken core)
{
	return getEQAuto();
}

void Core::core_setEqAuto(CoreToken core, int enable)
{
	setEQAuto(enable);
}

void Core::core_setCustomMsg(CoreToken core, const wchar_t *text)
{}

void Core::core_registerExtension(const wchar_t *extensions, const wchar_t *extension_name, const wchar_t *family)
{}


const wchar_t *Core::core_getDecoderName(const wchar_t *filename)
{
	wchar_t fn[MAX_PATH+3] = {0};
	WCSNPRINTF(fn, MAX_PATH, L"hi.%s", filename);
	In_Module *player = (In_Module *)wa2.CanPlay(fn);
	if (player)
	{
		// cope nicely with the 5.64+ input plug-ins with unicode descriptions
		static wchar_t decoder_name[512];
		int ver = ((player->version & ~IN_UNICODE) & ~IN_INIT_RET);
		if (ver == IN_VER)
		{
			StringCchCopyW(decoder_name, 512, (wchar_t *)player->description);
		}
		else
		{
			MultiByteToWideCharSZ(CP_ACP, 0, player->description, -1, decoder_name, 512);
		}
		return decoder_name;
	}
	return NULL;
}

const wchar_t *Core::core_getExtensionFamily(const wchar_t *extension)
{
	wchar_t fn[MAX_PATH] = {0};
	WCSNPRINTF(fn, MAX_PATH, L"hi.%s", extension);
	In_Module *player = (In_Module *)wa2.CanPlay(fn);
	if (player)
	{
		/*
		char *p = player->FileExtensions;
		const wchar_t *realExt = PathFindExtensionW(fn);
		if (realExt && *realExt)
			realExt++;
		AutoChar ext(realExt);
		while (p && *p)
		{
			char *b = p;
			char *c;
			do
			{
				char d[20] = {0};
				lstrcpyn(d, b, 15);
				if ((c = strstr(b, ";")))
				{
					if ((c-b)<15)
						d[c - b] = 0;
				}

				if (!_stricmp(ext, d))
				{
					p += lstrlen(p) + 1; // skip to the name
					MultiByteToWideCharSZ(CP_ACP, 0, p, -1, family, 512);
					return family;
				}
				b = c + 1;
			}
			while (c);
			p += lstrlen(p) + 1;
			if (!*p) break;
			p += lstrlen(p) + 1;
		}
	
*/
		static wchar_t family[512];
		MultiByteToWideCharSZ(CP_ACP, 0, player->description, -1, family, 512);
		return family;
	}
	return NULL;
}

void Core::core_unregisterExtension(const wchar_t *extensions)
{}

const wchar_t *Core::core_getTitle(CoreToken core)
{
	return getTitle();
}

void Core::core_setTitle(const wchar_t *new_title)
{
	setTitle(new_title);
}

int Core::core_getRating()
{
	return wa2.getCurTrackRating();
}

void Core::core_setRating(int newRating)
{
	wa2.setCurTrackRating(newRating);
}
