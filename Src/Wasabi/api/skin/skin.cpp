#include <precomp.h>
#include <api.h>
#include "skin.h"
#include <api/skin/skinbmps.h>
#include <tataki/canvas/bltcanvas.h>
#include <api/wnd/basewnd.h>
#include <tataki/bitmap/bitmap.h>
#include <bfc/parse/pathparse.h>
#include <bfc/file/readdir.h>
//#include <api/wac/main.h> // CUT!!
#include <api/skin/skinparse.h>
#include <api/wac/compon.h>

#ifdef WASABI_COMPILE_WNDMGR
#include <api/wndmgr/skinembed.h>
#endif

#include <api/script/vcpu.h>
#include <api/skin/skinelem.h>
#include <api/application/wkc.h>
#include <api/skin/gammamgr.h>
#include <api/wnd/wndtrack.h>

#ifdef WASABI_COMPILE_PAINTSETS
#include <api/wnd/paintset.h>
#endif

#include <bfc/string/StringW.h>

#ifdef WIN32
#include "resource.h"
#include "../Agave/Language/api_language.h"
#endif

// Version number has now a style of x.yz (same as gen_ff version)
#define SKIN_LEGACY_VERSION  80.f // oldest supported is 0.8
#define SKIN_VERSION        136.f // current version is 1.36

Skin *tha = NULL;


static wchar_t *loadSkinList = NULL, skName[64];

Skin::Skin()
{
	if (deferedskinset == NULL)
	{
		deferedskinset = new SkinTimer();
	}
	base = NULL;
	scaled = NULL;
	scale_x = 0;
	scale_y = 0;
	validRgn = NULL;
	resizing = FALSE;
}

Skin::~Skin()
{
	if (this == tha)
	{
		delete deferedskinset;
		deferedskinset = NULL;
	}
	delete validRgn;
	validRgn = NULL;
	delete base;
	delete scaled;
	if (this == tha)
	{
		if (loadSkinList)
		{
			FREE(loadSkinList);
			loadSkinList = NULL;
		}
	}
}

Skin *Skin::getCurSkin()
{
	return tha;
}

void Skin::setSkinName(const wchar_t *newskinname, const wchar_t *skinpath)
{
	if (newskinname)
		skinName = newskinname;
	else
		skinName = WASABI_API_LNGSTRINGW_BUF(IDS_NO_SKIN_LOADED_,skName,64);

	if (skinpath == NULL)
	{
		skinPath = WASABI_API_SKIN->getSkinsPath();
		skinPath.AppendFolder(newskinname);
	}
	else
	{
		skinPath = skinpath;
		skinPath.AddBackslash();
	}
}

const wchar_t *Skin::getSkinName()
{
	return skinName.getValue();
}

const wchar_t *Skin::getSkinPath()
{
	return skinPath.getValue();
}

const wchar_t *Skin::getDefaultSkinPath()
{
	defSkinPath = WASABI_API_SKIN->getSkinsPath();
	defSkinPath.AppendFolder(L"Default");
	return defSkinPath;
}

void Skin::setBaseTexture(const wchar_t *b)
{
	if (b == NULL)
	{
		delete base;
		base = NULL;
		return ;
	}
	base = new AutoSkinBitmap();
	base->setBitmap(b);
}

void Skin::rescaleBaseTexture(int w, int h)
{
	/*  if (scaled != NULL && scale_x == w && scale_y == h)
	    return;
	  if ((resizing && (w > m_x || h > m_y)) || (!resizing && (w != m_x || h != m_y)))
	    {
	    delete scaled;
	    int lw = resizing ? maxw : w;
	    int lh = resizing ? maxh : h;
	    scaled = new BltCanvas(lw , lh);
	    m_x = lw; m_y = lh;
	    api_region *reg = new api_region(0,0,lw,lh);
	    scaled->selectClipRgn(reg);
	    delete reg;
	    lastw = w;
	    lasth = h;
	    }
	  // Empty valid region
	  if (validRgn)
	    validRgn->empty();
	  else
	    validRgn = new api_region();
	  scale_x = w;
	  scale_y = h;*/
}

void Skin::invalidateBaseTexture(Skin *s)
{ //FG
	if (!s) s = tha;
	if (s) s->_invalidateBaseTexture();
}

