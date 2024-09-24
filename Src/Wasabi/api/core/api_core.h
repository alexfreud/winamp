#ifndef __API_CORE_H
#define __API_CORE_H

#include <bfc/dispatch.h>

typedef unsigned int CoreToken;
class CoreCallback;
class ItemSequencer;

class NOVTABLE api_core : public Dispatchable
{
public:
	const wchar_t *core_getSupportedExtensions();
	const wchar_t *core_getExtSupportedExtensions();
	CoreToken core_create();
	int core_free(CoreToken core);
	int core_setNextFile(CoreToken core, const wchar_t *playstring);
	int core_getStatus(CoreToken core);
	const wchar_t *core_getCurrent(CoreToken core);
	int core_getCurPlaybackNumber(CoreToken core);
	int core_getPosition(CoreToken core);
	int core_getWritePosition(CoreToken core);
	int core_setPosition(CoreToken core, int ms);
	int core_getLength(CoreToken core);
	int core_getPluginData(const wchar_t *playstring, const wchar_t *name, wchar_t *data, int data_len, int data_type = 0);
	unsigned int core_getVolume(CoreToken core);
	void core_setVolume(CoreToken core, unsigned int vol);
	int core_getPan(CoreToken core);
	void core_setPan(CoreToken core, int val);
	void core_addCallback(CoreToken core, CoreCallback *cb);
	void core_delCallback(CoreToken core, CoreCallback *cb);
	int core_getVisData(CoreToken core, void *dataptr, int sizedataptr);
	int core_getLeftVuMeter(CoreToken core);
	int core_getRightVuMeter(CoreToken core);
	int core_registerSequencer(CoreToken core, ItemSequencer *seq);
	int core_deregisterSequencer(CoreToken core, ItemSequencer *seq);
	void core_userButton(CoreToken core, int button);
	int core_getEqStatus(CoreToken core);
	void core_setEqStatus(CoreToken core, int enable);
	int core_getEqPreamp(CoreToken core);
	void core_setEqPreamp(CoreToken core, int pre);
	int core_getEqBand(CoreToken core, int band);
	void core_setEqBand(CoreToken core, int band, int val);
	int core_getEqAuto(CoreToken core);
	void core_setEqAuto(CoreToken core, int enable);
	void core_setCustomMsg(CoreToken core, const wchar_t *text);
	void core_registerExtension(const wchar_t *extensions, const wchar_t *extension_name, const wchar_t *family = NULL);
	const wchar_t *core_getExtensionFamily(const wchar_t *extension);
	void core_unregisterExtension(const wchar_t *extensions);
	const wchar_t *core_getTitle(CoreToken core);
	void core_setTitle(const wchar_t *new_title);
	const wchar_t *core_getDecoderName(const wchar_t *filename);
	// these don't necessarily belong here, but api_core is already over-bloated :)
	int core_getRating();
	void core_setRating(int newRating);

