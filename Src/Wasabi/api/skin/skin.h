#ifndef _SKIN_H
#define _SKIN_H

#include <api/skin/api_skin.h>

#include <bfc/platform/platform.h>
#include <api/wnd/basewnd.h>
#include <bfc/ptrlist.h>
#include <tataki/region/region.h>
#include <tataki/bitmap/autobitmap.h>
#include <bfc/string/bfcstring.h>
#include <api/wndmgr/container.h>
#include <api/xml/xmlreader.h>
#include <bfc/string/StringW.h>

class SkinBitmap;
#include "SkinVersion.h"

#define CB_SETSKINDEFERRED 0x5492

class SkinTimer;

class Skin
{
public:
	enum {
	    CHKSKIN_UNKNOWN = -1,
	    CHKSKIN_ISWA3 = 1,
	    CHKSKIN_ISWA3OLD = 2,
	    CHKSKIN_ISWA3FUTURE = 3,
	    CHKSKIN_ISWA2 = 4,
	    CHKSKIN_DISALLOWED = 5,
	};

	Skin();
	virtual ~Skin();

	static const wchar_t *getSkinName();
	static void setSkinName(const wchar_t *newskinname, const wchar_t *skinpath = NULL);
	static const wchar_t *getSkinPath();
	static const wchar_t *getDefaultSkinPath();
	static Skin *getCurSkin();

	void setBaseTexture(const wchar_t *b);

	//CUT  static int registerCallback(SkinCallback *cb);
	//CUT  static int deregisterCallback(SkinCallback *cb);

	static void renderBaseTexture(ifc_window *base, Skin *s, ifc_canvas *c, const RECT &r, ifc_window *dest, int alpha = 255);
	static void renderBaseTexture(ifc_window *s, ifc_canvas *c, const RECT &r, ifc_window *dest, int alpha = 255);

	static void invalidateBaseTexture(Skin *s);
	static void invalidateAllBaseTextures();
	static Skin *baseToSkin(ifc_window *b);
	static void registerBaseSkin(Skin *s, ifc_window *b);
	static Skin *unregisterBaseSkin(ifc_window *b);


	static void unloadAllBaseTextures();
	static void reloadAllBaseTextures();
	void _unloadBaseTexture();
	void _reloadBaseTexture();

	static wchar_t *enumLoadableSkins(int refresh = FALSE);

	static int checkSkin(const wchar_t *name);
	static void toggleSkin(const wchar_t *name, const wchar_t *skin_path = NULL, int deferred = 0);
	static void unloadSkin();
	static void parseSkinFilename(const wchar_t *filename, const wchar_t *incpath);
	static int isDefaultSkin;
	static void sendUnloadingCallback();
	static int sendAbortCallback(const wchar_t *skinname);
	static void sendResetCallback();
	static void sendReloadCallback();
	static void sendBeforeLoadingElementsCallback();
	static void sendGuiLoadedCallback();
	static void sendLoadedCallback();
	static int isSkinReady();
	static void setSkinReady(int i);
	static int isDynamicGroupReloadEnabled() { return enable_group_reload; }
	static void unloadSkinPart(int id);
	static int loadSkinPart(const wchar_t *xmlfile);
	static void main_notifySkinLoaded();
	static int isLoading() { return loading; }

	static int unloadResources();
	static int reloadResources();
	static bool isLoaded();
private:
	void rescaleBaseTexture(int w, int h);
	void _renderBaseTexture(ifc_window *base, ifc_canvas *c, const RECT &r, ifc_window *dest, int alpha);
	void _invalidateBaseTexture(void);
	void validateBaseTextureRect(RECT *r);

	AutoSkinBitmap *base;
	BltCanvas *scaled;
	int scale_x, scale_y;
	bool forceinvalid;
	int m_x, m_y;
	int lastw, lasth, maxw, maxh;
	bool resizing;
	RegionI *validRgn;
	static int loading;
	static int enable_group_reload;

	static PtrList<Skin> skinList;
	static PtrList<ifc_window> baseList;

	static StringW skinName;
	static StringW skinPath;
	static StringW defSkinPath;
	static int highest_id;
	static int reloadingskin;
	static int skin_loaded;

	static SkinTimer *deferedskinset;
};

class SkinTimer : public TimerClientDI
{
public :
	SkinTimer() {}
	virtual ~SkinTimer() {}

	void setSkinDeferred(const wchar_t *skinname)
	{
		skin = skinname;
		timerclient_postDeferredCallback(CB_SETSKINDEFERRED, 0);
	}

	virtual int timerclient_onDeferredCallback(intptr_t p1, intptr_t p2)
	{
		if (p1 == CB_SETSKINDEFERRED)
		{
			Skin::toggleSkin(skin);
			skin.trunc(0);
		}
		else
			return TimerClientDI::timerclient_onDeferredCallback(p1, p2);
		return 1;
	}

private:
	StringW skin;
};


extern Skin *tha;

#endif