void Skin::invalidateAllBaseTextures()
{ //FG
	tha->_invalidateBaseTexture();
	for (int i = 0;i < skinList.getNumItems();i++)
	{
		Skin *s = skinList.enumItem(i);
		s->_invalidateBaseTexture();
	}
}

void Skin::unloadAllBaseTextures()
{
	if (tha) tha->_unloadBaseTexture();
	for (int i = 0;i < skinList.getNumItems();i++)
	{
		Skin *s = skinList.enumItem(i);
		s->_unloadBaseTexture();
	}
}

void Skin::reloadAllBaseTextures()
{
	if (tha) tha->_reloadBaseTexture();
	for (int i = 0;i < skinList.getNumItems();i++)
	{
		Skin *s = skinList.enumItem(i);
		s->_reloadBaseTexture();
	}
	invalidateAllBaseTextures();
}

void Skin::_unloadBaseTexture()
{
	if (base)
		base->reset();
}

void Skin::_reloadBaseTexture()
{
	if (!tha) return ;

	if (base)
		base->reload();
}

void Skin::_invalidateBaseTexture(void)
{ //FG
	if (validRgn)
		validRgn->empty();
}

void Skin::registerBaseSkin(Skin *s, ifc_window *b)
{
	skinList.addItem(s);
	baseList.addItem(b);
}

Skin *Skin::unregisterBaseSkin(ifc_window *b)
{
	for (int i = 0;i < baseList.getNumItems();i++)
	{
		if (baseList.enumItem(i) == b)
		{
			Skin *s = skinList.enumItem(i);
			baseList.delByPos(i);
			skinList.delByPos(i);
			if (baseList.getNumItems() == 0)
				baseList.removeAll();
			if (skinList.getNumItems() == 0)
				skinList.removeAll();
			return s;
		}
	}
	return NULL;
}

Skin *Skin::baseToSkin(ifc_window *b)
{
	if (b == NULL) return NULL;
	for (int i = 0;i < baseList.getNumItems();i++)
		if (baseList.enumItem(i) == b)
			return skinList.enumItem(i);
	return NULL;
}

void Skin::renderBaseTexture(ifc_window *base, ifc_canvas *c, const RECT &r, ifc_window *dest, int alpha)
{
	renderBaseTexture(base, baseToSkin(base), c, r, dest, alpha);
}

void Skin::renderBaseTexture(ifc_window *base, Skin *s, ifc_canvas *c, const RECT &r, ifc_window *dest, int alpha)
{
	ASSERT(tha != NULL);
	if (!s)
	{
		DebugStringW(L"Warning, base texture from main wnd?!\n");
		s = tha;
	}

	s->_renderBaseTexture(base, c, r, dest, alpha);
}

void Skin::validateBaseTextureRect(RECT *r)
{
	/*  if (!base) {
	    ASSERT(!(tha && this == tha));
	    if (origbase)
	      base = new SkinBitmap(origbase, origbase->getWidth(), origbase->getHeight());
	    else
	      base = new SkinBitmap(tha->base, tha->base->getWidth(), tha->base->getHeight());
	    if (!base) return;
	  }
	 
		// make a region with the rect we have to draw
		api_region *newregion = new api_region(r);
	 
	  // check if newregion is enclosed in validRgn, put whatever is outside back into newregion
	  if (newregion->enclosed(validRgn, newregion)) {
	    delete newregion;
	    return;
	  }
		// compute projected coordinates
		RECT destRect, srcRect;
		newregion->getRgnBox(&destRect);
		srcRect.left = (int)(((float)destRect.left / scale_x) * base->getWidth());
		srcRect.right = (int)(((float)destRect.right / scale_x) * base->getWidth());
		srcRect.top = (int)(((float)destRect.top / scale_y) * base->getHeight());
		srcRect.bottom = (int)(((float)destRect.bottom / scale_y) * base->getHeight());
	 
		// stretch the relevant portion of the image
		base->stretchRectToRect(scaled, &srcRect, &destRect);
	 
		#if 0 //FG> debug purpose
		HDC dc = GetDC(NULL);
		BitBlt(dc, 0, 0, scale_x, scale_y, scaled->getHDC(), 0, 0, SRCCOPY);
		ReleaseDC(NULL, dc);
		#endif
	 
		// add this region to the valid region
		validRgn->add(newregion);
		delete newregion;*/
}

#define SAFEROUND(d) ((float)(int)d == d) ? (int)d : (d - (float)(int)d > 0) ? ((int)d)+1 : ((int)d)-1

