#ifndef NULLSOFT_GEN_FF_ALBUMART_H
#define NULLSOFT_GEN_FF_ALBUMART_H
#include <api/wnd/wndclass/bufferpaintwnd.h>
#include <api/skin/widgets/layer.h>
#include <api/syscb/callbacks/metacb.h>
//#include <api/syscb/callbacks/corecbi.h>

//#define ALBUMART_PARENT GuiObjectWnd
//#define ALBUMART_PARENT BufferPaintWnd
#define ALBUMART_PARENT Layer

// {6DCB05E4-8AC4-48c2-B193-49F0910EF54A}
static const GUID albumArtGuid = 
{ 0x6dcb05e4, 0x8ac4, 0x48c2, { 0xb1, 0x93, 0x49, 0xf0, 0x91, 0xe, 0xf5, 0x4a } };


class AlbumArtScriptController : public LayerScriptController
{
  public:

    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController() { return layerController; }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid();
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  private:

    static function_descriptor_struct exportedFunction[];
    
};

extern AlbumArtScriptController *albumartController;

class AlbumArtThreadContext;

class AlbumArt : public ALBUMART_PARENT,
			public CoreCallbackI, /* to get song change updates */
			public MetadataCallbackI /* to find out when album art changes */
//			public SkinCallbackI  /* to get color theme changes */
{
public:
	AlbumArt();
	~AlbumArt();

protected:
	void metacb_ArtUpdated(const wchar_t *filename);
	virtual int corecb_onUrlChange(const wchar_t *filename);
	virtual int onInit();
	//virtual int onBufferPaint(BltCanvas *canvas, int w, int h);
	int setXuiParam(int _xuihandle, int attrid, const wchar_t *name, const wchar_t *strval);
	int skincb_onColorThemeChanged(const wchar_t *newcolortheme);
	
	bool LoadArt(AlbumArtThreadContext *context, HANDLE thread_to_notify);
	/* Layer */
	SkinBitmap *getBitmap();
	bool layer_isInvalid();
	void onSetVisible(int show);
/*
	virtual int getWidth();
	virtual int getHeight();
*/
	/*static */void CreateXMLParameters(int master_handle);
private:
	void layer_adjustDest(RECT *r);

	int		w,h;
	int		valign;		// 0 = center, 1 = bottom, -1 = top
	int		align;		// 0 = center, 1 = right, -1 = left
	bool	stretched;
	bool	forceRefresh;
	bool	noAutoRefresh;
	bool	noMakiCallback;
	volatile int	isLoading;

	void ArtLoaded(int _w, int _h, ARGB32 *_bits);
	StringW src_file;
	ARGB32 *bits;
	SkinBitmap *artBitmap;
	AutoSkinBitmap missing_art_image;
	volatile LONG iterator;
	HANDLE thread_semaphore, hMainThread;

	friend class AlbumArtThreadContext;
private:
	/* XML Parameters */
	enum
	{
		ALBUMART_NOTFOUNDIMAGE,
		ALBUMART_SOURCE,
		ALBUMART_VALIGN,
		ALBUMART_ALIGN,
		ALBUMART_STRETCHED,
		ALBUMART_NOREFRESH
	};
	static XMLParamPair params[];
	int xuihandle;

public:
	static scriptVar script_vcpu_refresh(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_onAlbumArtLoaded(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar success);
	static scriptVar script_vcpu_isLoading(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
};

extern const wchar_t albumArtXuiObjectStr[];
extern char albumArtXuiSvcName[];
class AlbumArtXuiSvc : public XuiObjectSvc<AlbumArt, albumArtXuiObjectStr, albumArtXuiSvcName> {};


#endif
