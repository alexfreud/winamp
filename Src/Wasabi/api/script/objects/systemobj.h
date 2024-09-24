#ifndef _SYSTEMOBJ_H
#define _SYSTEMOBJ_H

#include <api/script/script.h>
#include <api/script/objects/rootobj.h>
#include <api/script/objects/timer.h>
#ifdef WASABI_COMPILE_MEDIACORE
#include <api/syscb/callbacks/corecbi.h>
#endif
#include <api/syscb/callbacks/browsercbi.h>
#include <bfc/string/bfcstring.h>

#ifdef WASABI_COMPILE_WNDMGR
class WindowHolder;
#endif

// {D6F50F64-93FA-49b7-93F1-BA66EFAE3E98}
static const GUID systemObjectGuid = 
{ 0xd6f50f64, 0x93fa, 0x49b7, { 0x93, 0xf1, 0xba, 0x66, 0xef, 0xae, 0x3e, 0x98 } };

class SystemScriptObjectController : public ScriptObjectControllerI {
  public:
	virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController() { return rootScriptObjectController; }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid();
    virtual int getInstantiable();
    virtual int getReferenceable();
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  private:
    static function_descriptor_struct exportedFunction[];
};

extern SystemScriptObjectController *systemController;

class Group;

#ifdef WASABI_COMPILE_WNDMGR
class Layout;
class Container;
class WindowHolder;
#endif

#ifdef WASABI_COMPILE_COMPONENTS
class WACObject;
#endif

#define SYSTEMOBJECT_SCRIPTPARENT RootObjectInstance

#ifdef WASABI_COMPILE_MEDIACORE
#define SYSTEMOBJECT_PARENT SYSTEMOBJECT_SCRIPTPARENT, public CoreCallbackI
#else
#define SYSTEMOBJECT_PARENT SYSTEMOBJECT_SCRIPTPARENT
#endif

class SystemObject : public SYSTEMOBJECT_PARENT, public BrowserCallbackI {
  public:
	SystemObject();
	virtual ~SystemObject();
	void setScriptId(int id);
	int getScriptId();
	void setTimer(int delay);
	void setParam(const wchar_t *p);
	void setParentGroup(Group *g);
	Group *getParentGroup();
	const wchar_t *getParam();
	virtual void onLoad();
	virtual void onUnload();
	virtual int isLoaded() { return loaded; }
#ifdef WASABI_COMPILE_WNDMGR
	WindowHolder *getSuitableWindowHolderByGuid(GUID g);
	int onGetCancelComponent(GUID g, int i);
	static void onCreateLayout(Layout *l);
	static void onShowLayout(Layout *l);
	static void onHideLayout(Layout *l);
	static int getCurAppRect(RECT *r);
#endif
	static void onQuit();
	static void onKeyDown(const wchar_t *s);
	static void onKeyUp(const wchar_t *s);
	static int onAccelerator(const wchar_t *action, const wchar_t *section, const wchar_t *key);
	TList < int > *getTypesList();
	void setIsOldFormat(int is);
	int isOldFormat();
	static int isAppActive();

	static void onViewPortChanged(int width, int height);
	static int onShowNotification();
	static void onDownloadFinished(const wchar_t * url, boolean success, const wchar_t * filename);

	static PtrList < ScriptObject > *getAllScriptObjects() { return &scriptobjects; }
	void setSkinPartId(int _skinpartid);
	int getSkinPartId();

	void addInstantiatedObject(ScriptObject *obj);
	void removeInstantiatedObject(ScriptObject *obj);
	void garbageCollect();

	static int isObjectValid(ScriptObject *o);
	static void addScriptObject(ScriptObject *o);
	static void removeScriptObject(ScriptObject *o);

#ifdef WASABI_COMPILE_SKIN
	void onSetXuiParam(const wchar_t *param, const wchar_t *value);
#endif

	const wchar_t *getFilename() { return filename; }
	void setFilename(const wchar_t *file) { filename = file; }
	static void navigateUrl(const wchar_t *url);
	static void navigateUrlBrowser(const wchar_t *url);