// FG> Please change this only if you REALLY know what you are doing. this needs to account for basewnd
// coordinates (start from 0,0), as well as virtualwnd (relative to parent), at any depth (group holding
// texture as 2nd group of the tree, and rendering the base texture in a basewnd in a virtual in the group),
// and should handle resized textures and scaled windows. ooch

void Skin::_renderBaseTexture(ifc_window *wndbase, ifc_canvas *c, const RECT &r, ifc_window *dest, int alpha)
{
	// pick our basetexture
	AutoSkinBitmap *b = base ? base : tha->base;

	if (!b) return ;

	// srcRect is the source rectangle in the basetexture
	RECT srcRect;
	// destProjectedRect is the basetexture rectangle projected to dest coordinates
	RECT destProjectedRect;

	ifc_window *p = dest;
	POINT pt;
	int sx = 0, sy = 0;
	while (p && p != wndbase)
	{
		if (!p->isVirtual())
		{
			p->getPosition(&pt);
			sx += pt.x;
			sy += pt.y;
		}
		p = p->getParent();
	}
	ASSERT(p);

	wndbase->getNonClientRect(&destProjectedRect);
	destProjectedRect.left -= sx;
	destProjectedRect.top -= sy;
	destProjectedRect.right -= sx;
	destProjectedRect.bottom -= sy;

	srcRect.left = 0;
	srcRect.top = 0;
	srcRect.right = b->getBitmap()->getWidth();
	srcRect.bottom = b->getBitmap()->getHeight();

#if 0//CUT
	// NONPORTABLE
	HDC hdc = c->getHDC();
	HRGN oldRgn = CreateRectRgn(0, 0, 0, 0);
	HRGN newRgn = CreateRectRgnIndirect(&r);

	int cs = GetClipRgn(hdc, oldRgn);

	ExtSelectClipRgn(hdc, newRgn, (cs != 1) ? RGN_COPY : RGN_AND);

	b->getBitmap()->stretchToRectAlpha(c, &srcRect, &destProjectedRect, alpha);

	SelectClipRgn(hdc, cs ? oldRgn : NULL);

	DeleteObject(oldRgn);
	DeleteObject(newRgn);
#endif
	BaseCloneCanvas clone(c);
	RegionI oldRgn, newRgn(&r);
#ifdef _WIN32
	int cs = clone.getClipRgn(&oldRgn);
	if (cs) newRgn.andRegion(&oldRgn);
	clone.selectClipRgn(&newRgn);
	b->getBitmap()->stretchToRectAlpha(&clone, &srcRect, &destProjectedRect, alpha);
	clone.selectClipRgn(cs ? &oldRgn : NULL);
#else
#warning port me
	b->getBitmap()->stretchToRectAlpha(&clone, &srcRect, &destProjectedRect, alpha);
#endif
}

wchar_t *Skin::enumLoadableSkins(int refresh)
{
	static size_t loadSkinListSize = 1024;

	if (loadSkinList)
	{
		if (!refresh)
			return loadSkinList;
		FREE(loadSkinList);
	}

	loadSkinList = WMALLOC(loadSkinListSize);
	loadSkinList[0] = 0;

	int first = 1;
	ReadDir skins(L"skins");

	while (skins.next())
	{
		const wchar_t *filename = skins.getFilename();

		wchar_t *ext = const_cast<wchar_t *>(Wasabi::Std::extension(filename));

		if (skins.isDir() || !WCSICMP(ext, L"wal") ||
		        !WCSICMP(ext, L"wsz") || !WCSICMP(ext, L"zip"))
		{
			if (!skins.isDotDir() && !skins.isDotDotDir())
			{
				if (!skins.isDir() )
				{
					if (ext && *ext) *(ext - 1) = 0;
				}

				// check loadSkinList size
				if ((wcslen(loadSkinList) + wcslen(filename) + 2) > loadSkinListSize)
				{
					loadSkinListSize *= 2;
					loadSkinList = (wchar_t *)REALLOC(loadSkinList, sizeof(wchar_t) * loadSkinListSize);
				}

				if (!first)
					wcscat(loadSkinList, L"/");
				wcscat(loadSkinList, filename);
				first = 0;
			}
		}
	}

	return loadSkinList;
}