	enum
	{
	    API_CORE_GETSUPPORTEDEXTENSIONS = 0,
	    API_CORE_GETEXTSUPPORTEDEXTENSIONS = 10,
	    API_CORE_CREATE = 20,
	    API_CORE_FREE = 30,
	    API_CORE_SETNEXTFILE = 40,
	    API_CORE_GETSTATUS = 50,
	    API_CORE_GETCURRENT = 60,
	    API_CORE_GETCURPLAYBACKNUMBER = 70,
	    API_CORE_GETPOSITION = 80,
	    API_CORE_GETWRITEPOSITION = 90,
	    API_CORE_SETPOSITION = 100,
	    API_CORE_GETLENGTH = 110,
	    API_CORE_GETPLUGINDATA = 120,
	    API_CORE_GETVOLUME = 130,
	    API_CORE_SETVOLUME = 140,
	    API_CORE_GETPAN = 150,
	    API_CORE_SETPAN = 160,
	    API_CORE_ADDCALLBACK = 170,
	    API_CORE_DELCALLBACK = 180,
	    API_CORE_GETVISDATA = 190,
	    API_CORE_GETLEFTVUMETER = 200,
	    API_CORE_GETRIGHTVUMETER = 210,
	    API_CORE_REGISTERSEQUENCER = 220,
	    API_CORE_DEREGISTERSEQUENCER = 230,
	    API_CORE_USERBUTTON = 240,
	    API_CORE_GETEQSTATUS = 250,
	    API_CORE_SETEQSTATUS = 260,
	    API_CORE_GETEQPREAMP = 270,
	    API_CORE_SETEQPREAMP = 280,
	    API_CORE_GETEQBAND = 290,
	    API_CORE_SETEQBAND = 300,
	    API_CORE_GETEQAUTO = 310,
	    API_CORE_SETEQAUTO = 320,
	    API_CORE_SETCUSTOMMSG = 330,
	    API_CORE_REGISTEREXTENSION = 340,
	    API_CORE_GETEXTENSIONFAMILY = 350,
	    API_CORE_UNREGISTEREXTENSION = 360,
	    API_CORE_GETTITLE = 370,
		API_CORE_GETRATING = 380,
		API_CORE_SETRATING = 390,
		API_CORE_GETDECODERNAME = 400,
		API_CORE_SETTITLE = 410,
	};
};

inline const wchar_t *api_core::core_getSupportedExtensions()
{
	return _call(API_CORE_GETSUPPORTEDEXTENSIONS, (const wchar_t *)0);
}

inline const wchar_t *api_core::core_getExtSupportedExtensions()
{
	return _call(API_CORE_GETEXTSUPPORTEDEXTENSIONS, (const wchar_t *)0);
}

inline CoreToken api_core::core_create()
{
	return _call(API_CORE_CREATE, (CoreToken)NULL);
}

inline int api_core::core_free(CoreToken core)
{
	return _call(API_CORE_FREE, (int)0, core);
}

inline int api_core::core_setNextFile(CoreToken core, const wchar_t *playstring)
{
	return _call(API_CORE_SETNEXTFILE, (int)0, core, playstring);
}

inline int api_core::core_getStatus(CoreToken core)
{
	return _call(API_CORE_GETSTATUS, (int)0, core);
}

inline const wchar_t *api_core::core_getCurrent(CoreToken core)
{
	return _call(API_CORE_GETCURRENT, (const wchar_t *)0, core);
}

inline int api_core::core_getCurPlaybackNumber(CoreToken core)
{
	return _call(API_CORE_GETCURPLAYBACKNUMBER, (int)0, core);
}

inline int api_core::core_getPosition(CoreToken core)
{
	return _call(API_CORE_GETPOSITION, (int)0, core);
}

inline int api_core::core_getWritePosition(CoreToken core)
{
	return _call(API_CORE_GETWRITEPOSITION, (int)0, core);
}

inline int api_core::core_setPosition(CoreToken core, int ms)
{
	return _call(API_CORE_SETPOSITION, (int)0, core, ms);
}

inline int api_core::core_getLength(CoreToken core)
{
	return _call(API_CORE_GETLENGTH, (int)0, core);
}

inline int api_core::core_getPluginData(const wchar_t *playstring, const wchar_t *name, wchar_t *data, int data_len, int data_type)
{
	return _call(API_CORE_GETPLUGINDATA, (int)0, playstring, name, data, data_len, data_type);
}

inline unsigned int api_core::core_getVolume(CoreToken core)
{
	return _call(API_CORE_GETVOLUME, (unsigned int)0, core);
}

inline void api_core::core_setVolume(CoreToken core, unsigned int vol)
{
	_voidcall(API_CORE_SETVOLUME, core, vol);
}

inline int api_core::core_getPan(CoreToken core)
{
	return _call(API_CORE_GETPAN, (int)0, core);
}