	// called by vcpu
	static scriptVar vcpu_getVersion( SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar vcpu_onScriptLoaded( SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar vcpu_onScriptUnloading( SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar vcpu_onQuit( SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
#ifdef WASABI_COMPILE_SKIN
	static scriptVar vcpu_onSetXuiParam( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar param, scriptVar value);
	static scriptVar vcpu_getSkinName( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_switchSkin( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar skinname);
	static scriptVar vcpu_isLoadingSkin( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_lockUI( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_unlockUI( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
#endif
	static scriptVar vcpu_isObjectValid( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar o);
	static scriptVar vcpu_getTimeOfDay( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_navigateUrl( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar url);
	static scriptVar vcpu_navigateUrlBrowser( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar url);
	static scriptVar vcpu_isKeyDown( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar vk_code);
	static scriptVar vcpu_setClipboard( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar text);
	static scriptVar vcpu_chr( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar n);
	static scriptVar vcpu_onAccelerator( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar action, scriptVar section, scriptVar key);
	static scriptVar vcpu_triggerAction( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar guiobj, scriptVar actionstr, scriptVar paramstr);
	static scriptVar vcpu_getParam( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
#ifdef WASABI_COMPILE_SKIN
	static scriptVar vcpu_getScriptGroup( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_newGroup( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar groupname);
#endif
	static scriptVar vcpu_getMousePosX( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getMousePosY( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_isMinimized( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_isDesktopAlphaAvailable( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_isTransparencyAvailable( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_minimizeApplication( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_restoreApplication( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_activateApplication( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_integerToString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar i);
	static scriptVar vcpu_stringToInteger( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar s);
	static scriptVar vcpu_floatToString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar f, scriptVar digits);
	static scriptVar vcpu_stringToFloat( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar s);
	static scriptVar vcpu_integerToTime( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i);
	static scriptVar vcpu_integerToLongTime( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i);
	static scriptVar vcpu_dateToTime( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i);
	static scriptVar vcpu_dateToLongTime( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i);
	static scriptVar vcpu_formatDate( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i);
	static scriptVar vcpu_formatLongDate( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i);
	static scriptVar vcpu_getDateYear( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i);
	static scriptVar vcpu_getDateMonth( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i);
	static scriptVar vcpu_getDateDay( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i);
	static scriptVar vcpu_getDateDow( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i);
	static scriptVar vcpu_getDateDoy( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i);
	static scriptVar vcpu_getDateHour( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i);
	static scriptVar vcpu_getDateMin( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i);
	static scriptVar vcpu_getDateSec( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i);
	static scriptVar vcpu_getDateDst( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i);
	static scriptVar vcpu_getDate( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_strmid( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar str, scriptVar start, scriptVar len);
	static scriptVar vcpu_strleft( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar str, scriptVar len);
	static scriptVar vcpu_strright( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str, scriptVar _len);
	static scriptVar vcpu_strsearch( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str, scriptVar substr);
	static scriptVar vcpu_strlen( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar str);
	static scriptVar vcpu_strupper( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str);
	static scriptVar vcpu_strlower( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str);
	static scriptVar vcpu_urlencode( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str);
	static scriptVar vcpu_urldecode( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str);
	static scriptVar vcpu_removepath( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str);
	static scriptVar vcpu_getpath( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str);
	static scriptVar vcpu_getextension( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str);
	static scriptVar vcpu_gettoken( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar tok, scriptVar sep, scriptVar num);
	static scriptVar vcpu_sin( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar d);
	static scriptVar vcpu_cos( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar d);
	static scriptVar vcpu_tan( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar d);
	static scriptVar vcpu_asin( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar d);
	static scriptVar vcpu_acos( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar d);
	static scriptVar vcpu_atan( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar d);
	static scriptVar vcpu_atan2( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar y, scriptVar x);
	static scriptVar vcpu_pow( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y);
	static scriptVar vcpu_sqr( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _d);
	static scriptVar vcpu_sqrt( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar d);
	static scriptVar vcpu_random( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar m);
	static scriptVar vcpu_integer( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar v);
	static scriptVar vcpu_frac( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar v);
	static scriptVar vcpu_log( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar v);
	static scriptVar vcpu_log10( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar v);
	static scriptVar vcpu_getMonitorLeft( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getMonitorTop( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getMonitorWidth( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getMonitorHeight( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getMonitorLeftFP( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y);
	static scriptVar vcpu_getMonitorTopFP( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y);
	static scriptVar vcpu_getMonitorLeftGO( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar obj);
	static scriptVar vcpu_getMonitorTopGO( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar obj);
	static scriptVar vcpu_getMonitorWidthFP( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y);
	static scriptVar vcpu_getMonitorHeightFP( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y);
	static scriptVar vcpu_getMonitorWidthGO( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar obj);
	static scriptVar vcpu_getMonitorHeightGO( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar obj);
	static scriptVar vcpu_onViewPortChanged( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar width, scriptVar height);
	static scriptVar vcpu_getViewportWidth( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getViewportHeight( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getViewportWidthGO( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar obj);
	static scriptVar vcpu_getViewportHeightGO( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar obj);
	static scriptVar vcpu_getViewportLeft( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getViewportTop( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getViewportLeftGO( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar obj);
	static scriptVar vcpu_getViewportTopGO( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar obj);
	static scriptVar vcpu_getViewportWidthFP( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y);
	static scriptVar vcpu_getViewportHeightFP( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y);
	static scriptVar vcpu_getViewportLeftFP( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y);
	static scriptVar vcpu_getViewportTopFP( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y);
	static scriptVar vcpu_getTickCount( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_onKeyDown( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v);
	static scriptVar vcpu_onKeyUp( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v);
	static scriptVar vcpu_debugString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar str, scriptVar severity);
	static scriptVar vcpu_messageBox( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar msg, scriptVar title, scriptVar flags, scriptVar nam);
	static scriptVar vcpu_getAtom( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar atomname);
	static scriptVar vcpu_setAtom( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar atomname, scriptVar obj);
#ifdef WASABI_COMPILE_MAKIDEBUG
	static scriptVar vcpu_invokeDebugger( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
#endif
#if defined (WASABI_COMPILE_WNDMGR) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)
	static scriptVar vcpu_onCreateLayout( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar l);
	static scriptVar vcpu_onShowLayout( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar l);
	static scriptVar vcpu_onHideLayout( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar l);
	static scriptVar vcpu_getContainer( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar contname);
	static scriptVar vcpu_newDynamicContainer( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar contname);
	static scriptVar vcpu_newGroupAsLayout( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar groupname);
	static scriptVar vcpu_getNumContainers( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_enumContainer( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar n);
	static scriptVar vcpu_onLookForComponent( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar guid);
	static scriptVar vcpu_onGetCancelComponent( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar guid, scriptVar i);
	static scriptVar vcpu_getCurAppLeft( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getCurAppTop( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getCurAppWidth( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getCurAppHeight( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_isAppActive( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_showWindow( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar wndguidorgroup, scriptVar prefcontainer, scriptVar transient);
	static scriptVar vcpu_hideWindow( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar container);
	static scriptVar vcpu_hideNamedWindow( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar wndguidorgroup);
	static scriptVar vcpu_isNamedWindowVisible( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar wndguidorgroup);
#endif
#if defined(WASABI_COMPILE_MEDIACORE) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)
	static scriptVar vcpu_getStatus( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_onStop( SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar vcpu_onPlay( SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar vcpu_onPause( SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar vcpu_onResume( SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar vcpu_onTitleChange( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar title);
	static scriptVar vcpu_onTitle2Change( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar title);
	static scriptVar vcpu_onUrlChange( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar title);
	static scriptVar vcpu_onInfoChange( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar info);
	static scriptVar vcpu_onStatusMsg( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar info);
	static scriptVar vcpu_onEqBandChanged( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar band, scriptVar val);
	static scriptVar vcpu_onEqFreqChanged( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar val);
	static scriptVar vcpu_onEqPreAmpChanged( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar val);
	static scriptVar vcpu_onEqChanged( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar val);
	static scriptVar vcpu_onVolumeChanged( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar val);
	static scriptVar vcpu_onSeeked( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar val);
	static scriptVar vcpu_getPlayItemDisplayTitle( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getPlayItemString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getPlayItemLength( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getPlayItemMetadataString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar name);
	static scriptVar vcpu_getMetadataString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar filename, scriptVar name);
	static scriptVar vcpu_getExtFamily( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar ext);
	static scriptVar vcpu_getDecoderName( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar ext);
	static scriptVar vcpu_playFile( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar file);
	static scriptVar vcpu_enqueueFile( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar file);
	static scriptVar vcpu_clearPlaylist( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);	
	static scriptVar vcpu_getLeftVuMeter( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getRightVuMeter( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getVisBand( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar chan, scriptVar band);
	static scriptVar vcpu_getVolume( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_setVolume( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar v);
	static scriptVar vcpu_play( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_stop( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_pause( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_next( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_previous( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_eject( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_seekTo( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar to);
	static scriptVar vcpu_setEqBand( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar band, scriptVar val);
	static scriptVar vcpu_setEqPreAmp( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar v);
	static scriptVar vcpu_setEq( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar v);
	static scriptVar vcpu_getPosition( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getEqBand( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar band);
	static scriptVar vcpu_getEqPreAmp( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getEq( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_hasVideoSupport( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_isVideo( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_isVideoFullscreen( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_setVideoFullscreen( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar val);
	static scriptVar vcpu_getIdealVideoWidth( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getIdealVideoHeight( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getPlaylistLength( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getPlaylistIndex( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_onShowNotification( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getSongInfoText( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getSongInfoTextTranslated( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
#endif
#if defined(WASABI_COMPILE_COMPONENTS) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)
	static scriptVar vcpu_getWac( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar guid);
#endif
#if defined (WASABI_COMPILE_CONFIG) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)
	static scriptVar vcpu_setPrivateString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar section, scriptVar item, scriptVar str);
	static scriptVar vcpu_setPrivateInt( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar section, scriptVar item, scriptVar val);
	static scriptVar vcpu_getPrivateString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar section, scriptVar item, scriptVar defstr);
	static scriptVar vcpu_getPrivateInt( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar section, scriptVar item, scriptVar defval);
	static scriptVar vcpu_setPublicString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar item, scriptVar str);
	static scriptVar vcpu_setPublicInt( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar item, scriptVar val);
	static scriptVar vcpu_getPublicString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar item, scriptVar defstr);
	static scriptVar vcpu_getPublicInt( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar item, scriptVar defval);
#endif
#if defined (WA3COMPATIBILITY) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)
	static scriptVar vcpu_ddeSend( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar ddetarget, scriptVar cmd, scriptVar minInterval);
	static scriptVar vcpu_popMb( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getMainMB( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_setMenuTransparency( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar a);
	static scriptVar vcpu_windowMenu( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_systemMenu( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
#endif
	static scriptVar vcpu_selectFile( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar extlist, scriptVar id, scriptVar prevfn);
	static scriptVar vcpu_selectFolder( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar title, scriptVar info, scriptVar defpath);

	static scriptVar vcpu_getRating( SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_setRating ( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar v);

#ifdef WASABI_COMPILE_MEDIACORE
	// core callbacks
	virtual int corecb_onVolumeChange(int newvol);
	virtual int corecb_onEQStatusChange(int newval);
	virtual int corecb_onEQPreampChange(int newval);
	virtual int corecb_onEQBandChange(int band, int newval);
	virtual int corecb_onEQFreqChange(int newval);
	virtual int corecb_onStarted();
	virtual int corecb_onStopped();
	virtual int corecb_onPaused();
	virtual int corecb_onUnpaused();
	virtual int corecb_onSeeked(int newpos);
	virtual int corecb_onTitleChange(const wchar_t *title);
	virtual int corecb_onTitle2Change(const wchar_t *title);
	virtual int corecb_onUrlChange(const wchar_t *url);
	virtual int corecb_onInfoChange(const wchar_t *info);
	virtual int corecb_onStatusMsg(const wchar_t *info);
#endif

	virtual void browsercb_onOpenURL(wchar_t *url, bool *override);
	static scriptVar vcpu_onOpenURL( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url);

	static scriptVar vcpu_downloadURL(SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar url,scriptVar filename,scriptVar title);
	static scriptVar vcpu_downloadMedia(SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar url, scriptVar filepath, scriptVar storeInMl, scriptVar notifyDownloadsList);
	static scriptVar vcpu_onDownloadFinished(SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar url,scriptVar success, scriptVar filename);
	static scriptVar vcpu_getDownloadPath(SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_setDownloadPath(SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar new_path);
	static scriptVar vcpu_getAlbumArt(SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar filename);
	static scriptVar vcpu_isWinampPro(SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_enumEmbedGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar index);
	static scriptVar vcpu_getWinampVersion(SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getBuildNumber(SCRIPT_FUNCTION_PARAMS, ScriptObject *object);
	static scriptVar vcpu_getFileSize(SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar file);
	static scriptVar vcpu_translate(SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar str);
	static scriptVar vcpu_getString(SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar table, scriptVar id);
	static scriptVar vcpu_getLanguageId(SCRIPT_FUNCTION_PARAMS, ScriptObject *object);

  private:
	int scriptVCPUId;
	StringW param;
	static wchar_t staticStr[4096];
	static wchar_t translateStr[4096];
	TList < int > typeslist;
	int isoldformat;
	Group * parentGroup;
	int skinpartid;
	static PtrList < ScriptObject > scriptobjects;
	PtrList < ScriptObject > instantiated;
	int loaded;
	int started_up;
	StringW filename;

  public:
};

#endif