#ifndef _AUTOBITMAP_H
#define _AUTOBITMAP_H

#include "bitmap.h"
#include <api/syscb/callbacks/syscb.h>
#include <api/syscb/callbacks/skincb.h>
#include <tataki/export.h>

#ifdef DROP_BITMAP_ON_IDLE
#include <api/timer/timerclient.h>
#define DROP_BITMAP_ANCESTOR , public TimerClientDI
#else
#define DROP_BITMAP_ANCESTOR
#endif


class TATAKIAPI AutoSkinBitmap : public SysCallback DROP_BITMAP_ANCESTOR {
public:
	AutoSkinBitmap(const wchar_t *_name=NULL);
	virtual ~AutoSkinBitmap();

	const wchar_t *setBitmap(const wchar_t *_name=NULL);
	int setBitmap(int _id=0);

	// call this when you get freeResources called on you
	// doesn't hurt to call as much as you want
	void reset();
	void reload() { getBitmap(); }	// force a reload

	// this loads the bitmap if necessary
	SkinBitmap *getBitmap();
	operator SkinBitmap *() { return getBitmap(); }

	const wchar_t *operator =(const wchar_t *_name) { return setBitmap(_name); }
	int operator =(int _id) { return setBitmap(_id); }

	const wchar_t *getBitmapName();

	void setHInstanceBitmapColorGroup(const wchar_t *_colorgroup);

	enum
	{
		RESAMPLING_MODE_NONE = 0,
		RESAMPLING_MODE_SUPERSAMPLING = 1,
	};

	void setResamplingMode(int mode);
	int getResamplingMode();

	// feel free to add more methods here to help make using this class
	// transparent...
	int getWidth() { return getBitmap()->getWidth(); };
	int getHeight() { return getBitmap()->getHeight(); }; 
	void stretchToRectAlpha(ifc_canvas *canvas, RECT *r, int alpha=255) {
		getBitmap()->stretchToRectAlpha(canvas, r, alpha);
	}
	void stretchToRectAlpha(ifc_canvas *canvas, RECT *r, RECT *dest, int alpha=255) {
		getBitmap()->stretchToRectAlpha(canvas, r, dest, alpha);
	}
	void stretchToRect(ifc_canvas *canvas, RECT *r) {
		getBitmap()->stretchToRect(canvas, r);
	}
	void blitAlpha(ifc_canvas *canvas, int x, int y, int alpha=255) {
		getBitmap()->blitAlpha(canvas, x, y, alpha);
	}

#ifdef _WIN32
	void setHInstance(HINSTANCE hinstance); // use this if you use autoskinbitmap and resource in a wac
#endif
#ifdef DROP_BITMAP_ON_IDLE
	virtual void timerclient_timerCallback(int id);
#endif

protected:
	FOURCC getEventType() { return SysCallback::SKINCB; }
	int notify(int msg, intptr_t param1 = 0, intptr_t param2 = 0)
	{
		if (msg == SkinCallback::RESET)
			return skincb_onReset();
		else
			return 0;
	}

	int skincb_onReset();
#ifdef DROP_BITMAP_ON_IDLE
	virtual void tryUnload();
#endif

private:
	int use;
	int id;
	wchar_t *name;
	wchar_t *colorgroup;
	SkinBitmap *bitmap;
	int resamplingMode;
#ifdef _WIN32
	HINSTANCE myInstance;
#endif
#ifdef DROP_BITMAP_ON_IDLE
	uint32_t lastuse;
#endif
protected:
	RECVS_DISPATCH;
};

#endif