int Skin::loadSkinPart(const wchar_t *xmlfile)
{
#ifdef WASABI_COMPILE_COMPONENTS
	WasabiKernelController *wkc = Main::getKernelController();
	if (wkc && !wkc->testSkinFile(xmlfile)) return -1;
#endif

	int id = WASABI_API_PALETTE->newSkinPart();
	SkinElementsMgr::onBeforeLoadingScriptElements(xmlfile, id);
	SkinParser::loadScriptXml(xmlfile, id);
	SkinElementsMgr::onAfterLoadingScriptElements();
#ifdef WASABI_COMPILE_WNDMGR
	SkinParser::startupContainers(id);
#endif
	return id;
}

void Skin::unloadSkinPart(int skinpartid)
{
	SkinElementsMgr::unloadScriptElements(skinpartid);
	SkinParser::cleanupScript(skinpartid);
}

int Skin::checkSkin(const wchar_t *skinname)
{
	OSFILETYPE fh = WFOPEN(StringPathCombine(WASABI_API_SKIN->getSkinPath(), L"skin.xml"), WF_READONLY_BINARY);
	if (fh != OPEN_FAILED)
	{
		FCLOSE(fh);
		// ok it's a wa3 skin, now check the skin version number in the xml file
		SkinVersionXmlReader r(skinname);
		if (!r.getWalVersion()) return CHKSKIN_ISWA3OLD;
		#ifndef LC_NUMERIC
		  #define LC_NUMERIC 4
		#endif
		float v = (float)(WTOF(r.getWalVersion()) * 100); // Since wa5.51 we will do a check for x.yz style
		if (v < (SKIN_LEGACY_VERSION-0.5f)) return CHKSKIN_ISWA3OLD;
		if (v > (SKIN_VERSION+0.5f)) return CHKSKIN_ISWA3FUTURE;
		return CHKSKIN_ISWA3;
	}
	fh = WFOPEN(StringPathCombine(WASABI_API_SKIN->getSkinPath(), L"Main.bmp"), WF_READONLY_BINARY);
	if (fh != OPEN_FAILED)
	{
		FCLOSE(fh);
		return CHKSKIN_ISWA2;
	}
	return CHKSKIN_UNKNOWN;
}