inline void api_core::core_setPan(CoreToken core, int val)
{
	_voidcall(API_CORE_SETPAN, core, val);
}

inline void api_core::core_addCallback(CoreToken core, CoreCallback *cb)
{
	_voidcall(API_CORE_ADDCALLBACK, core, cb);
}

inline void api_core::core_delCallback(CoreToken core, CoreCallback *cb)
{
	_voidcall(API_CORE_DELCALLBACK, core, cb);
}

inline int api_core::core_getVisData(CoreToken core, void *dataptr, int sizedataptr)
{
	return _call(API_CORE_GETVISDATA, (int)0, core, dataptr, sizedataptr);
}

inline int api_core::core_getLeftVuMeter(CoreToken core)
{
	return _call(API_CORE_GETLEFTVUMETER, (int)0, core);
}

inline int api_core::core_getRightVuMeter(CoreToken core)
{
	return _call(API_CORE_GETRIGHTVUMETER, (int)0, core);
}

inline int api_core::core_registerSequencer(CoreToken core, ItemSequencer *seq)
{
	return _call(API_CORE_REGISTERSEQUENCER, (int)0, core, seq);
}

inline int api_core::core_deregisterSequencer(CoreToken core, ItemSequencer *seq)
{
	return _call(API_CORE_DEREGISTERSEQUENCER, (int)0, core, seq);
}

inline void api_core::core_userButton(CoreToken core, int button)
{
	_voidcall(API_CORE_USERBUTTON, core, button);
}

inline int api_core::core_getEqStatus(CoreToken core)
{
	return _call(API_CORE_GETEQSTATUS, (int)0, core);
}

inline void api_core::core_setEqStatus(CoreToken core, int enable)
{
	_voidcall(API_CORE_SETEQSTATUS, core, enable);
}

inline int api_core::core_getEqPreamp(CoreToken core)
{
	return _call(API_CORE_GETEQPREAMP, (int)0, core);
}

inline void api_core::core_setEqPreamp(CoreToken core, int pre)
{
	_voidcall(API_CORE_SETEQPREAMP, core, pre);
}

inline int api_core::core_getEqBand(CoreToken core, int band)
{
	return _call(API_CORE_GETEQBAND, (int)0, core, band);
}

inline void api_core::core_setEqBand(CoreToken core, int band, int val)
{
	_voidcall(API_CORE_SETEQBAND, core, band, val);
}

inline int api_core::core_getEqAuto(CoreToken core)
{
	return _call(API_CORE_GETEQAUTO, (int)0, core);
}

inline void api_core::core_setEqAuto(CoreToken core, int enable)
{
	_voidcall(API_CORE_SETEQAUTO, core, enable);
}

inline void api_core::core_setCustomMsg(CoreToken core, const wchar_t *text)
{
	_voidcall(API_CORE_SETCUSTOMMSG, core, text);
}

inline void api_core::core_registerExtension(const wchar_t *extensions, const wchar_t *extension_name, const wchar_t *family)
{
	_voidcall(API_CORE_REGISTEREXTENSION, extensions, extension_name, family);
}

inline const wchar_t *api_core::core_getExtensionFamily(const wchar_t *extension)
{
	return _call(API_CORE_GETEXTENSIONFAMILY, (const wchar_t *)0, extension);
}

inline void api_core::core_unregisterExtension(const wchar_t *extensions)
{
	_voidcall(API_CORE_UNREGISTEREXTENSION, extensions);
}

inline const wchar_t *api_core::core_getTitle(CoreToken core)
{
	return _call(API_CORE_GETTITLE, (const wchar_t *)0, core);
}

inline void api_core::core_setTitle(const wchar_t *new_title)
{
	_voidcall(API_CORE_SETTITLE, new_title);
}

