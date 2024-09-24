#ifndef __GENFF_CORE_H
#define __GENFF_CORE_H

#include <bfc/string/StringW.h> 
//#include "../studio/bfc/timerclient.h"
#include <api/core/buttons.h>
#include <api/syscb/callbacks/corecbi.h>
#include <api/core/api_core.h>
namespace Agave
{
	#include "../Agave/Config/api_config.h"
}

#define STATUS_UNKNOWN -1
#define STATUS_STOP     0
#define STATUS_PLAY     1
#define STATUS_PAUSE    2

class Core : public api_coreI//, public CoreCallbackI
{ //, public TimerClientDI {
public:
	Core();
	virtual ~Core();

	void gotCallback(int wParam, int forcecb = 0);

	void addCallback(CoreCallback *cb);
	void delCallback(CoreCallback *cb);

	//    virtual void timerclient_timerCallback(int id);

	int getStatus();
	void userButton(int button);

	void setVolume(int vol);
	int getVolume();

	void setPosition(int ms);
	int getPosition();
	int getLength();

	void setPanning(int p);
	int getPanning();

	void setShuffle(int shuffle);
	int getShuffle();

	void setRepeat(int repeat);
	int getRepeat();

	int getSamplerate(int wa2_getinfo);
	int getBitrate(int wa2_getinfo);
	int getChannels(int wa2_getinfo);

	int getEqBand(int band);
	void setEqBand(int band, int val);

	int getEQStatus();
	void setEQStatus(int enable);

	int getEQAuto();
	void setEQAuto(int enable);

	int getEQPreamp();
	void setEQPreamp(int enable);

	const wchar_t *getTitle();
	void setTitle(const wchar_t * new_title);
	const wchar_t *getPlaystring();

	int getCurPlaylistEntry();

	static const wchar_t *getSongInfoText();
	static const wchar_t *getSongInfoTextTranslated();

	// api_core ------------------------------------------------------------------------


	virtual const wchar_t *core_getSupportedExtensions();
	virtual const wchar_t *core_getExtSupportedExtensions();
	virtual CoreToken core_create();
	virtual int core_free(CoreToken core);
	virtual int core_setNextFile(CoreToken core, const wchar_t *playstring);
	virtual int core_getStatus(CoreToken core);
	virtual const wchar_t *core_getCurrent(CoreToken core);
	virtual int core_getCurPlaybackNumber(CoreToken core);
	virtual int core_getPosition(CoreToken core);
	virtual int core_getWritePosition(CoreToken core);
	virtual int core_setPosition(CoreToken core, int ms);
	virtual int core_getLength(CoreToken core);
	virtual int core_getPluginData(const wchar_t *playstring, const wchar_t *name, wchar_t *data, int data_len, int data_type = 0);
	virtual unsigned int core_getVolume(CoreToken core);
	virtual void core_setVolume(CoreToken core, unsigned int vol);
	virtual int core_getPan(CoreToken core);
	virtual void core_setPan(CoreToken core, int val);
	virtual void core_addCallback(CoreToken core, CoreCallback *cb);
	virtual void core_delCallback(CoreToken core, CoreCallback *cb);
	virtual int core_getVisData(CoreToken core, void *dataptr, int sizedataptr);
	virtual int core_getLeftVuMeter(CoreToken core);
	virtual int core_getRightVuMeter(CoreToken core);
	virtual int core_registerSequencer(CoreToken core, ItemSequencer *seq);
	virtual int core_deregisterSequencer(CoreToken core, ItemSequencer *seq);
	virtual void core_userButton(CoreToken core, int button);
	virtual int core_getEqStatus(CoreToken core);
	virtual void core_setEqStatus(CoreToken core, int enable);
	virtual int core_getEqPreamp(CoreToken core);
	virtual void core_setEqPreamp(CoreToken core, int pre);
	virtual int core_getEqBand(CoreToken core, int band);
	virtual void core_setEqBand(CoreToken core, int band, int val);
	virtual int core_getEqAuto(CoreToken core);
	virtual void core_setEqAuto(CoreToken core, int enable);
	virtual void core_setCustomMsg(CoreToken core, const wchar_t *text);
	virtual void core_registerExtension(const wchar_t *extensions, const wchar_t *extension_name, const wchar_t *family = NULL);
	virtual const wchar_t *core_getExtensionFamily(const wchar_t *extension);
	virtual void core_unregisterExtension(const wchar_t *extensions);
	virtual const wchar_t *core_getTitle(CoreToken core);
	virtual void core_setTitle(const wchar_t *new_title);
	const wchar_t *core_getDecoderName(const wchar_t *Filename);
	virtual int core_getRating();
	virtual void core_setRating(int newRating);

private:
	void sendCoreCallback(int message, int param1 = 0, int param2 = 0);
	StringW m_lasttitle;
	StringW m_playstring, m_lastfile;
	int m_laststatus;
	int m_lastpos;
	int m_lastvol;
	int m_lastpan;
	int m_lasteqband[10];
	int m_lastfreqband;
	int m_lasteq;
	int m_lasteqauto;
	int m_lasteqpreamp;
	int m_lastchan;
	int m_lastbitrate;
	int m_lastsamplerate;
	int m_lastpeentry;

	PtrList<CoreCallback> callbacks;
	ReentryFilterObject rf;
	Agave::api_config *config;
};

extern Core *g_Core;

#endif