void Skin::toggleSkin(const wchar_t *skin_name, const wchar_t *skin_path, int deferred)
{
	StringW skinName = skin_name;

	if (sendAbortCallback(skinName)) return ;

	enable_group_reload = 0;

	StringW oldSkinPath = skinPath;
	char title[32] = {0};
	setSkinName(skinName, skin_path);
	int skinType = checkSkin(skinName);
	skinPath = oldSkinPath;

#ifdef WASABI_COMPILE_COMPONENTS
	WasabiKernelController *wkc = Main::getKernelController();
	if (wkc && !wkc->testSkin(skinName)) skinType = CHKSKIN_DISALLOWED;
#endif

	switch (skinType)
	{
	case CHKSKIN_ISWA3OLD:
		{
#ifdef WIN32
			WASABI_API_WND->appdeactivation_setbypass(1);
			int ret = MessageBoxA(GetActiveWindow(), WASABI_API_LNGSTRING(IDS_SKIN_LOAD_FORMAT_OLD),
								 WASABI_API_LNGSTRING_BUF(IDS_SKIN_LOAD_WARNING,title,32),
								 MB_ICONWARNING | MB_YESNO);
			WASABI_API_WND->appdeactivation_setbypass(0);
			if (ret == IDNO) return ;
#else
			DebugString( "The skin you are trying to load is meant for an older Winamp3 version.\n" );
#endif
			break;
		}
	case CHKSKIN_ISWA3FUTURE:
		{
#ifdef WIN32
			WASABI_API_WND->appdeactivation_setbypass(1);
			int ret = MessageBoxA(GetActiveWindow(), WASABI_API_LNGSTRING(IDS_SKIN_LOAD_FORMAT_TOO_RECENT),
								 WASABI_API_LNGSTRING_BUF(IDS_SKIN_LOAD_WARNING,title,32),
								 MB_ICONWARNING | MB_YESNO);
			WASABI_API_WND->appdeactivation_setbypass(0);
			if (ret == IDNO) return ;
#else
			DebugString( "The skin you are trying to load is meant for an older Winamp3 version.\n" );
#endif
			break;
		}
	case CHKSKIN_UNKNOWN:
#ifdef WIN32
		WASABI_API_WND->appdeactivation_setbypass(1);
		MessageBoxA(GetActiveWindow(), WASABI_API_LNGSTRING(IDS_SKIN_LOAD_NOT_SUPPORTED),
		           WASABI_API_LNGSTRING_BUF(IDS_ERROR,title,32), MB_ICONERROR);
		WASABI_API_WND->appdeactivation_setbypass(0);
#else
		DebugString( "The skin you are trying to load is meant for an older Winamp3 version.\n" );
#endif
		return ;

	case CHKSKIN_DISALLOWED:

		// kernel controller should output its own error message

		return ;

	case CHKSKIN_ISWA2: break;
	}

	WASABI_API_COLORTHEMES->StartTransaction();
	if (skin_loaded)
	{
		sendUnloadingCallback();
	}

	loading = 1;

	if (skin_loaded)
	{
		//ComponentManager::detachAllTemporary();
		//ComponentManager::destroyAllCompContainer();

#ifdef WASABI_COMPILE_WNDMGR
 #ifdef WASABI_COMPILE_CONFIG
 #ifndef WASABI_WNDMGR_NORESPAWN
		skinEmbedder->saveState();
#endif
 #endif
 #endif

		// unload current skin
		SkinParser::cleanUp();

		delete(tha);
		tha = NULL;
		//delete(VCPU::scriptManager);

		unloadResources();

		// TODO: benski> unload WAC files inside skin.
		// we should have saved a list of WacComponent * when loading
		// add a new method ComponentManager::unload(WacComponent *);

		Skin::sendResetCallback();

		//  	VCPU::scriptManager = new ScriptObjectManager();
		Skin *n = new Skin;
		tha = n;
	}

	setSkinName(skinName, skin_path);

	if (skin_loaded)
	{
		SkinElementsMgr::resetSkinElements();
		//SkinElementsMgr::loadSkinElements(skinName); // only loads element definitions, not actual bitmaps

		sendReloadCallback();

		reloadResources();
	}

	// TODO: benski> load WAC files inside skin.  save list of WacComponent * to a list to unload later
	// make ComponentManager::load() return the WacComponent to allow this

#ifdef WASABI_COMPILE_WNDMGR
	int ncont = SkinParser::loadContainers(skinName); //sends guiloaded cb
#endif

	GammaMgr::loadDefault();

#ifdef WASABI_COMPILE_WNDMGR
	SkinParser::startupContainers();
#ifdef WASABI_COMPILE_CONFIG
#ifndef WASABI_WNDMGR_NORESPAWN
	skinEmbedder->restoreSavedState();
#endif
#endif
#endif

	enable_group_reload = 1;

	sendLoadedCallback();

#ifdef WASABI_COMPILE_WNDMGR
	SkinParser::centerSkin();
#endif

	loading = 0;

#ifdef WASABI_COMPILE_WNDMGR
#ifdef WA3COMPATIBILITY
	if (ncont == 0)
		SkinParser::emmergencyReloadDefaultSkin();
#endif
#endif
	skin_loaded = 1;
	WASABI_API_COLORTHEMES->EndTransaction();
}

void Skin::unloadSkin()
{
	if (!skin_loaded) return ;

	sendUnloadingCallback();

	loading = -1;

#ifdef WASABI_COMPILE_WNDMGR
 #ifdef WASABI_COMPILE_CONFIG
 #ifndef WASABI_WNDMGR_NORESPAWN
	skinEmbedder->saveState();
#endif
 #endif
 #endif

	// unload current skin
	SkinParser::cleanUp();

	delete(tha);
	tha = NULL;
	//delete(VCPU::scriptManager);

	unloadResources();

	Skin::sendResetCallback();

	//  	VCPU::scriptManager = new ScriptObjectManager();
	Skin *n = new Skin;
	tha = n;

	setSkinName(WASABI_API_LNGSTRINGW_BUF(IDS_NO_SKIN_LOADED_,skName,64));

	SkinElementsMgr::resetSkinElements();
	//SkinElementsMgr::loadSkinElements(skinName); // only loads element definitions, not actual bitmaps

	sendReloadCallback();

	reloadResources();
	loading = 0;
	skin_loaded = 0;
}