inline int api_core::core_getRating()
{
	return _call(API_CORE_GETRATING, (int)0);
}

	inline void api_core::core_setRating(int newRating)
	{
		_voidcall(API_CORE_SETRATING, newRating);
	}

	inline const wchar_t *api_core::core_getDecoderName(const wchar_t *filename)
	{
		return _call(API_CORE_GETDECODERNAME, (const wchar_t *)0, filename);
	}

class api_coreI : public api_core
{
public:
	virtual const wchar_t *core_getSupportedExtensions() = 0;
	virtual const wchar_t *core_getExtSupportedExtensions() = 0;
	virtual CoreToken core_create() = 0;
	virtual int core_free(CoreToken core) = 0;
	virtual int core_setNextFile(CoreToken core, const wchar_t *playstring) = 0;
	virtual int core_getStatus(CoreToken core) = 0;
	virtual const wchar_t *core_getCurrent(CoreToken core) = 0;
	virtual int core_getCurPlaybackNumber(CoreToken core) = 0;
	virtual int core_getPosition(CoreToken core) = 0;
	virtual int core_getWritePosition(CoreToken core) = 0;
	virtual int core_setPosition(CoreToken core, int ms) = 0;
	virtual int core_getLength(CoreToken core) = 0;
	virtual int core_getPluginData(const wchar_t *playstring, const wchar_t *name, wchar_t *data, int data_len, int data_type = 0) = 0;
	virtual unsigned int core_getVolume(CoreToken core) = 0;
	virtual void core_setVolume(CoreToken core, unsigned int vol) = 0;
	virtual int core_getPan(CoreToken core) = 0;
	virtual void core_setPan(CoreToken core, int val) = 0;
	virtual void core_addCallback(CoreToken core, CoreCallback *cb) = 0;
	virtual void core_delCallback(CoreToken core, CoreCallback *cb) = 0;
	virtual int core_getVisData(CoreToken core, void *dataptr, int sizedataptr) = 0;
	virtual int core_getLeftVuMeter(CoreToken core) = 0;
	virtual int core_getRightVuMeter(CoreToken core) = 0;
	virtual int core_registerSequencer(CoreToken core, ItemSequencer *seq) = 0;
	virtual int core_deregisterSequencer(CoreToken core, ItemSequencer *seq) = 0;
	virtual void core_userButton(CoreToken core, int button) = 0;
	virtual int core_getEqStatus(CoreToken core) = 0;
	virtual void core_setEqStatus(CoreToken core, int enable) = 0;
	virtual int core_getEqPreamp(CoreToken core) = 0;
	virtual void core_setEqPreamp(CoreToken core, int pre) = 0;
	virtual int core_getEqBand(CoreToken core, int band) = 0;
	virtual void core_setEqBand(CoreToken core, int band, int val) = 0;
	virtual int core_getEqAuto(CoreToken core) = 0;
	virtual void core_setEqAuto(CoreToken core, int enable) = 0;
	virtual void core_setCustomMsg(CoreToken core, const wchar_t *text) = 0;
	virtual void core_registerExtension(const wchar_t *extensions, const wchar_t *extension_name, const wchar_t *family = NULL) = 0;
	virtual const wchar_t *core_getExtensionFamily(const wchar_t *extension) = 0;
	virtual void core_unregisterExtension(const wchar_t *extensions) = 0;
	virtual const wchar_t *core_getTitle(CoreToken core) = 0;
	virtual void core_setTitle(const wchar_t *new_title) = 0;
	virtual int core_getRating()=0;
	virtual void core_setRating(int newRating)=0;
	virtual const wchar_t *core_getDecoderName(const wchar_t *filename)=0;


protected:
	RECVS_DISPATCH;
};

// {966E3DA1-C2C5-43a9-A931-EB5F8B040A4F}
static const GUID coreApiServiceGuid =
    { 0x966e3da1, 0xc2c5, 0x43a9, { 0xa9, 0x31, 0xeb, 0x5f, 0x8b, 0x4, 0xa, 0x4f } };

extern api_core *coreApi;

#endif