void Skin::sendUnloadingCallback()
{
#if defined(WASABI_COMPILE_COMPONENTS) | defined(GEN_FF) // MULTIAPI-FIXME!!
	ComponentManager::broadcastNotify(WAC_NOTIFY_SKINUNLOADING, WASABI_API_PALETTE->getSkinPartIterator());
#endif
	WASABI_API_SYSCB->syscb_issueCallback(SysCallback::SKINCB, SkinCallback::UNLOADING);
}

int Skin::sendAbortCallback(const wchar_t *skinname)
{
	int a = 0;
	WASABI_API_SYSCB->syscb_issueCallback(SysCallback::SKINCB, SkinCallback::CHECKPREVENTSWITCH, (intptr_t)skinname, (intptr_t)&a);
	return a;
}

void Skin::sendResetCallback()
{
	WASABI_API_SYSCB->syscb_issueCallback(SysCallback::SKINCB, SkinCallback::RESET);
}

void Skin::sendReloadCallback()
{
#if defined(WASABI_COMPILE_COMPONENTS) | defined(GEN_FF) // MULTIAPI-FIXME!!
	ComponentManager::broadcastNotify(WAC_NOTIFY_SWITCHINGSKIN, WASABI_API_PALETTE->getSkinPartIterator()); // this msg primilarily here to insert stuff between unloading of the old skin and reloading of the new one
#endif
	WASABI_API_SYSCB->syscb_issueCallback(SysCallback::SKINCB, SkinCallback::RELOAD);
}

void Skin::sendBeforeLoadingElementsCallback()
{
#if defined(WASABI_COMPILE_COMPONENTS) | defined(GEN_FF) // MULTIAPI-FIXME!!
	ComponentManager::broadcastNotify(WAC_NOTIFY_BEFORELOADINGSKINELEMENTS, WASABI_API_PALETTE->getSkinPartIterator()); // this msg primilarily here to insert stuff between unloading of the old skin and reloading of the new one
#endif
	WASABI_API_SYSCB->syscb_issueCallback(SysCallback::SKINCB, SkinCallback::BEFORELOADINGELEMENTS);
}

void Skin::sendGuiLoadedCallback()
{
	WASABI_API_SYSCB->syscb_issueCallback(SysCallback::SKINCB, SkinCallback::GUILOADED);
}

void Skin::sendLoadedCallback()
{
#if defined(WASABI_COMPILE_COMPONENTS) | defined(GEN_FF) // MULTIAPI-FIXME!!
	ComponentManager::broadcastNotify(WAC_NOTIFY_SKINLOADED, WASABI_API_PALETTE->getSkinPartIterator());
#endif
	WASABI_API_SYSCB->syscb_issueCallback(SysCallback::SKINCB, SkinCallback::LOADED);
}

void Skin::setSkinReady(int i)
{
	loading = !i;
}

void Skin::main_notifySkinLoaded()
{
	skin_loaded = 1;
}

int Skin::isSkinReady()
{
	return !loading;
}

int Skin::unloadResources()
{
	if (windowTracker)
	{
		for (int i = 0;i < windowTracker->getNumAllWindows();i++)
		{
			ifc_window *w = windowTracker->enumAllWindows(i);
#ifdef _WIN32
			if (w) w->wndProc(w->gethWnd(), WM_WA_RELOAD, 0, 0);
#else
#warning port me
#endif
		}
		Skin::unloadAllBaseTextures();
#ifdef WASABI_COMPILE_PAINTSETS
		paintset_reset();
#endif

	}

	sendResetCallback();

	return 1;
}

int Skin::reloadResources()
{
	if (windowTracker)
	{
		for (int i = 0;i < windowTracker->getNumAllWindows();i++)
		{
			ifc_window *w = windowTracker->enumAllWindows(i);
#ifdef _WIN32
			if (w) w->wndProc(w->gethWnd(), WM_WA_RELOAD, 1, 0);
#else
#warning port me
#endif
		}
		Skin::reloadAllBaseTextures();
	}

	sendReloadCallback();

	return 1;
}

bool Skin::isLoaded()
{
	return !!skin_loaded;
}

PtrList<Skin> Skin::skinList;
PtrList<ifc_window> Skin::baseList;
StringW Skin::skinName;
StringW Skin::skinPath;
int Skin::isDefaultSkin = 0;
int Skin::loading = 0;
int Skin::highest_id = 0;
int Skin::reloadingskin = 0;
int Skin::enable_group_reload = 0;
StringW Skin::defSkinPath;
SkinTimer *Skin::deferedskinset = NULL;
int Skin::skin_loaded = 0;