#include <precomp.h>

#include <bfc/wasabi_std.h>
#include <bfc/wasabi_std_wnd.h>
#include <api/script/script.h>
#include <api/script/scriptmgr.h>
#include <api/script/scriptobj.h>
#include <api/script/scriptguid.h>
#include <api/script/objects/systemobj.h>
#include <api/service/svcs/svc_action.h>
#include <api/util/selectfile.h>
#include <api/script/vcpu.h> // for getAtom, CUT

#ifdef WASABI_COMPILE_WNDMGR
#include <api/wnd/wndclass/wndholder.h>
#include <api/wndmgr/skinembed.h>
#endif

#include <api/script/objects/timer.h>
#include <time.h>
#include <api.h>

#ifdef WASABI_COMPILE_SKIN
#include <api/skin/skinparse.h>
#include <api/skin/skin.h>
#endif
#include <math.h>
#include <shlobj.h>

#include <bfc/string/url.h>
#include <bfc/parse/pathparse.h>
#include <bfc/tlist.h>
#ifdef WASABI_COMPILE_WNDMGR
#include <api/util/varmgr.h>
#endif

#include <bfc/nsguid.h>
#include <api/script/objecttable.h>
#include <api/skin/groupmgr.h>

#ifdef WA3COMPATIBILITY
#include <api/skin/widgets/mb/xuibrowser.h>
#include <api/skin/widgets/mb/mainminibrowser.h>
#include <api/util/dde.h>
#include <api/wac/main.h>//CUT!!
#endif

#ifdef WASABI_COMPILE_MEDIACORE
#include <api/service/svcs/svc_player.h>
#include <api/core/buttons.h>
#include <api/core/api_core.h>
#include <api/core/corehandle.h>	// safe to include even if core isn't there
#endif

#ifdef WASABI_COMPILE_CONFIG
#include <api/config/items/attrbool.h>
#endif

#include <api/locales/xlatstr.h>
#include "../Agave/Language/api_language.h"
#include <api/syscb/callbacks/consolecb.h>
#include "../nu/AutoChar.h"
#include "../nu/AutoUrl.h"
#include "../nu/AutoWide.h"
#ifdef _WIN32
#include <shlwapi.h>
extern HINSTANCE hInstance;
#endif

#ifdef GEN_FF
#include "../../../../Plugins/General/gen_ff/wa2frontend.h"
#endif

SystemScriptObjectController _systemController;
SystemScriptObjectController *systemController = &_systemController;

#define MAKI_RUNTIME_VERSION 2

// -- Functions table -------------------------------------
function_descriptor_struct SystemScriptObjectController::exportedFunction[] =
{
	{L"getRuntimeVersion",         0, (void*)SystemObject::vcpu_getVersion},
	{L"onScriptLoaded",            0, (void*)SystemObject::vcpu_onScriptLoaded},
	{L"onScriptUnloading",         0, (void*)SystemObject::vcpu_onScriptUnloading},
	{L"onQuit",                    0, (void*)SystemObject::vcpu_onQuit},
	{L"onKeyDown",                 1, (void*)SystemObject::vcpu_onKeyDown},
	{L"onKeyUp",                   1, (void*)SystemObject::vcpu_onKeyUp},
	{L"onAccelerator",             3, (void*)SystemObject::vcpu_onAccelerator},
	{L"getMousePosX",              0, (void*)SystemObject::vcpu_getMousePosX},
	{L"getMousePosY",              0, (void*)SystemObject::vcpu_getMousePosY},
	{L"isMinimized",               0, (void*)SystemObject::vcpu_isMinimized},
	{L"restoreApplication",        0, (void*)SystemObject::vcpu_restoreApplication},
	{L"activateApplication",       0, (void*)SystemObject::vcpu_activateApplication},
	{L"minimizeApplication",       0, (void*)SystemObject::vcpu_minimizeApplication},
	{L"isDesktopAlphaAvailable",   0, (void*)SystemObject::vcpu_isDesktopAlphaAvailable},
	{L"isTransparencyAvailable",   0, (void*)SystemObject::vcpu_isTransparencyAvailable},
//-----
	{L"integerToString",           1, (void*)SystemObject::vcpu_integerToString},
	{L"stringToInteger",           1, (void*)SystemObject::vcpu_stringToInteger},
	{L"floatToString",             2, (void*)SystemObject::vcpu_floatToString},
	{L"stringToFloat",             1, (void*)SystemObject::vcpu_stringToFloat},
	{L"integerToTime",             1, (void*)SystemObject::vcpu_integerToTime},
	{L"integerToLongTime",         1, (void*)SystemObject::vcpu_integerToLongTime},
	{L"dateToTime",                1, (void*)SystemObject::vcpu_dateToTime},
	{L"dateToLongTime",            1, (void*)SystemObject::vcpu_dateToLongTime},
	{L"formatDate",                1, (void*)SystemObject::vcpu_formatDate},
	{L"formatLongDate",            1, (void*)SystemObject::vcpu_formatLongDate},
	{L"getDateYear",               1, (void*)SystemObject::vcpu_getDateYear},
	{L"getDateMonth",              1, (void*)SystemObject::vcpu_getDateMonth},
	{L"getDateDay",                1, (void*)SystemObject::vcpu_getDateDay},
	{L"getDateDow",                1, (void*)SystemObject::vcpu_getDateDow},
	{L"getDateDoy",                1, (void*)SystemObject::vcpu_getDateDoy},
	{L"getDateHour",               1, (void*)SystemObject::vcpu_getDateHour},
	{L"getDateMin",                1, (void*)SystemObject::vcpu_getDateMin},
	{L"getDateSec",                1, (void*)SystemObject::vcpu_getDateSec},
	{L"getDateDst",                1, (void*)SystemObject::vcpu_getDateDst},
	{L"getDate",                   0, (void*)SystemObject::vcpu_getDate},
	{L"StrMid",                    3, (void*)SystemObject::vcpu_strmid},
	{L"StrLeft",                   2, (void*)SystemObject::vcpu_strleft},
	{L"StrRight",                  2, (void*)SystemObject::vcpu_strright},
	{L"StrSearch",                 2, (void*)SystemObject::vcpu_strsearch},
	{L"StrLen",                    1, (void*)SystemObject::vcpu_strlen},
	{L"StrUpper",                  1, (void*)SystemObject::vcpu_strupper},
	{L"StrLower",                  1, (void*)SystemObject::vcpu_strlower},
	{L"UrlEncode",                 1, (void*)SystemObject::vcpu_urlencode},
	{L"UrlDecode",                 1, (void*)SystemObject::vcpu_urldecode},
	{L"RemovePath",                1, (void*)SystemObject::vcpu_removepath},
	{L"GetPath",                   1, (void*)SystemObject::vcpu_getpath},
	{L"GetExtension",              1, (void*)SystemObject::vcpu_getextension},
	{L"getToken",                  3, (void*)SystemObject::vcpu_gettoken},
//-----
	{L"sin",                       1, (void*)SystemObject::vcpu_sin},
	{L"cos",                       1, (void*)SystemObject::vcpu_cos},
	{L"tan",                       1, (void*)SystemObject::vcpu_tan},
	{L"asin",                      1, (void*)SystemObject::vcpu_asin},
	{L"acos",                      1, (void*)SystemObject::vcpu_acos},
	{L"atan",                      1, (void*)SystemObject::vcpu_atan},
	{L"atan2",                     2, (void*)SystemObject::vcpu_atan2},
	{L"pow",                       2, (void*)SystemObject::vcpu_pow},
	{L"sqr",                       1, (void*)SystemObject::vcpu_sqr},
	{L"sqrt",                      1, (void*)SystemObject::vcpu_sqrt},
	{L"random",                    1, (void*)SystemObject::vcpu_random},
	{L"integer",                   1, (void*)SystemObject::vcpu_integer},
	{L"frac",                      1, (void*)SystemObject::vcpu_frac},
	{L"ln",                        1, (void*)SystemObject::vcpu_log},
	{L"log10",                     1, (void*)SystemObject::vcpu_log10},
//-----
	{L"getParam",                  0, (void*)SystemObject::vcpu_getParam},
	{L"getViewportWidth",          0, (void*)SystemObject::vcpu_getViewportWidth},
	{L"getViewportHeight",         0, (void*)SystemObject::vcpu_getViewportHeight},
	{L"getViewportLeft",           0, (void*)SystemObject::vcpu_getViewportLeft},
	{L"getViewportTop",            0, (void*)SystemObject::vcpu_getViewportTop},
	{L"getViewportWidthFromPoint", 2, (void*)SystemObject::vcpu_getViewportWidthFP},
	{L"getViewportHeightFromPoint",2, (void*)SystemObject::vcpu_getViewportHeightFP},
	{L"getViewportLeftFromPoint",  2, (void*)SystemObject::vcpu_getViewportLeftFP},
	{L"getViewportTopFromPoint",   2, (void*)SystemObject::vcpu_getViewportTopFP},
	{L"getViewportWidthFromGuiObject",    1, (void*)SystemObject::vcpu_getViewportWidthGO},
	{L"getViewportHeightFromGuiObject",   1, (void*)SystemObject::vcpu_getViewportHeightGO},
	{L"getViewportLeftFromGuiObject",     1, (void*)SystemObject::vcpu_getViewportLeftGO},
	{L"getViewportTopFromGuiObject",      1, (void*)SystemObject::vcpu_getViewportTopGO},
	{L"onViewPortChanged",         2, (void*)SystemObject::vcpu_onViewPortChanged},
	{L"debugString",               2, (void*)SystemObject::vcpu_debugString},
	{L"isObjectValid",             1, (void*)SystemObject::vcpu_isObjectValid},
	{L"getTimeOfDay",              0, (void*)SystemObject::vcpu_getTimeOfDay},
	{L"navigateUrl",               1, (void*)SystemObject::vcpu_navigateUrl},
	{L"navigateUrlBrowser",        1, (void*)SystemObject::vcpu_navigateUrlBrowser},
	{L"isKeyDown",                 1, (void*)SystemObject::vcpu_isKeyDown},
	{L"setClipboardText",          1, (void*)SystemObject::vcpu_setClipboard},
	{L"Chr",                       1, (void*)SystemObject::vcpu_chr},
	{L"triggerAction",             3, (void*)SystemObject::vcpu_triggerAction},
	{L"messageBox",                4, (void*)SystemObject::vcpu_messageBox},
	{L"setAtom",                   2, (void*)SystemObject::vcpu_setAtom},
	{L"getAtom",                   1, (void*)SystemObject::vcpu_getAtom},
#ifdef WASABI_COMPILE_MAKIDEBUG
	{L"invokeDebugger",            0, (void*)SystemObject::vcpu_invokeDebugger},
#endif
#ifdef WASABI_COMPILE_SKIN
	{L"newGroup",                  1, (void*)SystemObject::vcpu_newGroup},
	{L"onSetXuiParam",             2, (void*)SystemObject::vcpu_onSetXuiParam},
	{L"getScriptGroup",            0, (void*)SystemObject::vcpu_getScriptGroup},
	{L"getSkinName",               0, (void*)SystemObject::vcpu_getSkinName},
	{L"newGroupAsLayout",          1, (void*)SystemObject::vcpu_newGroupAsLayout},
	{L"getNumContainers",          0, (void*)SystemObject::vcpu_getNumContainers},
	{L"enumContainer",             1, (void*)SystemObject::vcpu_enumContainer},
	{L"onCreateLayout",            1, (void*)SystemObject::vcpu_onCreateLayout},
	{L"onShowLayout",              1, (void*)SystemObject::vcpu_onShowLayout},
	{L"onHideLayout",              1, (void*)SystemObject::vcpu_onHideLayout},
	{L"switchSkin",                1, (void*)SystemObject::vcpu_switchSkin},
	{L"isLoadingSkin",             0, (void*)SystemObject::vcpu_isLoadingSkin},
	{L"lockUI",                    0, (void*)SystemObject::vcpu_lockUI},
	{L"unlockUI",                  0, (void*)SystemObject::vcpu_unlockUI},
#endif
#if defined(WASABI_COMPILE_WNDMGR) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)
	{
	  L"getContainer",              1, (void*)SystemObject::vcpu_getContainer
	},
	{L"newDynamicContainer",       1, (void*)SystemObject::vcpu_newDynamicContainer},
	{L"onGetCancelComponent",      2, (void*)SystemObject::vcpu_onGetCancelComponent},
	{L"onLookForComponent",        1, (void*)SystemObject::vcpu_onLookForComponent},
	{L"isAppActive",               0, (void*)SystemObject::vcpu_isAppActive},
	{L"showWindow",                3, (void*)SystemObject::vcpu_showWindow},
	{L"hideWindow",                1, (void*)SystemObject::vcpu_hideWindow},
	{L"hideNamedWindow",           1, (void*)SystemObject::vcpu_hideNamedWindow},
	{L"isNamedWindowVisible",      1, (void*)SystemObject::vcpu_isNamedWindowVisible},
	{L"getCurAppLeft",             0, (void*)SystemObject::vcpu_getCurAppLeft},
	{L"getCurAppTop",              0, (void*)SystemObject::vcpu_getCurAppTop},
	{L"getCurAppWidth",            0, (void*)SystemObject::vcpu_getCurAppWidth},
	{L"getCurAppHeight",           0, (void*)SystemObject::vcpu_getCurAppHeight},
#endif
#if defined (WASABI_COMPILE_CONFIG) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)
	{
	  L"setPrivateString",         3, (void*)SystemObject::vcpu_setPrivateString
	},
	{L"setPrivateInt",             3, (void*)SystemObject::vcpu_setPrivateInt},
	{L"getPrivateString",          3, (void*)SystemObject::vcpu_getPrivateString},
	{L"getPrivateInt",             3, (void*)SystemObject::vcpu_getPrivateInt},
	{L"setPublicString",           2, (void*)SystemObject::vcpu_setPublicString},
	{L"setPublicInt",              2, (void*)SystemObject::vcpu_setPublicInt},
	{L"getPublicString",           2, (void*)SystemObject::vcpu_getPublicString},
	{L"getPublicInt",              2, (void*)SystemObject::vcpu_getPublicInt},
#endif
#if defined (WASABI_COMPILE_MEDIACORE) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)
	{
	  L"getPlayItemString",         0, (void*)SystemObject::vcpu_getPlayItemString
	},
	{L"getPlayItemLength",         0, (void*)SystemObject::vcpu_getPlayItemLength},
	{L"getPlayItemMetadataString", 1, (void*)SystemObject::vcpu_getPlayItemMetadataString},
	{L"getMetadataString", 2, (void*)SystemObject::vcpu_getMetadataString},
	{L"getPlayItemDisplayTitle",   0, (void*)SystemObject::vcpu_getPlayItemDisplayTitle},
	{L"getExtFamily",              1, (void*)SystemObject::vcpu_getExtFamily},
	{L"getDecoderName",            1, (void*)SystemObject::vcpu_getDecoderName},
	{L"playFile",                  1, (void*)SystemObject::vcpu_playFile},
	{L"enqueueFile",               1, (void*)SystemObject::vcpu_enqueueFile},
	{L"clearPlaylist",             0, (void*)SystemObject::vcpu_clearPlaylist},
	{L"onStop",                    0, (void*)SystemObject::vcpu_onStop},
	{L"onPlay",                    0, (void*)SystemObject::vcpu_onPlay},
	{L"onPause",                   0, (void*)SystemObject::vcpu_onPause},
	{L"onResume",                  0, (void*)SystemObject::vcpu_onResume},
	{L"onTitleChange",             1, (void*)SystemObject::vcpu_onTitleChange},
	{L"onTitle2Change",            1, (void*)SystemObject::vcpu_onTitle2Change},
	{L"onUrlChange",			   1, (void*)SystemObject::vcpu_onUrlChange},
	{L"onInfoChange",              1, (void*)SystemObject::vcpu_onInfoChange},
	{L"onStatusMsg",               1, (void*)SystemObject::vcpu_onStatusMsg},
	{L"getLeftVuMeter",            0, (void*)SystemObject::vcpu_getLeftVuMeter},
	{L"getRightVuMeter",           0, (void*)SystemObject::vcpu_getRightVuMeter},
	{L"getVisBand",                2, (void*)SystemObject::vcpu_getVisBand},
	{L"getVolume",                 0, (void*)SystemObject::vcpu_getVolume},
	{L"setVolume",                 1, (void*)SystemObject::vcpu_setVolume},
	{L"play",                      0, (void*)SystemObject::vcpu_play},
	{L"stop",                      0, (void*)SystemObject::vcpu_stop},
	{L"pause",                     0, (void*)SystemObject::vcpu_pause},
	{L"next",                      0, (void*)SystemObject::vcpu_next},
	{L"previous",                  0, (void*)SystemObject::vcpu_previous},
	{L"eject",                     0, (void*)SystemObject::vcpu_eject},
	{L"seekTo",                    1, (void*)SystemObject::vcpu_seekTo},
	{L"getPosition",               0, (void*)SystemObject::vcpu_getPosition},
	{L"setEqBand",                 2, (void*)SystemObject::vcpu_setEqBand},
	{L"setEqPreAmp",               1, (void*)SystemObject::vcpu_setEqPreAmp},
	{L"setEq",                     1, (void*)SystemObject::vcpu_setEq},
	{L"getEqBand",                 1, (void*)SystemObject::vcpu_getEqBand},
	{L"getEqPreAmp",               0, (void*)SystemObject::vcpu_getEqPreAmp},
	{L"getEq",                     0, (void*)SystemObject::vcpu_getEq},
	{L"onEqBandChanged",           2, (void*)SystemObject::vcpu_onEqBandChanged},
	{L"onEqFreqChanged",           1, (void*)SystemObject::vcpu_onEqFreqChanged},
	{L"onEqPreAmpChanged",         1, (void*)SystemObject::vcpu_onEqPreAmpChanged},
	{L"onEqChanged",               1, (void*)SystemObject::vcpu_onEqChanged},
	{L"onVolumeChanged",           1, (void*)SystemObject::vcpu_onVolumeChanged},
	{L"onSeek",                    1, (void*)SystemObject::vcpu_onSeeked},
	{L"getStatus",                 0, (void*)SystemObject::vcpu_getStatus},
	{L"getStatus",                 0, (void*)SystemObject::vcpu_getStatus},
	{L"getSongInfoText",           0, (void*)SystemObject::vcpu_getSongInfoText},
	{L"getSongInfoTextTranslated", 0, (void*)SystemObject::vcpu_getSongInfoTextTranslated},
	{L"hasVideoSupport",           0, (void*)SystemObject::vcpu_hasVideoSupport},
	{L"isVideo",                   0, (void*)SystemObject::vcpu_isVideo},
	{L"isVideoFullscreen",         0, (void*)SystemObject::vcpu_isVideoFullscreen},
    {L"setVideoFullscreen",        1, (void*)SystemObject::vcpu_setVideoFullscreen},
	{L"getIdealVideoWidth",        0, (void*)SystemObject::vcpu_getIdealVideoWidth},
	{L"getIdealVideoHeight",       0, (void*)SystemObject::vcpu_getIdealVideoHeight},
	{L"getPlaylistIndex",          0, (void*)SystemObject::vcpu_getPlaylistIndex},
	{L"onShowNotification",        0, (void*)SystemObject::vcpu_onShowNotification},
	{L"getPlaylistLength",         0, (void*)SystemObject::vcpu_getPlaylistLength},
	{L"getCurrentTrackRating",     0, (void *)SystemObject::vcpu_getRating},
	{L"setCurrentTrackRating",     1, (void *)SystemObject::vcpu_setRating},
#endif
#if defined(WASABI_COMPILE_COMPONENTS) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)
	{
	  L"getWac",                    1, (void*)SystemObject::vcpu_getWac
	},
#endif
#if defined(WA3COMPATIBILITY) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)
	{
	  L"ddesend",                   3, (void*)SystemObject::vcpu_ddeSend
	},
	{L"setMenuTransparency",      1, (void*)SystemObject::vcpu_setMenuTransparency},
	{L"popMainBrowser",           0, (void*)SystemObject::vcpu_popMb},
	{L"getMainBrowser",           0, (void*)SystemObject::vcpu_getMainMB},
	{L"windowMenu",               0, (void*)SystemObject::vcpu_windowMenu},
	{L"systemMenu",               0, (void*)SystemObject::vcpu_systemMenu},
#endif
#if defined (WA3COMPATIBILITY) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)
	{
	  L"selectFile",               3, (void*)SystemObject::vcpu_selectFile
	},
#endif
	{L"selectFolder",               3, (void*)SystemObject::vcpu_selectFolder},
	{L"onOpenURL",             1, (void*)SystemObject::vcpu_onOpenURL},
	{L"getMonitorLeft", 0, (void*)SystemObject::vcpu_getMonitorLeftGO},
	{L"getMonitorTop", 0, (void*)SystemObject::vcpu_getMonitorTopGO},
	{L"getMonitorLeftFromPoint", 2, (void*)SystemObject::vcpu_getMonitorLeftFP},
	{L"getMonitorTopFromPoint", 2, (void*)SystemObject::vcpu_getMonitorTopFP},
	{L"getMonitorLeftFromGuiObject", 1, (void*)SystemObject::vcpu_getMonitorLeftGO},
	{L"getMonitorTopFromGuiObject", 1, (void*)SystemObject::vcpu_getMonitorTopGO},
	{L"getMonitorWidth",          0, (void*)SystemObject::vcpu_getMonitorWidth},
	{L"getMonitorHeight",         0, (void*)SystemObject::vcpu_getMonitorHeight},
	{L"getMonitorWidthFromPoint", 2, (void*)SystemObject::vcpu_getMonitorWidthFP},
	{L"getMonitorHeightFromPoint",2, (void*)SystemObject::vcpu_getMonitorHeightFP},
	{L"getMonitorWidthFromGuiObject", 1, (void*)SystemObject::vcpu_getMonitorWidthGO},
	{L"getMonitorHeightFromGuiObject", 1, (void*)SystemObject::vcpu_getMonitorHeightGO},
	{L"downloadURL", 3, (void*)SystemObject::vcpu_downloadURL},
	{L"downloadMedia", 4, (void*)SystemObject::vcpu_downloadMedia},
	{L"onDownloadFinished", 3, (void*)SystemObject::vcpu_onDownloadFinished},
	{L"getDownloadPath", 0, (void*)SystemObject::vcpu_getDownloadPath},
	{L"setDownloadPath", 1, (void*)SystemObject::vcpu_setDownloadPath},
	{L"getAlbumArt", 1, (void*)SystemObject::vcpu_getAlbumArt},
	{L"isProVersion", 0, (void*)SystemObject::vcpu_isWinampPro}, // ugh, i hate putting this here but ohh well
	{L"enumEmbedGUID", 1, (void*)SystemObject::vcpu_enumEmbedGUID}, // ugh, i hate putting this here but ohh well
	{L"getWinampVersion",0, (void*)SystemObject::vcpu_getWinampVersion},
	{L"getBuildNumber",0, (void*)SystemObject::vcpu_getBuildNumber},
	{L"getFileSize",1, (void*)SystemObject::vcpu_getFileSize},
	{L"getString", 2, (void*)SystemObject::vcpu_getString},
	{L"translate", 1, (void*)SystemObject::vcpu_translate},
	{L"getLanguageId", 0, (void*)SystemObject::vcpu_getLanguageId}
};
// --------------------------------------------------------


const wchar_t *SystemScriptObjectController::getClassName()
{
	return L"SystemObject";
}

const wchar_t *SystemScriptObjectController::getAncestorClassName()
{
	return L"Object";
}

ScriptObject *SystemScriptObjectController::instantiate()
{
	return NULL;
}

void SystemScriptObjectController::destroy(ScriptObject *o)
{
	ASSERTALWAYS("don't delete systemobject!");
}

void *SystemScriptObjectController::encapsulate(ScriptObject *o)
{
	return NULL;
}

void SystemScriptObjectController::deencapsulate(void *o)
{
}

int SystemScriptObjectController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

const function_descriptor_struct *SystemScriptObjectController::getExportedFunctions()
{
	return exportedFunction;
}

GUID SystemScriptObjectController::getClassGuid()
{
	return systemObjectGuid;
}

int SystemScriptObjectController::getReferenceable()
{
	return 0;
}

int SystemScriptObjectController::getInstantiable()
{
	return 0;
}

#ifndef _NOSTUDIO

SystemObject::SystemObject()
{
	getScriptObject()->vcpu_setInterface( systemObjectGuid, ( void * )static_cast<SystemObject *>( this ) );
	getScriptObject()->vcpu_setClassName( L"System" );
	getScriptObject()->vcpu_setController( systemController );

	loaded       = 0;
	started_up   = 0;
	scriptVCPUId = -1;
	isoldformat  = 0;
	parentGroup  = NULL;

#ifdef WASABI_COMPILE_SKIN
	skinpartid   = WASABI_API_PALETTE->getSkinPartIterator();
#else
	skinpartid   = -1;
#endif

	SOM::registerSystemObject( this );

#ifdef WASABI_COMPILE_MEDIACORE
	WASABI_API_MEDIACORE->core_addCallback( 0, this );
#endif

	WASABI_API_SYSCB->syscb_registerCallback( this );
}

SystemObject::~SystemObject()
{
	SOM::unregisterSystemObject( this );
#ifdef WASABI_COMPILE_MEDIACORE
	WASABI_API_MEDIACORE->core_delCallback( 0, this );
#endif
	WASABI_API_SYSCB->syscb_deregisterCallback( this );
}

void SystemObject::setScriptId( int id )
{
	scriptVCPUId = id;
}

int SystemObject::getScriptId()
{
	return scriptVCPUId;
}

void SystemObject::setParam( const wchar_t *p )
{
#ifdef WASABI_COMPILE_WNDMGR
	StringW *s = PublicVarManager::translate_nocontext( p );
	if ( s )
		param.swap( s );
	else
		param = p;
	delete s;
#else
	param = p;
#endif
}

void SystemObject::setParentGroup( Group *g )
{
	parentGroup = g;
}

Group *SystemObject::getParentGroup()
{
	return parentGroup;
}

const wchar_t *SystemObject::getParam()
{
	return param;
}

void SystemObject::onLoad()
{
	loaded = 1;
	vcpu_onScriptLoaded( SCRIPT_CALL, getScriptObject() );
	started_up = 1;
}

void SystemObject::onUnload()
{
	if ( loaded && started_up )
		vcpu_onScriptUnloading( SCRIPT_CALL, getScriptObject() );

	// that was the script's last chance to delete its stuff, now we need to garbageCollect whatever is left
	garbageCollect();
}

#ifdef WASABI_COMPILE_WNDMGR
WindowHolder *SystemObject::getSuitableWindowHolderByGuid( GUID g )
{
	wchar_t guidstr[ 256 ] = { 0 };
	nsGUID::toCharW( g, guidstr );
	scriptVar v = vcpu_onLookForComponent( SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING( guidstr ) );

	if ( v.type == SCRIPT_OBJECT )
	{
		ScriptObject *ret = GET_SCRIPT_OBJECT( v );
		if ( ret )
		{
			WindowHolder *co = static_cast<WindowHolder *>( ret->vcpu_getInterface( windowHolderGuid ) );
			return co;
		}
	}
	return NULL;
}

int SystemObject::onGetCancelComponent( GUID g, int i )
{
	wchar_t guidstr[ 256 ] = { 0 };
	nsGUID::toCharW( g, guidstr );
	scriptVar v = vcpu_onGetCancelComponent( SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING( guidstr ), MAKE_SCRIPT_BOOLEAN( i ) );

	if ( v.type != SCRIPT_VOID )
		return GET_SCRIPT_INT( v );

	return 0;
}

void SystemObject::onViewPortChanged( int width, int height )
{
	for ( int i = SOM::getNumSystemObjects() - 1; i >= 0; i-- )
		vcpu_onViewPortChanged( SCRIPT_CALL, SOM::getSystemObject( i )->getScriptObject(), MAKE_SCRIPT_INT( width ), MAKE_SCRIPT_INT( height ) );
}

int SystemObject::onShowNotification()
{
	WASABI_API_MAKI->vcpu_resetComplete();
#if defined(WASABI_COMPILE_MEDIACORE) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)
	for ( int i = SOM::getNumSystemObjects() - 1; i >= 0 && !WASABI_API_MAKI->vcpu_getComplete(); i-- )
	{
		scriptVar v = vcpu_onShowNotification( SCRIPT_CALL, SOM::getSystemObject( i )->getScriptObject() );
		if ( v.type != SCRIPT_VOID )
			return GET_SCRIPT_INT( v );
	}
#endif
	return 0;
}

void SystemObject::onCreateLayout( Layout *l )
{
	for ( int i = SOM::getNumSystemObjects() - 1; i >= 0; i-- )
		vcpu_onCreateLayout( SCRIPT_CALL, SOM::getSystemObject( i )->getScriptObject(), MAKE_SCRIPT_OBJECT( l->getGuiObject()->guiobject_getScriptObject() ) );
}

void SystemObject::onShowLayout( Layout *l )
{
	for ( int i = SOM::getNumSystemObjects() - 1; i >= 0; i-- )
		vcpu_onShowLayout( SCRIPT_CALL, SOM::getSystemObject( i )->getScriptObject(), MAKE_SCRIPT_OBJECT( l->getGuiObject()->guiobject_getScriptObject() ) );
}

void SystemObject::onHideLayout( Layout *l )
{
	for ( int i = SOM::getNumSystemObjects() - 1; i >= 0; i-- )
		vcpu_onHideLayout( SCRIPT_CALL, SOM::getSystemObject( i )->getScriptObject(), MAKE_SCRIPT_OBJECT( l->getGuiObject()->guiobject_getScriptObject() ) );
}

int SystemObject::getCurAppRect( RECT *r )
{
	ASSERT( r );
#ifdef WIN32
	HWND w = GetForegroundWindow();
	if ( !IsWindowVisible( w ) ) return 0;
	GetWindowRect( w, r );
#else
	DebugString( "portme  SystemObject::getCurAppRect\n" );
#endif
	return 1;
}

void SystemObject::onQuit()
{
	for ( int i = SOM::getNumSystemObjects() - 1; i >= 0; i-- )
		vcpu_onQuit( SCRIPT_CALL, SOM::getSystemObject( i )->getScriptObject() );
}

#endif // wndmgr

static bool FilterBrowserUrl( const wchar_t *url )
{
	const wchar_t filterNowPlaying[] = L"http://client.winamp.com/nowplaying";
	size_t urlLength, filterLength;

	if ( NULL == url )
		return false;

	urlLength = wcslen( url );
	filterLength = sizeof( filterNowPlaying ) / sizeof( filterNowPlaying[ 0 ] ) - 1;
	if ( urlLength >= filterLength &&
		0 == _wcsnicmp( url, filterNowPlaying, filterLength ) )
	{
		return true;
	}

	return false;
}

void SystemObject::browsercb_onOpenURL( wchar_t *url, bool *override )
{
	if ( !*override && false == FilterBrowserUrl( url ) ) // ignore if someone else already overrode
	{
		scriptVar v = vcpu_onOpenURL( SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING( url ) );
		if ( v.type == SCRIPT_INT )
		{
			if ( v.data.idata == 1 )
				*override = true;
		}
	}
}

scriptVar SystemObject::vcpu_onOpenURL( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	PROCESS_HOOKS1( o, systemController, url );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT1( o, url );
#else
	RETURN_SCRIPT_VOID;
#endif
}

void SystemObject::onKeyDown( const wchar_t *s )
{
	WASABI_API_MAKI->vcpu_resetComplete();
	for ( int i = SOM::getNumSystemObjects() - 1; i >= 0 && !WASABI_API_MAKI->vcpu_getComplete(); i-- )
		vcpu_onKeyDown( SCRIPT_CALL, SOM::getSystemObject( i )->getScriptObject(), MAKE_SCRIPT_STRING( s ) );
}

void SystemObject::onKeyUp( const wchar_t *s )
{
	WASABI_API_MAKI->vcpu_resetComplete();
	for ( int i = SOM::getNumSystemObjects() - 1; i >= 0 && !WASABI_API_MAKI->vcpu_getComplete(); i-- )
		vcpu_onKeyUp( SCRIPT_CALL, SOM::getSystemObject( i )->getScriptObject(), MAKE_SCRIPT_STRING( s ) );
}

int SystemObject::onAccelerator( const wchar_t *action, const wchar_t *section, const wchar_t *key )
{
	int r = 0;
	WASABI_API_MAKI->vcpu_resetComplete();
	for ( int i = SOM::getNumSystemObjects() - 1; i >= 0 && !WASABI_API_MAKI->vcpu_getComplete(); i-- )
	{
		scriptVar v = vcpu_onAccelerator( SCRIPT_CALL, SOM::getSystemObject( i )->getScriptObject(), MAKE_SCRIPT_STRING( action ), MAKE_SCRIPT_STRING( section ), MAKE_SCRIPT_STRING( key ) );
		if ( SOM::isNumeric( &v ) )
			r |= GET_SCRIPT_INT( v );
	}
	return r;
}

void SystemObject::onDownloadFinished( const wchar_t *url, boolean success, const wchar_t *filename )
{
	for ( int i = SOM::getNumSystemObjects() - 1; i >= 0; i-- )
		vcpu_onDownloadFinished( SCRIPT_CALL, SOM::getSystemObject( i )->getScriptObject(), MAKE_SCRIPT_STRING( url ), MAKE_SCRIPT_BOOLEAN( success ), MAKE_SCRIPT_STRING( filename ) );
}

TList < int > *SystemObject::getTypesList()
{
	return &typeslist;
}

void SystemObject::setIsOldFormat( int is )
{
	isoldformat = is;
}

int SystemObject::isOldFormat()
{
	return isoldformat;
}
#endif

#ifdef WASABI_COMPILE_WND
int SystemObject::isAppActive()
{
#ifdef WIN32
	HWND w = GetForegroundWindow();
	wchar_t classname[ 256 ] = L"";
	GetClassNameW( w, classname, 256 );
	return ( w == WASABI_API_WND->main_getRootWnd()->gethWnd() || !wcscmp( classname, BASEWNDCLASSNAME ) );
#else
	DebugString( "portme SystemObject::isAppActive\n" );
	return 1;
#endif
}
#endif

void SystemObject::setSkinPartId( int _skinpartid )
{
	skinpartid = _skinpartid;
}

int SystemObject::getSkinPartId()
{
	return skinpartid;
}

void SystemObject::addInstantiatedObject( ScriptObject *obj )
{
	ASSERT( !instantiated.haveItem( obj ) );
	instantiated.addItem( obj );
}

void SystemObject::removeInstantiatedObject( ScriptObject *obj )
{
	int n = instantiated.searchItem( obj );
	if ( n < 0 ) return; //ASSERT(n >= 0);
	instantiated.removeByPos( n );
}

void SystemObject::garbageCollect()
{
	foreach( instantiated )
		ObjectTable::destroy( instantiated.getfor() );
	endfor
		instantiated.removeAll();
}

int SystemObject::isObjectValid( ScriptObject *o )
{
	static ScriptObject *cached = NULL;
	static int cachedn = -1;
	if ( o == cached && scriptobjects.enumItem( cachedn ) == o ) return 1;
	int is = scriptobjects.searchItem( o );
	if ( is >= 0 )
	{
		cached = o;
		cachedn = is;
	}
	return ( is > -1 );
}

void SystemObject::addScriptObject( ScriptObject *o )
{
	ASSERT( !scriptobjects.haveItem( o ) );
	scriptobjects.addItem( o );
}

void SystemObject::removeScriptObject( ScriptObject *o )
{
	int pos = scriptobjects.searchItem( o );
	if ( pos < 0 ) return;
	scriptobjects.removeByPos( pos );
}

#ifdef WASABI_COMPILE_SKIN
void SystemObject::onSetXuiParam( const wchar_t *param, const wchar_t *value )
{
	vcpu_onSetXuiParam( SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING( param ), MAKE_SCRIPT_STRING( value ) );
}
#endif

void SystemObject::navigateUrl( const wchar_t *url )
{
#ifdef WA3COMPATIBILITY
	if ( MainMiniBrowser::getScriptObject() )
	{
		MainMiniBrowser::navigateUrl( url );
		MainMiniBrowser::popMb();
	}
	else   // no minibrowser container -> launch IE
	{
		Std::shellExec( url );
	}
#else
	Wasabi::Std::shellExec( url );
#endif
}

void SystemObject::navigateUrlBrowser( const wchar_t *url )
{
	wa2.openUrl( url );
}

scriptVar SystemObject::vcpu_getVersion( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0( o, systemController );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	return MAKE_SCRIPT_DOUBLE( MAKI_RUNTIME_VERSION );
}

scriptVar SystemObject::vcpu_onScriptLoaded( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0( o, systemController );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SystemObject *so = static_cast<SystemObject *>( o->vcpu_getInterface( systemObjectGuid ) );
	ASSERT( so != NULL );
	return WASABI_API_MAKI->maki_triggerEvent( o, DLF_ID, 0, so->scriptVCPUId );
}

scriptVar SystemObject::vcpu_onScriptUnloading( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0( o, systemController );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SystemObject *so = static_cast<SystemObject *>( o->vcpu_getInterface( systemObjectGuid ) );
	ASSERT( so != NULL );
	return WASABI_API_MAKI->maki_triggerEvent( o, DLF_ID, 0, so->scriptVCPUId );
}

scriptVar SystemObject::vcpu_onQuit( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0( o, systemController );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT0( o );
}

#ifdef WASABI_COMPILE_SKIN
scriptVar SystemObject::vcpu_onSetXuiParam( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar param, scriptVar value )
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS2( o, systemController, param, value );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT2( o, param, value );
}

scriptVar SystemObject::vcpu_getSkinName( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_STRING( WASABI_API_SKIN->getSkinName() );
}

scriptVar SystemObject::vcpu_switchSkin( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar skinname )
{
	SCRIPT_FUNCTION_INIT;
#ifdef SWITCH_SKIN
	SWITCH_SKIN( GET_SCRIPT_STRING( skinname ) );
#else
	WASABI_API_SKIN->skin_switchSkin( GET_SCRIPT_STRING( skinname ) );
#endif
	RETURN_SCRIPT_ZERO;
}

scriptVar SystemObject::vcpu_isLoadingSkin( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	int l = 0;
#ifdef IS_SKIN_STILL_LOADING
	IS_SKIN_STILL_LOADING( l );
#endif
	if ( !l ) return MAKE_SCRIPT_INT( Skin::isLoading() );
	return MAKE_SCRIPT_INT( l );
}

scriptVar SystemObject::vcpu_lockUI( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	WASABI_API_SKIN->skin_setLockUI( 1 );
	RETURN_SCRIPT_NULL;
}

scriptVar SystemObject::vcpu_unlockUI( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	WASABI_API_SKIN->skin_setLockUI( 0 );
	RETURN_SCRIPT_NULL;
}

#endif

scriptVar SystemObject::vcpu_isObjectValid( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar o )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_BOOLEAN( SystemObject::isObjectValid( GET_SCRIPT_OBJECT( o ) ) );
}

scriptVar SystemObject::vcpu_getTimeOfDay( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	time_t ts = Wasabi::Std::getTimeStamp();
	struct tm *tm_now;
	tm_now = localtime( (const time_t *)&ts );
	uint32_t tcnow = Wasabi::Std::getTickCount();
	static uint32_t lasttc = 0;

	int v = tm_now->tm_hour * 3600000 + tm_now->tm_min * 60000 + tm_now->tm_sec * 1000;

	// yay milliseconds!

	static int lastv = 0;
	static int total = 0;
	int tv = v;
	if ( v == lastv )
	{
		total += tcnow - lasttc;
		v += total;
	}
	else total = 0;
	lasttc = tcnow;
	lastv = tv;

	return MAKE_SCRIPT_INT( v );
}

scriptVar SystemObject::vcpu_navigateUrl( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar url )
{
	SCRIPT_FUNCTION_INIT;
	SystemObject::navigateUrl( GET_SCRIPT_STRING( url ) );
	RETURN_SCRIPT_VOID;
}

scriptVar SystemObject::vcpu_navigateUrlBrowser( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar url )
{
	SCRIPT_FUNCTION_INIT;
	SystemObject::navigateUrlBrowser( GET_SCRIPT_STRING( url ) );
	RETURN_SCRIPT_VOID;
}

scriptVar SystemObject::vcpu_isKeyDown( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar vk_code )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_INT( Std::keyDown( GET_SCRIPT_INT( vk_code ) ) );
}

scriptVar SystemObject::vcpu_setClipboard( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar text )
{
	SCRIPT_FUNCTION_INIT;
#ifdef _WIN32
	const wchar_t *source = GET_SCRIPT_STRING( text );
	DebugStringW( L"%s\n", source );
	OpenClipboard( 0 );
	EmptyClipboard();
	int len = ( wcslen( source ) + 1 );
	HGLOBAL clipbuffer = GlobalAlloc( GMEM_DDESHARE, sizeof( wchar_t ) * len );
	wchar_t *buffer = (wchar_t *)GlobalLock( clipbuffer );
	wcsncpy( buffer, source, len - 1 );
	GlobalUnlock( clipbuffer );
	SetClipboardData( CF_UNICODETEXT, clipbuffer );
	CloseClipboard();
#else
	DebugString( "portme SystemObject::vcpu_setClipboard\n" );
#endif
	RETURN_SCRIPT_VOID;
}

scriptVar SystemObject::vcpu_chr( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar n )
{
	SCRIPT_FUNCTION_INIT;
	WCSCPYN( staticStr, StringPrintfW( L"%c", GET_SCRIPT_INT( n ) ), 4096 );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_onAccelerator( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar action, scriptVar section, scriptVar key )
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS3( o, systemController, action, section, key );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT3( o, action, section, key );
}

scriptVar SystemObject::vcpu_triggerAction( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar guiobj, scriptVar actionstr, scriptVar paramstr )
{
	SCRIPT_FUNCTION_INIT;
	ASSERT( actionstr.type == SCRIPT_STRING );
	ASSERT( paramstr.type == SCRIPT_STRING );
	const wchar_t *astr = GET_SCRIPT_STRING( actionstr );
	const wchar_t *pstr = GET_SCRIPT_STRING( paramstr );
#ifdef WASABI_COMPILE_WNDMGR
	ScriptObject *context = guiobj.data.odata;
	if ( context != NULL )
	{
		GuiObject *go = static_cast<GuiObject *>( context->vcpu_getInterface( guiObjectGuid ) );
		if ( go != NULL )
		{
			ifc_window *wc = go->guiobject_getRootWnd();
			if ( wc != NULL )
			{
				ifc_window *lr = wc->getDesktopParent();
				if ( lr != NULL )
				{
					Layout *lay = static_cast<Layout *>( lr->getInterface( layoutGuid ) );
					if ( lay != NULL )
					{
						int ia = WASABI_API_SKIN->parse( astr, L"internal_action" );
						if ( ia == ACTION_NONE )
						{
							ActionEnum ae( astr );
							svc_action *act = ae.getFirst();
							if ( act )
							{
								act->onAction( astr, pstr, 0, 0, NULL, 0, wc );
								SvcEnum::release( act );
							}
						}
						else
							lay->runAction( ia, pstr );
					}
				}
			}
		}
	}
	else
	{
		ActionEnum ae( astr );
		svc_action *act = ae.getFirst();
		if ( act )
		{
			act->onAction( astr, pstr );
			SvcEnum::release( act );
		}
	}
#endif
	RETURN_SCRIPT_VOID;
}

scriptVar SystemObject::vcpu_getParam( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	SystemObject *o = static_cast<SystemObject *>( object->vcpu_getInterface( systemObjectGuid ) );
	if ( o ) return MAKE_SCRIPT_STRING( o->getParam() );
	return MAKE_SCRIPT_STRING( L"" );
}

#ifdef WASABI_COMPILE_SKIN
scriptVar SystemObject::vcpu_getScriptGroup( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	SystemObject *o = static_cast<SystemObject *>( object->vcpu_getInterface( systemObjectGuid ) );
	if ( o )
	{
		Group *g = o->getParentGroup();
		return MAKE_SCRIPT_OBJECT( g ? g->getScriptObject() : NULL );
	}
	return MAKE_SCRIPT_OBJECT( NULL );
}

scriptVar SystemObject::vcpu_newGroup( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar groupname )
{
	SCRIPT_FUNCTION_INIT;
	GuiObject *g = SkinParser::newDynamicGroup( GET_SCRIPT_STRING( groupname ), GROUP_GROUP );
	if ( g != NULL )
	{
		SystemObject *so = static_cast<SystemObject *>( object->vcpu_getInterface( systemObjectGuid ) );
		so->addInstantiatedObject( g->guiobject_getScriptObject() );
	}
	return MAKE_SCRIPT_OBJECT( g ? g->guiobject_getScriptObject() : NULL );
}
#endif

scriptVar SystemObject::vcpu_getMousePosX( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt;
	Wasabi::Std::getMousePos( &pt );
	return MAKE_SCRIPT_INT( pt.x );
}

scriptVar SystemObject::vcpu_getMousePosY( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt;
	Wasabi::Std::getMousePos( &pt );
	return MAKE_SCRIPT_INT( pt.y );
}

scriptVar SystemObject::vcpu_minimizeApplication( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef _WIN32
	ShowWindow( WASABI_API_WND->main_getRootWnd()->gethWnd(), SW_MINIMIZE );
#else
	#warning port me
#endif
		RETURN_SCRIPT_VOID;
}

scriptVar SystemObject::vcpu_activateApplication( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef _WIN32
	SetForegroundWindow( WASABI_API_WND->main_getRootWnd()->gethWnd() );
#else
	#warning port me
#endif
		RETURN_SCRIPT_VOID;
}

scriptVar SystemObject::vcpu_restoreApplication( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef _WIN32
	ShowWindow( WASABI_API_WND->main_getRootWnd()->gethWnd(), SW_RESTORE );
#else
	#warning port me
#endif
		RETURN_SCRIPT_VOID;
}

scriptVar SystemObject::vcpu_isDesktopAlphaAvailable( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_CONFIG
	extern _bool cfg_uioptions_desktopalpha;
	return MAKE_SCRIPT_INT( Wasabi::Std::Wnd::isDesktopAlphaAvailable() && cfg_uioptions_desktopalpha.getValueAsInt() );
#else
	return MAKE_SCRIPT_INT( Wasabi::Std::Wnd::isDesktopAlphaAvailable() );
#endif
}

scriptVar SystemObject::vcpu_isTransparencyAvailable( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_INT( Wasabi::Std::Wnd::isTransparencyAvailable() );
}

scriptVar SystemObject::vcpu_isMinimized( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef _WIN32
	return MAKE_SCRIPT_INT( IsIconic( WASABI_API_WND->main_getRootWnd()->gethWnd() ) );
#else
	#warning port me
		RETURN_SCRIPT_ZERO;
#endif
}

// extern String System.translate(String str);
scriptVar SystemObject::vcpu_translate( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar str )
{
	SCRIPT_FUNCTION_INIT;
	WCSCPYN( translateStr, _( GET_SCRIPT_STRING( str ) ), ( sizeof( translateStr ) / sizeof( *translateStr ) ) );
	return MAKE_SCRIPT_STRING( translateStr );
}

// extern String System.getString(String table, Int id);
scriptVar SystemObject::vcpu_getString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar table, scriptVar id )
{
	SCRIPT_FUNCTION_INIT;
	const wchar_t *xlat = LocalesManager::GetString( GET_SCRIPT_STRING( table ), GET_SCRIPT_INT( id ) );
	if ( xlat )
		return MAKE_SCRIPT_STRING( xlat );
	else
		return MAKE_SCRIPT_STRING( L"" );
}

// extern String System.getString(String table, Int id);
scriptVar SystemObject::vcpu_getLanguageId( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	const wchar_t *langId = WASABI_API_LNG->GetLanguageIdentifier( LANG_IDENT_STR );
	if ( langId )
		return MAKE_SCRIPT_STRING( langId );
	else
		return MAKE_SCRIPT_STRING( L"" );
}

scriptVar SystemObject::vcpu_integerToString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar i )
{
	SCRIPT_FUNCTION_INIT;
	WCSNPRINTF( staticStr, 4096, L"%d", GET_SCRIPT_INT( i ) );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_stringToInteger( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar s )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_INT( WTOI( GET_SCRIPT_STRING( s ) ) );
}

scriptVar SystemObject::vcpu_floatToString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar f, scriptVar digits )
{
	SCRIPT_FUNCTION_INIT;
	// to whoever made this use StringPrintf, you left out the digits param, GRRRRRR!!
	double v = GET_SCRIPT_DOUBLE( f );
	StringPrintfW tmp( v );
	WCSCPYN( staticStr, tmp, ( sizeof( staticStr ) / sizeof( *staticStr ) ) );
	wchar_t *p = wcschr( staticStr, '.' );
	if ( p )
	{
		int numdigits = GET_SCRIPT_INT( digits );
		if ( numdigits > 0 )
		{
			int n = numdigits;
			int curnumdigits = staticStr + wcslen( staticStr ) - ( p + 1 );
			n -= curnumdigits;
			if ( n > 0 )
			{
				for ( int i = 0; i < n && i < ( sizeof( staticStr ) / sizeof( *staticStr ) ) - 128; i++ )
					wcscat( staticStr, L"0" );
			}
			else if ( n < 0 )
			{
				*( p + 1 + numdigits ) = 0;
			}
		}
		else
		{
			*p = 0;
		}
	}
	else
	{
		int n = GET_SCRIPT_INT( digits );
		if ( n > 0 )
		{
			wcscat( staticStr, L"." );
			for ( int i = 0; i < n && i < ( sizeof( staticStr ) / sizeof( *staticStr ) ) - 128; i++ )
				wcscat( staticStr, L"0" );
		}
	}
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_stringToFloat( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar s )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_FLOAT( (float)WTOF( GET_SCRIPT_STRING( s ) ) );
}

scriptVar SystemObject::vcpu_integerToTime( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i )
{
	SCRIPT_FUNCTION_INIT;
	int i = GET_SCRIPT_INT( _i );
	WCSNPRINTF( staticStr, 4096, L"%d:%02d", i / 60000, i % 60000 / 1000 );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_integerToLongTime( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i )
{
	SCRIPT_FUNCTION_INIT;
	int i = GET_SCRIPT_INT( _i );
	WCSNPRINTF( staticStr, 4096, L"%d:%02d:%02d", i / 3600000, ( i % 3600000 ) / 60000, i % 60000 / 1000 );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_dateToTime( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i )
{
	SCRIPT_FUNCTION_INIT;
	//CUT: int i = GET_SCRIPT_INT(_i);
	time_t tt = GET_SCRIPT_INT( _i );
	struct tm *t = localtime( &tt );
	WCSNPRINTF( staticStr, 4096, L"%d:%02d", t->tm_hour, t->tm_min );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_dateToLongTime( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i )
{
	SCRIPT_FUNCTION_INIT;
	//CUT: int i = GET_SCRIPT_INT(_i);
	time_t tt = GET_SCRIPT_INT( _i );
	struct tm *t = localtime( &tt );
	WCSNPRINTF( staticStr, 4096, L"%d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_formatDate( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i )
{
	SCRIPT_FUNCTION_INIT;
	//CUT: int i = GET_SCRIPT_INT(_i);
	time_t tt = GET_SCRIPT_INT( _i );
	struct tm *t = localtime( &tt );
	wcsftime( staticStr, 4096, L"%c", t );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_formatLongDate( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i )
{
	SCRIPT_FUNCTION_INIT;
	//CUT: int i = GET_SCRIPT_INT(_i);
	time_t tt = GET_SCRIPT_INT( _i );
	struct tm *t = localtime( &tt );
	wcsftime( staticStr, 4096, L"%#c", t );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_getDateYear( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i )
{
	SCRIPT_FUNCTION_INIT;
	//CUT: int i = GET_SCRIPT_INT(_i);
	time_t tt = GET_SCRIPT_INT( _i );
	struct tm *t = localtime( &tt );
	return MAKE_SCRIPT_INT( t->tm_year );
}

scriptVar SystemObject::vcpu_getDateMonth( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i )
{
	SCRIPT_FUNCTION_INIT;
	//CUT: int i = GET_SCRIPT_INT(_i);
	time_t tt = GET_SCRIPT_INT( _i );
	struct tm *t = localtime( &tt );
	return MAKE_SCRIPT_INT( t->tm_mon );
}

scriptVar SystemObject::vcpu_getDateDay( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i )
{
	SCRIPT_FUNCTION_INIT;
	//CUT: int i = GET_SCRIPT_INT(_i);
	time_t tt = GET_SCRIPT_INT( _i );
	struct tm *t = localtime( &tt );
	return MAKE_SCRIPT_INT( t->tm_mday );
}

scriptVar SystemObject::vcpu_getDateDow( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i )
{
	SCRIPT_FUNCTION_INIT;
	//CUT: int i = GET_SCRIPT_INT(_i);
	time_t tt = GET_SCRIPT_INT( _i );
	struct tm *t = localtime( &tt );
	return MAKE_SCRIPT_INT( t->tm_wday );
}

scriptVar SystemObject::vcpu_getDateDoy( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i )
{
	SCRIPT_FUNCTION_INIT;
	//CUT: int i = GET_SCRIPT_INT(_i);
	time_t tt = GET_SCRIPT_INT( _i );
	struct tm *t = localtime( &tt );
	return MAKE_SCRIPT_INT( t->tm_yday );
}

scriptVar SystemObject::vcpu_getDateHour( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i )
{
	SCRIPT_FUNCTION_INIT;
	//CUT: int i = GET_SCRIPT_INT(_i);
	time_t tt = GET_SCRIPT_INT( _i );
	struct tm *t = localtime( &tt );
	return MAKE_SCRIPT_INT( t->tm_hour );
}

scriptVar SystemObject::vcpu_getDateMin( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i )
{
	SCRIPT_FUNCTION_INIT;
	//CUT: int i = GET_SCRIPT_INT(_i);
	time_t tt = GET_SCRIPT_INT( _i );
	struct tm *t = localtime( &tt );
	return MAKE_SCRIPT_INT( t->tm_min );
}

scriptVar SystemObject::vcpu_getDateSec( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i )
{
	SCRIPT_FUNCTION_INIT;
	//CUT: int i = GET_SCRIPT_INT(_i);
	time_t tt = GET_SCRIPT_INT( _i );
	struct tm *t = localtime( &tt );
	return MAKE_SCRIPT_INT( t->tm_sec );
}

scriptVar SystemObject::vcpu_getDateDst( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _i )
{
	SCRIPT_FUNCTION_INIT;
	//CUT: int i = GET_SCRIPT_INT(_i);
	time_t tt = GET_SCRIPT_INT( _i );
	struct tm *t = localtime( &tt );
	return MAKE_SCRIPT_INT( t->tm_isdst );
}

scriptVar SystemObject::vcpu_getDate( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	time_t t;
	time( &t );
	return MAKE_SCRIPT_INT( (int)t );
}

scriptVar SystemObject::vcpu_strmid( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar str, scriptVar start, scriptVar len )
{
	SCRIPT_FUNCTION_INIT;
	SOM::mid( staticStr, GET_SCRIPT_STRING( str ), MIN( GET_SCRIPT_INT( start ), 4096 ), MIN( GET_SCRIPT_INT( len ), 4096 ) );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_strleft( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar str, scriptVar len )
{
	SCRIPT_FUNCTION_INIT;
	SOM::mid( staticStr, GET_SCRIPT_STRING( str ), 0, MIN( GET_SCRIPT_INT( len ), 4096 ) );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_strright( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str, scriptVar _len )
{
	SCRIPT_FUNCTION_INIT;
	const wchar_t *str = GET_SCRIPT_STRING( _str );
	int len = GET_SCRIPT_INT( _len );
	SOM::mid( staticStr, str, MIN( (int)( wcslen( str ) - len ), 4096 ), MIN( len, 4096 ) );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_strsearch( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str, scriptVar substr )
{
	SCRIPT_FUNCTION_INIT;
	const wchar_t *str = GET_SCRIPT_STRING( _str );
	const wchar_t *p = wcsstr( str, GET_SCRIPT_STRING( substr ) );
	return MAKE_SCRIPT_INT( p ? p - str : -1 );
}

scriptVar SystemObject::vcpu_strlen( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar str )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_INT( wcslen( GET_SCRIPT_STRING( str ) ) );
}

scriptVar SystemObject::vcpu_strupper( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str )
{
	SCRIPT_FUNCTION_INIT;
	WCSCPYN( staticStr, GET_SCRIPT_STRING( _str ), ( sizeof( staticStr ) / sizeof( *staticStr ) ) );
	WCSTOUPPER( staticStr );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_strlower( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str )
{
	SCRIPT_FUNCTION_INIT;
	WCSCPYN( staticStr, GET_SCRIPT_STRING( _str ), ( sizeof( staticStr ) / sizeof( *staticStr ) ) );
	WCSTOLOWER( staticStr );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_urlencode( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str )
{
	SCRIPT_FUNCTION_INIT;
	StringW str( GET_SCRIPT_STRING( _str ) );
	str = AutoWide( AutoUrl( str ) );
	*staticStr = 0;
	if ( !str.isempty() ) WCSCPYN( staticStr, str, ( sizeof( staticStr ) / sizeof( *staticStr ) ) );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_urldecode( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str )
{
	SCRIPT_FUNCTION_INIT;
	StringW str( GET_SCRIPT_STRING( _str ) );
	Url::decode( str ); // Martin> I know there might be something nicer than this, but this does at least work
	*staticStr = 0;
	if ( !str.isempty() ) WCSCPYN( staticStr, str, ( sizeof( staticStr ) / sizeof( *staticStr ) ) );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_removepath( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str )
{
	SCRIPT_FUNCTION_INIT;
	PathParserW pp( GET_SCRIPT_STRING( _str ) );
//  ASSERT(pp.getNumStrings() >= 1);	// shouldn't happen ever
	wchar_t *lastString = pp.getLastString();
	if ( lastString )
	{
		WCSCPYN( staticStr, lastString, ( sizeof( staticStr ) / sizeof( *staticStr ) ) );
	}
	else
	{
		staticStr[ 0 ] = 0;
	}
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_getpath( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str )
{
	SCRIPT_FUNCTION_INIT;
	WCSCPYN( staticStr, GET_SCRIPT_STRING( _str ), ( sizeof( staticStr ) / sizeof( *staticStr ) ) );
#ifdef _WIN32
	PathRemoveFileSpecW( staticStr );
	PathRemoveBackslashW( staticStr );
#else
	#warning port me
#endif
		return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_getextension( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _str )
{
	SCRIPT_FUNCTION_INIT;
	const wchar_t *pt = Wasabi::Std::extension( GET_SCRIPT_STRING( _str ) );
	if ( pt != NULL ) WCSCPYN( staticStr, pt, ( sizeof( staticStr ) / sizeof( *staticStr ) ) );
	else *staticStr = '\0';
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_gettoken( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar tok, scriptVar sep, scriptVar num )
{
	SCRIPT_FUNCTION_INIT;
	wchar_t *p = const_cast<wchar_t *>( GET_SCRIPT_STRING( tok ) );
	wchar_t *d = const_cast<wchar_t *>( GET_SCRIPT_STRING( sep ) );
	int i = GET_SCRIPT_INT( num );
	if ( !d ) return MAKE_SCRIPT_STRING( L"" );
	if ( !p ) return MAKE_SCRIPT_STRING( L"" );
	wchar_t c = *d;
	d = staticStr;
	int n = 0, ct = 0;
	while ( p && *p && ct < 4096 )
	{
		if ( *p == c )
		{
			n++; p++; continue;
		}
		if ( n > i ) break;
		if ( n == i )
			*d++ = *p;
		p++;
		ct++;
	}
	*d = 0;
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_sin( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar d )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_DOUBLE( ::sin( GET_SCRIPT_DOUBLE( d ) ) );
}

scriptVar SystemObject::vcpu_cos( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar d )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_DOUBLE( ::cos( GET_SCRIPT_DOUBLE( d ) ) );
}

scriptVar SystemObject::vcpu_tan( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar d )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_DOUBLE( ::tan( GET_SCRIPT_DOUBLE( d ) ) );
}

scriptVar SystemObject::vcpu_asin( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar d )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_DOUBLE( ::asin( GET_SCRIPT_DOUBLE( d ) ) );
}

scriptVar SystemObject::vcpu_acos( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar d )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_DOUBLE( ::acos( GET_SCRIPT_DOUBLE( d ) ) );
}

scriptVar SystemObject::vcpu_atan( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar d )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_DOUBLE( ::atan( GET_SCRIPT_DOUBLE( d ) ) );
}

scriptVar SystemObject::vcpu_atan2( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar y, scriptVar x )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_DOUBLE( ::atan2( GET_SCRIPT_DOUBLE( y ), GET_SCRIPT_DOUBLE( x ) ) );
}

scriptVar SystemObject::vcpu_pow( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_DOUBLE( ::pow( GET_SCRIPT_DOUBLE( x ), GET_SCRIPT_DOUBLE( y ) ) );
}

scriptVar SystemObject::vcpu_sqr( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _d )
{
	SCRIPT_FUNCTION_INIT;
	double d = GET_SCRIPT_DOUBLE( _d );
	return MAKE_SCRIPT_DOUBLE( d * d );
}

scriptVar SystemObject::vcpu_sqrt( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar d )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_DOUBLE( ::sqrt( GET_SCRIPT_DOUBLE( d ) ) );
}

scriptVar SystemObject::vcpu_log10( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar d )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_DOUBLE( ::log10( GET_SCRIPT_DOUBLE( d ) ) );
}

scriptVar SystemObject::vcpu_log( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar d )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_DOUBLE( ::log( GET_SCRIPT_DOUBLE( d ) ) );
}


scriptVar SystemObject::vcpu_random( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar m )
{
	SCRIPT_FUNCTION_INIT;
	int m_ = GET_SCRIPT_INT( m );
	return MAKE_SCRIPT_INT( m_ <= 0 ? 0 : ::rand() % m_ );
}

scriptVar SystemObject::vcpu_integer( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar v )
{
	SCRIPT_FUNCTION_INIT;
	// precision will get lost automatically
	int _v = GET_SCRIPT_INT( v );
	return MAKE_SCRIPT_INT( _v );
}

scriptVar SystemObject::vcpu_frac( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar v )
{
	SCRIPT_FUNCTION_INIT;
	double _v = GET_SCRIPT_DOUBLE( v ) - GET_SCRIPT_INT( v );
	return MAKE_SCRIPT_DOUBLE( _v );
}

scriptVar SystemObject::vcpu_onViewPortChanged( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar width, scriptVar height )
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS2( o, systemController, width, height );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT2( o, width, height );
}

scriptVar SystemObject::vcpu_getViewportWidth( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt = { 0 };
	RECT r = { 0 };
	Wasabi::Std::getViewport( &r, &pt );
	return MAKE_SCRIPT_INT( r.right - r.left );
}

scriptVar SystemObject::vcpu_getViewportHeight( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt = { 0 };
	RECT r = { 0 };
	Wasabi::Std::getViewport( &r, &pt );
	return MAKE_SCRIPT_INT( r.bottom - r.top );
}

scriptVar SystemObject::vcpu_getViewportWidthGO( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar obj )
{
	SCRIPT_FUNCTION_INIT;
	RECT r = { 0 };
	GuiObject *o = static_cast<GuiObject *>( GET_SCRIPT_OBJECT_AS( obj, guiObjectGuid ) );
	if ( o )
	{
		Wasabi::Std::getViewport( &r, o->guiobject_getRootWnd()->getOsWindowHandle() );
		return MAKE_SCRIPT_INT( r.right - r.left );
	}
	else
	{
		POINT pt = { 0 };
		Wasabi::Std::getViewport( &r, &pt );
		return MAKE_SCRIPT_INT( r.right - r.left );
	}
	return MAKE_SCRIPT_INT( 0 );
}

scriptVar SystemObject::vcpu_getViewportHeightGO( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar obj )
{
	SCRIPT_FUNCTION_INIT;
	RECT r = { 0 };
	GuiObject *o = static_cast<GuiObject *>( GET_SCRIPT_OBJECT_AS( obj, guiObjectGuid ) );
	if ( o )
	{
		Wasabi::Std::getViewport( &r, o->guiobject_getRootWnd()->getOsWindowHandle() );

		// benski> this prevents the window from going "fullscreen" but adds a one-line space at the bottom
		RECT full = { 0 };
		Wasabi::Std::getViewport( &full, o->guiobject_getRootWnd()->getOsWindowHandle(), true );
		if (Wasabi::Std::rectEqual( &full, &r ) )
			r.bottom--;

		return MAKE_SCRIPT_INT( r.bottom - r.top );
	}
	else
	{
		POINT pt = { 0 };
		Wasabi::Std::getViewport( &r, &pt );
		return MAKE_SCRIPT_INT( r.bottom - r.top );
	}
	return MAKE_SCRIPT_INT( 0 );
}

scriptVar SystemObject::vcpu_getMonitorLeft( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt = { 0 };
	RECT r = { 0 };
	Wasabi::Std::getViewport( &r, &pt, 1 );
	return MAKE_SCRIPT_INT( r.left );
}

scriptVar SystemObject::vcpu_getMonitorLeftFP( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt = { GET_SCRIPT_INT( x ),GET_SCRIPT_INT( y ) };
	RECT r = { 0 };
	Wasabi::Std::getViewport( &r, &pt, 1 );
	return MAKE_SCRIPT_INT( r.left );
}

scriptVar SystemObject::vcpu_getMonitorLeftGO( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar obj )
{
	SCRIPT_FUNCTION_INIT;
	RECT r = { 0 };
	GuiObject *o = static_cast<GuiObject *>( GET_SCRIPT_OBJECT_AS( obj, guiObjectGuid ) );
	if ( o )
	{
		Wasabi::Std::getViewport( &r, o->guiobject_getRootWnd()->getOsWindowHandle(), 1 );
		return MAKE_SCRIPT_INT( r.left );
	}
	else
	{
		POINT pt = { 0 };
		Wasabi::Std::getViewport( &r, &pt, 1 );
		return MAKE_SCRIPT_INT( r.left );
	}
	return MAKE_SCRIPT_INT( 0 );
}

scriptVar SystemObject::vcpu_getMonitorTop( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt = { 0 };
	RECT r = { 0 };
	Wasabi::Std::getViewport( &r, &pt, 1 );
	return MAKE_SCRIPT_INT( r.top );
}

scriptVar SystemObject::vcpu_getMonitorTopFP( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt = { GET_SCRIPT_INT( x ),GET_SCRIPT_INT( y ) };
	RECT r = { 0 };
	Wasabi::Std::getViewport( &r, &pt, 1 );
	return MAKE_SCRIPT_INT( r.top );
}

scriptVar SystemObject::vcpu_getMonitorTopGO( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar obj )
{
	SCRIPT_FUNCTION_INIT;
	RECT r = { 0 };
	GuiObject *o = static_cast<GuiObject *>( GET_SCRIPT_OBJECT_AS( obj, guiObjectGuid ) );
	if ( o )
	{
		Wasabi::Std::getViewport( &r, o->guiobject_getRootWnd()->getOsWindowHandle(), 1 );
		return MAKE_SCRIPT_INT( r.top );
	}
	else
	{
		POINT pt = { 0 };
		Wasabi::Std::getViewport( &r, &pt, 1 );
		return MAKE_SCRIPT_INT( r.top );
	}
	return MAKE_SCRIPT_INT( 0 );
}

scriptVar SystemObject::vcpu_getMonitorWidth( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt = { 0 };
	RECT r = { 0 };
	Wasabi::Std::getViewport( &r, &pt, 1 );
	return MAKE_SCRIPT_INT( r.right - r.left );
}

scriptVar SystemObject::vcpu_getMonitorHeight( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt = { 0 };
	RECT r = { 0 };
	Wasabi::Std::getViewport( &r, &pt, 1 );
	return MAKE_SCRIPT_INT( r.bottom - r.top );
}

scriptVar SystemObject::vcpu_getMonitorWidthGO( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar obj )
{
	SCRIPT_FUNCTION_INIT;
	RECT r = { 0 };
	GuiObject *o = static_cast<GuiObject *>( GET_SCRIPT_OBJECT_AS( obj, guiObjectGuid ) );
	if ( o )
	{
		Wasabi::Std::getViewport( &r, o->guiobject_getRootWnd()->getOsWindowHandle(), 1 );
		return MAKE_SCRIPT_INT( r.right - r.left );
	}
	else
	{
		POINT pt = { 0 };
		Wasabi::Std::getViewport( &r, &pt, 1 );
		return MAKE_SCRIPT_INT( r.right - r.left );
	}
	return MAKE_SCRIPT_INT( 0 );
}

scriptVar SystemObject::vcpu_getMonitorHeightGO( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar obj )
{
	SCRIPT_FUNCTION_INIT;
	RECT r = { 0 };
	GuiObject *o = static_cast<GuiObject *>( GET_SCRIPT_OBJECT_AS( obj, guiObjectGuid ) );
	if ( o )
	{
		Wasabi::Std::getViewport( &r, o->guiobject_getRootWnd()->getOsWindowHandle(), true );
		return MAKE_SCRIPT_INT( r.bottom - r.top );
	}
	else
	{
		POINT pt = { 0 };
		Wasabi::Std::getViewport( &r, &pt, 1 );
		return MAKE_SCRIPT_INT( r.bottom - r.top );
	}
	return MAKE_SCRIPT_INT( 0 );
}

scriptVar SystemObject::vcpu_getViewportLeft( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt = { 0 };
	RECT r = { 0 };
	Wasabi::Std::getViewport( &r, &pt );
	return MAKE_SCRIPT_INT( r.left );
}

scriptVar SystemObject::vcpu_getViewportTop( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt = { 0 };
	RECT r = { 0 };
	Wasabi::Std::getViewport( &r, &pt );
	return MAKE_SCRIPT_INT( r.top );
}

scriptVar SystemObject::vcpu_getViewportLeftGO( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar obj )
{
	SCRIPT_FUNCTION_INIT;
	RECT r = { 0 };
	GuiObject *o = static_cast<GuiObject *>( GET_SCRIPT_OBJECT_AS( obj, guiObjectGuid ) );
	if ( o )
	{
		Wasabi::Std::getViewport( &r, o->guiobject_getRootWnd()->getOsWindowHandle() );
		return MAKE_SCRIPT_INT( r.left );
	}
	else
	{
		POINT pt = { 0 };
		Wasabi::Std::getViewport( &r, &pt );
		return MAKE_SCRIPT_INT( r.left );
	}
	return MAKE_SCRIPT_INT( 0 );
}

scriptVar SystemObject::vcpu_getViewportTopGO( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar obj )
{
	SCRIPT_FUNCTION_INIT;
	RECT r = { 0 };
	GuiObject *o = static_cast<GuiObject *>( GET_SCRIPT_OBJECT_AS( obj, guiObjectGuid ) );
	if ( o )
	{
		Wasabi::Std::getViewport( &r, o->guiobject_getRootWnd()->getOsWindowHandle() );
		return MAKE_SCRIPT_INT( r.top );
	}
	else
	{
		POINT pt = { 0 };
		Wasabi::Std::getViewport( &r, &pt );
		return MAKE_SCRIPT_INT( r.top );
	}
	return MAKE_SCRIPT_INT( 0 );
}

scriptVar SystemObject::vcpu_getViewportWidthFP( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt = { GET_SCRIPT_INT( x ),GET_SCRIPT_INT( y ) };
	RECT r = { 0 };
	Wasabi::Std::getViewport( &r, &pt );
	return MAKE_SCRIPT_INT( r.right - r.left );
}

scriptVar SystemObject::vcpu_getMonitorHeightFP( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt = { GET_SCRIPT_INT( x ),GET_SCRIPT_INT( y ) };
	RECT r = { 0 };
	Wasabi::Std::getViewport( &r, &pt, 1 );
	return MAKE_SCRIPT_INT( r.bottom - r.top );
}

scriptVar SystemObject::vcpu_getMonitorWidthFP( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt = { GET_SCRIPT_INT( x ),GET_SCRIPT_INT( y ) };
	RECT r = { 0 };
	Wasabi::Std::getViewport( &r, &pt, 1 );
	return MAKE_SCRIPT_INT( r.right - r.left );
}

scriptVar SystemObject::vcpu_getViewportHeightFP( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt = { GET_SCRIPT_INT( x ),GET_SCRIPT_INT( y ) };
	RECT r = { 0 };
	Wasabi::Std::getViewport( &r, &pt );
	return MAKE_SCRIPT_INT( r.bottom - r.top );
}

scriptVar SystemObject::vcpu_getViewportLeftFP( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt = { GET_SCRIPT_INT( x ),GET_SCRIPT_INT( y ) };
	RECT r = { 0 };
	Wasabi::Std::getViewport( &r, &pt );
	return MAKE_SCRIPT_INT( r.left );
}

scriptVar SystemObject::vcpu_getViewportTopFP( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar x, scriptVar y )
{
	SCRIPT_FUNCTION_INIT;
	POINT pt = { GET_SCRIPT_INT( x ),GET_SCRIPT_INT( y ) };
	RECT r = { 0 };
	Wasabi::Std::getViewport( &r, &pt );
	return MAKE_SCRIPT_INT( r.top );
}

scriptVar SystemObject::vcpu_getTickCount( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_INT(Wasabi::Std::getTickCount() );
}

scriptVar SystemObject::vcpu_onKeyDown( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v )
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS1( o, systemController, v );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT1( o, v );
}

scriptVar SystemObject::vcpu_onKeyUp( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v )
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS1( o, systemController, v );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT1( o, v );
}

scriptVar SystemObject::vcpu_debugString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar str, scriptVar severity )
{
	SCRIPT_FUNCTION_INIT;
	DebugStringW( L"%s", GET_SCRIPT_STRING( str ) );
	WASABI_API_SYSCB->syscb_issueCallback( SysCallback::CONSOLE, ConsoleCallback::DEBUGMESSAGE, GET_SCRIPT_INT( severity ), reinterpret_cast<intptr_t>( GET_SCRIPT_STRING( str ) ) );
	return MAKE_SCRIPT_VOID();
}

scriptVar SystemObject::vcpu_messageBox( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar msg, scriptVar title, scriptVar flags, scriptVar nam )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	return MAKE_SCRIPT_INT( WASABI_API_WNDMGR->messageBox( GET_SCRIPT_STRING( msg ), GET_SCRIPT_STRING( title ), GET_SCRIPT_INT( flags ), GET_SCRIPT_STRING( nam ), NULL ) );
#else
	return MAKE_SCRIPT_INT( MessageBox( NULL, GET_SCRIPT_STRING( msg ), GET_SCRIPT_STRING( title ), GET_SCRIPT_INT( flags ) ) );
#endif
}

scriptVar SystemObject::vcpu_getAtom( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar atomname )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_OBJECT( VCPU::getAtom( GET_SCRIPT_STRING( atomname ) ) );
}

scriptVar SystemObject::vcpu_setAtom( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar atomname, scriptVar obj )
{
	SCRIPT_FUNCTION_INIT;
	VCPU::setAtom( GET_SCRIPT_STRING( atomname ), GET_SCRIPT_OBJECT( obj ) );
	RETURN_SCRIPT_VOID;
}

#ifdef WASABI_COMPILE_MAKIDEBUG
scriptVar SystemObject::vcpu_invokeDebugger( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	WASABI_API_MAKIDEBUG->debugger_createJITD( object->vcpu_getScriptId(), 1 );
	RETURN_SCRIPT_VOID;
}
#endif

#if defined (WASABI_COMPILE_WNDMGR) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)

scriptVar SystemObject::vcpu_onCreateLayout( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar l )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	PROCESS_HOOKS1( o, systemController, l );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT1( o, l );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_onShowLayout( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar l )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	PROCESS_HOOKS1( o, systemController, l );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT1( o, l );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_onHideLayout( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar l )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	PROCESS_HOOKS1( o, systemController, l );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT1( o, l );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_getContainer( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar contname )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	Container *c = SkinParser::script_getContainer( GET_SCRIPT_STRING( contname ) );
	return MAKE_SCRIPT_OBJECT( c ? c->getScriptObject() : NULL );
#else
	RETURN_SCRIPT_NULL;
#endif
}

scriptVar SystemObject::vcpu_newDynamicContainer( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar contname )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	Container *c = SkinParser::newDynamicContainer( GET_SCRIPT_STRING( contname ) );
	return MAKE_SCRIPT_OBJECT( c ? c->getScriptObject() : NULL );
#else
	RETURN_SCRIPT_NULL;
#endif
}

scriptVar SystemObject::vcpu_newGroupAsLayout( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar groupname )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	Group *g = GroupMgr::instantiate( GET_SCRIPT_STRING( groupname ), GROUP_LAYOUTGROUP );
	if ( g )
	{
		Layout *l = static_cast<Layout *>( g );
		ifc_window *par = l->getCustomOwner();
		if ( !par ) par = WASABI_API_WND->main_getRootWnd();
		g->setAutoResizeAfterInit( 0 );
		g->setStartHidden( 1 );
		g->setParent( par );

#ifdef _WIN32
		g->init( hInstance, par->gethWnd(), TRUE );
#else
		#warning port me
			g->init( 0, par->gethWnd(), TRUE );
#endif
		SystemObject *so = SOM::getSystemObjectByScriptId( __vsd );
		if ( so != NULL ) so->addInstantiatedObject( g->getScriptObject() );
	}
	return MAKE_SCRIPT_OBJECT( g ? g->getScriptObject() : NULL );
#else
	RETURN_SCRIPT_NULL;
#endif
}

scriptVar SystemObject::vcpu_getNumContainers( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	return MAKE_SCRIPT_INT( SkinParser::getNumContainers() );
#else
	RETURN_SCRIPT_NULL;
#endif
}

scriptVar SystemObject::vcpu_enumContainer( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar n )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	Container *c = SkinParser::enumContainer( GET_SCRIPT_INT( n ) );
	return MAKE_SCRIPT_OBJECT( c ? c->getScriptObject() : NULL );
#else
	RETURN_SCRIPT_NULL;
#endif
}

scriptVar SystemObject::vcpu_onLookForComponent( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar guid )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	PROCESS_HOOKS1( o, systemController, guid );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT1( o, guid );
#else
	RETURN_SCRIPT_VOID
#endif
}

scriptVar SystemObject::vcpu_onGetCancelComponent( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar guid, scriptVar i )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	PROCESS_HOOKS2( o, systemController, guid, i );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT2( o, guid, i );
#else
	RETURN_SCRIPT_ZERO
#endif
}

scriptVar SystemObject::vcpu_getCurAppLeft( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	RECT r = { 0 };
	int s = getCurAppRect( &r );
	return MAKE_SCRIPT_INT( s ? r.left : -1 );
#else
	RETURN_SCRIPT_ZERO
#endif
}

scriptVar SystemObject::vcpu_getCurAppTop( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	RECT r = { 0 };
	int s = getCurAppRect( &r );
	return MAKE_SCRIPT_INT( s ? r.top : -1 );
#else
	RETURN_SCRIPT_ZERO
#endif
}

scriptVar SystemObject::vcpu_getCurAppWidth( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	RECT r = { 0 };
	int s = getCurAppRect( &r );
	return MAKE_SCRIPT_INT( s ? r.right - r.left : -1 );
#else
	RETURN_SCRIPT_ZERO
#endif
}

scriptVar SystemObject::vcpu_getCurAppHeight( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	RECT r = { 0 };
	int s = getCurAppRect( &r );
	return MAKE_SCRIPT_INT( s ? r.bottom - r.top : -1 );
#else
	RETURN_SCRIPT_ZERO
#endif
}

scriptVar SystemObject::vcpu_isAppActive( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	return MAKE_SCRIPT_BOOLEAN( isAppActive() );
#else
	RETURN_SCRIPT_ZERO
#endif
}

scriptVar SystemObject::vcpu_showWindow( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar wndguidorgroup, scriptVar prefcontainer, scriptVar transient )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	const wchar_t *s = GET_SCRIPT_STRING( wndguidorgroup );
	const wchar_t *p = GET_SCRIPT_STRING( prefcontainer );
	int t = GET_SCRIPT_INT( transient );
	if ( p != NULL && *p == 0 ) p = NULL;
	GUID _g = nsGUID::fromCharW( s );
#ifdef ON_CREATE_EXTERNAL_WINDOW_GUID
	if ( _g != INVALID_GUID )
	{
		int y = 0;
		ON_CREATE_EXTERNAL_WINDOW_GUID( _g, y );
		if ( y ) RETURN_SCRIPT_NULL;
	}
#endif
	ifc_window *ret = NULL;
	if ( _g != INVALID_GUID )
		ret = WASABI_API_WNDMGR->skinwnd_createByGuid( _g, p, 0, NULL, t );
	else
		ret = WASABI_API_WNDMGR->skinwnd_createByGroupId( s, p, 0, NULL, t );
	if ( ret )
	{
		if ( ret->getGuiObject() )
		{
			return MAKE_SCRIPT_OBJECT( ret->getGuiObject()->guiobject_getScriptObject() );
		}
	}
#endif
	RETURN_SCRIPT_NULL;
}

scriptVar SystemObject::vcpu_hideWindow( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar hw )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	GuiObject *o = static_cast<GuiObject *>( GET_SCRIPT_OBJECT_AS( hw, guiObjectGuid ) );
	if ( o )
		WASABI_API_WNDMGR->skinwnd_destroy( o->guiobject_getRootWnd() );
#endif
	RETURN_SCRIPT_NULL;
}

scriptVar SystemObject::vcpu_hideNamedWindow( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar guidorgroup )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_WNDMGR
	const wchar_t *s = GET_SCRIPT_STRING( guidorgroup );
	GUID _g = nsGUID::fromCharW( s );
	ifc_window *w = NULL;
	if ( _g != INVALID_GUID )
	{
		int n = WASABI_API_WNDMGR->skinwnd_getNumByGuid( _g ); // benski> CUT: int n = skinEmbedder->getNumItems(_g);
		if ( n > 0 )
			w = WASABI_API_WNDMGR->skinwnd_enumByGuid( _g, n - 1 ); // benski> CUT: w = skinEmbedder->enumItem(_g, n-1);
	}
	else
	{
		int n = WASABI_API_WNDMGR->skinwnd_getNumByGroupId( s ); // benski> CUT:    int n = skinEmbedder->getNumItems(s);
		if ( n > 0 )
			w = WASABI_API_WNDMGR->skinwnd_enumByGroupId( s, n - 1 ); // benski> CUT:    w = skinEmbedder->enumItem(s, n-1);
	}
	if ( w )
	{
		GuiObject *o = w->getGuiObject();
		if ( o )
			WASABI_API_WNDMGR->skinwnd_destroy( o->guiobject_getRootWnd() );
	}
#endif
	RETURN_SCRIPT_NULL;
}

scriptVar SystemObject::vcpu_isNamedWindowVisible( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar guidorgroup )
{
	SCRIPT_FUNCTION_INIT;
	const wchar_t *s = GET_SCRIPT_STRING( guidorgroup );
	GUID _g = nsGUID::fromCharW( s );
	ifc_window *w = NULL;
	if ( _g != INVALID_GUID )
	{
		int n = WASABI_API_WNDMGR->skinwnd_getNumByGuid( _g ); // benski> CUT: int n = skinEmbedder->getNumItems(_g);
		if ( n > 0 )
			w = WASABI_API_WNDMGR->skinwnd_enumByGuid( _g, n - 1 ); // benski> CUT: w = skinEmbedder->enumItem(_g, n-1);
	}
	else
	{
		int n = WASABI_API_WNDMGR->skinwnd_getNumByGroupId( s ); // benski> CUT:    int n = skinEmbedder->getNumItems(s);
		if ( n > 0 )
			w = WASABI_API_WNDMGR->skinwnd_enumByGroupId( s, n - 1 ); // benski> CUT:    w = skinEmbedder->enumItem(s, n-1);
	}
	if ( w )
	{
		return MAKE_SCRIPT_BOOLEAN( w->isVisible() );
	}
	RETURN_SCRIPT_ZERO;
}

#endif // if defined (WASABI_COMPILE_WNDMGR) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)

#if defined(WASABI_COMPILE_MEDIACORE) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)

scriptVar SystemObject::vcpu_getStatus( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	return MAKE_SCRIPT_INT( WASABI_API_MEDIACORE->core_getStatus( 0 ) );
#else
	RETURN_SCRIPT_ZERO;
#endif
}

scriptVar SystemObject::vcpu_getSongInfoText( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE

#ifdef GET_SONG_INFO_TEXT
	const wchar_t *c;
	GET_SONG_INFO_TEXT( c );
	WCSCPYN( staticStr, c, 4096 );
#else
	*staticStr = 0;
#pragma CHAT("lone", "bas", "need to get the song info text, the same as would be in a text field with SONGINFO")
#endif
	return MAKE_SCRIPT_STRING( staticStr );
#else
	RETURN_SCRIPT_ZERO;
#endif
}

scriptVar SystemObject::vcpu_getSongInfoTextTranslated( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE

#ifdef GET_SONG_INFO_TEXT_TRANSLATED
	const wchar_t *c;
	GET_SONG_INFO_TEXT_TRANSLATED( c );
	WCSCPYN( staticStr, c, 4096 );
#else
	*staticStr = 0;
#endif
	return MAKE_SCRIPT_STRING( staticStr );
#else
	RETURN_SCRIPT_ZERO;
#endif
}

#ifdef GEN_FF
#include "../../../../Plugins/General/gen_ff/wa2frontend.h"
#endif

scriptVar SystemObject::vcpu_hasVideoSupport( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef GEN_FF
	return MAKE_SCRIPT_INT( wa2.hasVideoSupport() );
#else
#pragma CHAT("lone", "bas", "how can we ask if the currently playing file is video thru the core objects ?")
	RETURN_SCRIPT_ZERO;
#endif
}

scriptVar SystemObject::vcpu_isVideo( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef GEN_FF
	return MAKE_SCRIPT_INT( wa2.isPlayingVideo() );
#else
#pragma CHAT("lone", "bas", "how can we ask if the currently playing file is video thru the core objects ?")
	RETURN_SCRIPT_ZERO;
#endif
}

scriptVar SystemObject::vcpu_isVideoFullscreen( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef GEN_FF
	return MAKE_SCRIPT_INT( wa2.isPlayingVideoFullscreen() );
#else
#pragma CHAT("lone", "bas", "how can we ask if we are running fullscreen (either vis or video) ?")
	RETURN_SCRIPT_ZERO;
#endif
}

scriptVar SystemObject::vcpu_setVideoFullscreen( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar v )
{
	SCRIPT_FUNCTION_INIT;
#ifdef GEN_FF
	const bool vidfs = GET_SCRIPT_BOOLEAN( v );
	if ( vidfs )
	{
		wa2.sendVidCmd( Winamp2FrontEnd::WA2_VIDCMD_FULLSCREEN );
	}
	else
	{
		wa2.sendVidCmd( Winamp2FrontEnd::WA2_VIDCMD_EXIT_FS );
	}
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_getPlaylistIndex( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef GEN_FF
	return MAKE_SCRIPT_INT( wa2.getCurPlaylistEntry() );
#else
#ifdef WASABI_COMPILE_MEDIACORE
	return MAKE_SCRIPT_INT( CoreHandle().getCurPlaybackNumber() );
#else
	RETURN_SCRIPT_ZERO;
#endif
#endif
}

scriptVar SystemObject::vcpu_getPlaylistLength( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef GEN_FF
	return MAKE_SCRIPT_INT( wa2.getPlaylistLength() );
#else
#ifdef WASABI_COMPILE_MEDIACORE
	//BU CoreHandle is your friend
	return MAKE_SCRIPT_INT( CoreHandle().getNumTracks() );
#else
	//FG not always, heh
	RETURN_SCRIPT_ZERO;
#endif
#endif
}

scriptVar SystemObject::vcpu_getIdealVideoHeight( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef GEN_FF
	int h = -1;
	wa2.getIdealVideoSize( NULL, &h );
	return MAKE_SCRIPT_INT( h );
#else
#pragma CHAT("lone", "bas", "how can we ask for the ideal video size thru the core objects ?")
	RETURN_SCRIPT_ZERO;
#endif
}

scriptVar SystemObject::vcpu_getIdealVideoWidth( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef GEN_FF
	int w = -1;
	wa2.getIdealVideoSize( &w, NULL );
	return MAKE_SCRIPT_INT( w );
#else
#pragma CHAT("lone", "bas", "how can we ask for the ideal video size thru the core objects ?")
	RETURN_SCRIPT_ZERO;
#endif
}

scriptVar SystemObject::vcpu_onStop( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	PROCESS_HOOKS0( o, systemController );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT0( o );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_onShowNotification( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	PROCESS_HOOKS0( o, systemController );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT0( o );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_onPlay( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	PROCESS_HOOKS0( o, systemController );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT0( o );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_onPause( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	PROCESS_HOOKS0( o, systemController );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT0( o );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_onResume( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	PROCESS_HOOKS0( o, systemController );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT0( o );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_onTitleChange( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar title )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	PROCESS_HOOKS1( o, systemController, title );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT1( o, title );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_onTitle2Change( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar title )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	PROCESS_HOOKS1( o, systemController, title );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT1( o, title );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_onUrlChange( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar title )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	PROCESS_HOOKS1( o, systemController, title );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT1( o, title );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_onInfoChange( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar info )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	PROCESS_HOOKS1( o, systemController, info );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT1( o, info );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_onStatusMsg( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar info )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	PROCESS_HOOKS1( o, systemController, info );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT1( o, info );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_onEqBandChanged( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar band, scriptVar val )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	PROCESS_HOOKS2( o, systemController, band, val );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT2( o, band, val );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_onEqFreqChanged( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar val )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	PROCESS_HOOKS1( o, systemController, val );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT1( o, val );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_onEqPreAmpChanged( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar val )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	PROCESS_HOOKS1( o, systemController, val );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT1( o, val );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_onEqChanged( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar val )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	PROCESS_HOOKS1( o, systemController, val );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT1( o, val );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_onVolumeChanged( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar val )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	PROCESS_HOOKS1( o, systemController, val );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT1( o, val );
#else
	RETURN_SCRIPT_VOID
#endif
}

scriptVar SystemObject::vcpu_onSeeked( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar val )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	PROCESS_HOOKS1( o, systemController, val );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT1( o, val );
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_getPlayItemString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	const wchar_t *cur = WASABI_API_MEDIACORE->core_getCurrent( 0 );
	if ( cur == NULL ) cur = L"";
	return MAKE_SCRIPT_STRING( cur );
#else
	return MAKE_SCRIPT_STRING( L"" );
#endif
}

scriptVar SystemObject::vcpu_getPlayItemLength( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	return MAKE_SCRIPT_INT( WASABI_API_MEDIACORE->core_getLength( 0 ) );
#else
	RETURN_SCRIPT_ZERO;
#endif
}

scriptVar SystemObject::vcpu_getPlayItemDisplayTitle( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	WCSCPYN( staticStr, WASABI_API_MEDIACORE->core_getTitle( 0 ), 4096 );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_getPlayItemMetadataString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar name )
{
	SCRIPT_FUNCTION_INIT;
#if defined(WASABI_COMPILE_MEDIACORE) & defined(WASABI_COMPILE_METADB)
	const char *cur = WASABI_API_MEDIACORE->core_getCurrent( 0 );
	if ( cur == NULL ) return MAKE_SCRIPT_STRING( "" );
	if ( cur && WASABI_API_METADB->metadb_getMetaData( cur, GET_SCRIPT_STRING( name ), staticStr, ( sizeof( staticStr ) / sizeof( *staticStr ) ) ) <= 0 ) return MAKE_SCRIPT_STRING( "" );
	return MAKE_SCRIPT_STRING( staticStr );
#elif defined(WASABI_CUSTOM_MINIDB)
	WASABI_CUSTOM_MINIDB( GET_SCRIPT_STRING( name ), staticStr, 4096 ); staticStr[ 4095 ] = 0;
	return MAKE_SCRIPT_STRING( staticStr );
#elif defined(WASABI_COMPILE_MEDIACORE)
	STRNCPY( staticStr, WASABI_API_MEDIACORE->core_getCurrent( 0 ), 4095 ); staticStr[ 4095 ] = 0;
	return MAKE_SCRIPT_STRING( staticStr );
#endif
}

scriptVar SystemObject::vcpu_getMetadataString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar filename, scriptVar name )
{
	SCRIPT_FUNCTION_INIT;
	wa2.getMetaData( GET_SCRIPT_STRING( filename ), GET_SCRIPT_STRING( name ), staticStr, 4096 );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_getExtFamily( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar ext )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	const wchar_t *ps = GET_SCRIPT_STRING( ext );
	staticStr[ 0 ] = '\0';
	if ( ps != NULL )
	{
		const wchar_t *f = WASABI_API_MEDIACORE->core_getExtensionFamily( ps );
		if ( f == NULL ) f = L"";
		WCSCPYN( staticStr, f, 4096 );
	}
	return MAKE_SCRIPT_STRING( staticStr );
#else
	return MAKE_SCRIPT_STRING( L"" );
#endif
}

scriptVar SystemObject::vcpu_getDecoderName( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar ext )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	const wchar_t *ps = GET_SCRIPT_STRING( ext );
	staticStr[ 0 ] = '\0';
	if ( ps != NULL )
	{
		const wchar_t *f = WASABI_API_MEDIACORE->core_getDecoderName( ps );
		if ( f == NULL ) f = L"";
		WCSCPYN( staticStr, f, 4096 );
	}
	return MAKE_SCRIPT_STRING( staticStr );
#else
	return MAKE_SCRIPT_STRING( L"" );
#endif
}

scriptVar SystemObject::vcpu_playFile( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar file )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
#ifdef GEN_FF
	StringW _file = GET_SCRIPT_STRING( file );
	const wchar_t *f = _file.getValue();
	if ( f && *f )
	{
		if ( wcsstr( _file, L"://" ) )
		{
			wa2.playFile( _file );
		}
		else if ( f[ 1 ] != ':' && f[ 0 ] != '\\' && f[ 0 ] != '/' )
		{
			PathParserW pp( f );
			if ( WCSCASEEQLSAFE( pp.enumString( 0 ), L"skins" )
				&& WCSCASEEQLSAFE( pp.enumString( 1 ), WASABI_API_SKIN->getSkinName() ) )
			{
				_file = L"";
				for ( int i = 0; i < pp.getNumStrings() - 2; i++ )
				{
					if ( i > 0 ) _file += L"\\";
					_file += pp.enumString( i + 2 );
				}
			}
			_file = StringPathCombine( WASABI_API_SKIN->getSkinPath(), _file );
			if ( !WACCESS( _file, 0 ) ) // avoid clearing playlist if file not found
				wa2.playFile( _file );
		}
	}
#else
	svc_player *sp = SvcEnumByGuid<svc_player>();
	if ( sp ) sp->openFile( GET_SCRIPT_STRING( file ) );
	WASABI_API_SVC->service_release( sp );
#endif
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_enqueueFile( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar file )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
#ifdef GEN_FF
	StringW _file = GET_SCRIPT_STRING( file );
	const wchar_t *f = _file.getValue();
	if ( f && *f )
	{
		if ( wcsstr( _file, L"://" ) )
		{
			wa2.enqueueFile( _file );
		}
		else if ( f[ 1 ] != ':' && f[ 0 ] != '\\' && f[ 0 ] != '/' )
		{
			PathParserW pp( f );
			if ( WCSCASEEQLSAFE( pp.enumString( 0 ), L"skins" )
				&& WCSCASEEQLSAFE( pp.enumString( 1 ), WASABI_API_SKIN->getSkinName() ) )
			{
				_file = L"";
				for ( int i = 0; i < pp.getNumStrings() - 2; i++ )
				{
					if ( i > 0 ) _file += L"\\";
					_file += pp.enumString( i + 2 );
				}
			}
			_file = StringPathCombine( WASABI_API_SKIN->getSkinPath(), _file );
			wa2.enqueueFile( _file );
		}
	}
#else
	svc_player *sp = SvcEnumByGuid<svc_player>();
	if ( sp ) sp->openFile( GET_SCRIPT_STRING( file ) );
	WASABI_API_SVC->service_release( sp );
#endif
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_clearPlaylist( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	wa2.clearPlaylist();
	return MAKE_SCRIPT_VOID();
}

scriptVar SystemObject::vcpu_getLeftVuMeter( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	return MAKE_SCRIPT_INT( WASABI_API_MEDIACORE->core_getLeftVuMeter( 0 ) );
#else
	RETURN_SCRIPT_ZERO;
#endif
}

scriptVar SystemObject::vcpu_getVisBand( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar chan, scriptVar band )
{
	{
		SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
		unsigned char visdata[ 576 * 2 * 2 ] = { 0 };
		int ret = WASABI_API_MEDIACORE->core_getVisData( 0, &visdata, sizeof( visdata ) );
		if ( !ret ) MEMSET( visdata, 0, sizeof( visdata ) );
		int b = MIN( MAX( 0, GET_SCRIPT_INT( band ) ), 74 );
		int a = MIN( MAX( 0, GET_SCRIPT_INT( chan ) ), 1 );
		return MAKE_SCRIPT_INT( MIN( 255, visdata[ 576 * a + b ] * 16 ) );
#else
		RETURN_SCRIPT_ZERO;
#endif
	}
}

scriptVar SystemObject::vcpu_getRightVuMeter( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	return MAKE_SCRIPT_INT( WASABI_API_MEDIACORE->core_getRightVuMeter( 0 ) );
#else
	RETURN_SCRIPT_ZERO;
#endif
}

scriptVar SystemObject::vcpu_getVolume( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	return MAKE_SCRIPT_INT( WASABI_API_MEDIACORE->core_getVolume( 0 ) );
#else
	RETURN_SCRIPT_ZERO;
#endif
}

scriptVar SystemObject::vcpu_setVolume( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar v )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	WASABI_API_MEDIACORE->core_setVolume( 0, GET_SCRIPT_INT( v ) );
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_play( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	WASABI_API_MEDIACORE->core_userButton( 0, UserButton::PLAY );
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_stop( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	WASABI_API_MEDIACORE->core_userButton( 0, UserButton::STOP );
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_pause( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	WASABI_API_MEDIACORE->core_userButton( 0, UserButton::PAUSE );
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_next( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	WASABI_API_MEDIACORE->core_userButton( 0, UserButton::NEXT );
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_previous( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	WASABI_API_MEDIACORE->core_userButton( 0, UserButton::PREV );
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_eject( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
#ifdef GEN_FF
	wa2.openFileDialog( WASABI_API_WND->main_getRootWnd()->gethWnd() );
#else
	svc_player *sp = SvcEnumByGuid<svc_player>();
	sp->openFiles();
	WASABI_API_SVC->service_release( sp );
#endif
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_seekTo( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar to )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	WASABI_API_MEDIACORE->core_setPosition( 0, GET_SCRIPT_INT( to ) );
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_setEqBand( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar band, scriptVar val )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	WASABI_API_MEDIACORE->core_setEqBand( 0, MAX( 0, MIN( 9, GET_SCRIPT_INT( band ) ) ), GET_SCRIPT_INT( val ) );
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_setEqPreAmp( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar v )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	WASABI_API_MEDIACORE->core_setEqPreamp( 0, GET_SCRIPT_INT( v ) );
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_setEq( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar v )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	WASABI_API_MEDIACORE->core_setEqStatus( 0, GET_SCRIPT_INT( v ) );
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_getPosition( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	return MAKE_SCRIPT_INT( WASABI_API_MEDIACORE->core_getPosition( 0 ) );
#else
	RETURN_SCRIPT_ZERO;
#endif
}

scriptVar SystemObject::vcpu_getEqBand( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar band )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	return MAKE_SCRIPT_INT( WASABI_API_MEDIACORE->core_getEqBand( 0, MAX( 0, MIN( 9, GET_SCRIPT_INT( band ) ) ) ) );
#else
	RETURN_SCRIPT_ZERO;
#endif
}

scriptVar SystemObject::vcpu_getEqPreAmp( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	return MAKE_SCRIPT_INT( WASABI_API_MEDIACORE->core_getEqPreamp( 0 ) );
#else
	RETURN_SCRIPT_ZERO;
#endif
}

scriptVar SystemObject::vcpu_getEq( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	return MAKE_SCRIPT_INT( WASABI_API_MEDIACORE->core_getEqStatus( 0 ) );
#else
	RETURN_SCRIPT_ZERO;
#endif
}

#endif // #if defined(WASABI_COMPILE_MEDIACORE) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)

#if defined(WASABI_COMPILE_COMPONENTS) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)

scriptVar SystemObject::vcpu_getWac( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar guid )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_COMPONENTS
	WACObject *wo = SOM::getWACObject( GET_SCRIPT_STRING( guid ) );
	return MAKE_SCRIPT_OBJECT( wo ? wo->getScriptObject() : NULL );
#else
	RETURN_SCRIPT_NULL;
#endif
}

#endif // if defined(WASABI_COMPILE_COMPONENTS) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)

#if defined(WASABI_COMPILE_CONFIG) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)

scriptVar SystemObject::vcpu_setPrivateString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar section, scriptVar item, scriptVar str )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_CONFIG
	WCSNPRINTF( staticStr, 4096, L"%s_%s", GET_SCRIPT_STRING( section ), GET_SCRIPT_STRING( item ) );
	WASABI_API_CONFIG->setStringPrivate( staticStr, GET_SCRIPT_STRING( str ) );
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_setPrivateInt( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar section, scriptVar item, scriptVar val )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_CONFIG
	WCSNPRINTF( staticStr, 4096, L"%s_%s", GET_SCRIPT_STRING( section ), GET_SCRIPT_STRING( item ) );
	WASABI_API_CONFIG->setIntPrivate( staticStr, GET_SCRIPT_INT( val ) );
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_getPrivateString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar section, scriptVar item, scriptVar defstr )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_CONFIG
	wchar_t t[ 4096 ] = { 0 };
	StringCchPrintfW( t, 4096, L"%s_%s", GET_SCRIPT_STRING( section ), GET_SCRIPT_STRING( item ) );
	WASABI_API_CONFIG->getStringPrivate( t, staticStr, 4095, GET_SCRIPT_STRING( defstr ) );
	return MAKE_SCRIPT_STRING( staticStr );
#else
	return MAKE_SCRIPT_STRING( L"" );
#endif
}

scriptVar SystemObject::vcpu_getPrivateInt( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar section, scriptVar item, scriptVar defval )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_CONFIG
	wchar_t t[ 4096 ] = { 0 };
	StringCchPrintfW( t, 4096, L"%s_%s", GET_SCRIPT_STRING( section ), GET_SCRIPT_STRING( item ) );
	return MAKE_SCRIPT_INT( WASABI_API_CONFIG->getIntPrivate( t, GET_SCRIPT_INT( defval ) ) );
#else
	RETURN_SCRIPT_ZERO;
#endif
}

scriptVar SystemObject::vcpu_setPublicString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar item, scriptVar str )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_CONFIG
	WASABI_API_CONFIG->setStringPrivate( GET_SCRIPT_STRING( item ), GET_SCRIPT_STRING( str ) );
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_setPublicInt( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar item, scriptVar val )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_CONFIG
	WASABI_API_CONFIG->setIntPrivate( GET_SCRIPT_STRING( item ), GET_SCRIPT_INT( val ) );
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_getPublicString( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar item, scriptVar defstr )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_CONFIG
	WASABI_API_CONFIG->getStringPrivate( GET_SCRIPT_STRING( item ), staticStr, 4095, GET_SCRIPT_STRING( defstr ) );
	return MAKE_SCRIPT_STRING( staticStr );
#else
	return MAKE_SCRIPT_STRING( L"" );
#endif
}

scriptVar SystemObject::vcpu_getPublicInt( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar item, scriptVar defval )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_CONFIG
	return MAKE_SCRIPT_INT( WASABI_API_CONFIG->getIntPrivate( GET_SCRIPT_STRING( item ), GET_SCRIPT_INT( defval ) ) );
#else
	RETURN_SCRIPT_ZERO;
#endif
}

#endif // #if defined(WASABI_COMPILE_CONFIG) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)

#if defined (WA3COMPATIBILITY) || defined(WASABI_SCRIPT_SYSTEMOBJECT_WA3COMPATIBLE)

scriptVar SystemObject::vcpu_ddeSend( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar ddetarget, scriptVar cmd, scriptVar minInterval )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WA3COMPATIBILITY
	DdeCom::sendCommand( GET_SCRIPT_STRING( ddetarget ), GET_SCRIPT_STRING( cmd ), GET_SCRIPT_INT( minInterval ) );
#endif
	RETURN_SCRIPT_VOID;
}

scriptVar SystemObject::vcpu_popMb( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WA3COMPATIBILITY
	MainMiniBrowser::popMb();
#endif
	RETURN_SCRIPT_VOID;
}

scriptVar SystemObject::vcpu_getMainMB( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WA3COMPATIBILITY
	return MAKE_SCRIPT_OBJECT( MainMiniBrowser::getScriptObject() );
#else
	RETURN_SCRIPT_NULL;
#endif
}

scriptVar SystemObject::vcpu_setMenuTransparency( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar a )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WA3COMPATIBILITY
	WASABI_API_CONFIG->setIntPublic( "Popup alpha", GET_SCRIPT_INT( a ) );
#endif
	RETURN_SCRIPT_VOID;
}

scriptVar SystemObject::vcpu_windowMenu( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WA3COMPATIBILITY
	Main::thingerContextMenu( NULL );
#endif
	RETURN_SCRIPT_VOID;
}

scriptVar SystemObject::vcpu_systemMenu( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WA3COMPATIBILITY
	Main::appContextMenu( NULL, FALSE, FALSE );
#endif
	RETURN_SCRIPT_VOID;
}

scriptVar SystemObject::vcpu_selectFile( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar extlist, scriptVar id, scriptVar prevfn )
{
	SCRIPT_FUNCTION_INIT;
	SelectFile sf( WASABI_API_WND->main_getRootWnd() );
	sf.setIdent( GET_SCRIPT_STRING( id ) );
	StringW ff = GET_SCRIPT_STRING( prevfn );
	if ( !ff.isempty() )
	{
		wchar_t *p = const_cast<wchar_t *>(Wasabi::Std::filename( ff ) );
		if ( p != NULL ) *p = 0;
		sf.setDefaultDir( ff );
	}
	StringW exts = GET_SCRIPT_STRING( extlist );
	if ( exts.isempty() )
		exts = L"All files(*.*)|*.*||";
	exts.changeChar( '|', '\0' );
	int r = sf.runSelector( L"files", FALSE, exts );
	StringW t;
	if ( !r || sf.getNumFiles() < 1 )
	{
		t = GET_SCRIPT_STRING( prevfn );
	}
	else
	{
		t = sf.enumFilename( 0 );
	}
	WCSCPYN( staticStr, t, 4096 );
	return MAKE_SCRIPT_STRING( staticStr );
}

#endif

const wchar_t *dldir;

BOOL CALLBACK browseEnumProc( HWND hwnd, LPARAM lParam )
{
	wchar_t cl[ 32 ] = { 0 };
	GetClassName( hwnd, cl, ARRAYSIZE( cl ) );
	if ( !lstrcmpi( cl, WC_TREEVIEW ) )
	{
		PostMessage( hwnd, TVM_ENSUREVISIBLE, 0, (LPARAM)TreeView_GetSelection( hwnd ) );
		return FALSE;
	}

	return TRUE;
}

static int CALLBACK BrowseCallbackProcX( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
	switch ( uMsg )
	{
		case BFFM_INITIALIZED:
		{
			const wchar_t *wnd_title = (wchar_t *)lpData;
			//if(wnd_title) wnd_title = _(wnd_title);
			if ( WCSICMP( _( wnd_title ), L"" ) ) SetWindowTextW( hwnd, _( wnd_title ) );
			SendMessageW( hwnd, BFFM_SETSELECTIONW, 1, (LPARAM)dldir );

			// this is not nice but it fixes the selection not working correctly on all OSes
			EnumChildWindows( hwnd, browseEnumProc, 0 );
		}
		return 0;
	}
	return 0;
}

void Shell_Free( void *p )
{
	IMalloc *m;
	SHGetMalloc( &m );
	m->Free( p );
}

scriptVar SystemObject::vcpu_selectFolder( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar title, scriptVar info, scriptVar defPath )
{
	SCRIPT_FUNCTION_INIT;

	dldir = defPath.data.sdata;

	BROWSEINFOW bi = { 0 };
	bi.hwndOwner = WASABI_API_WND->main_getRootWnd()->gethWnd();
	bi.lpszTitle = _( info.data.sdata );	// Info Text
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseCallbackProcX;
	bi.lParam = (LPARAM)title.data.sdata;	// Window Title

	ITEMIDLIST *idlist = SHBrowseForFolderW( &bi );
	if ( idlist )
	{
		SHGetPathFromIDListW( idlist, staticStr );
		Shell_Free( idlist );

		return MAKE_SCRIPT_STRING( staticStr );
	}

	return MAKE_SCRIPT_STRING( L"" );
}

#ifdef WASABI_COMPILE_MEDIACORE

int SystemObject::corecb_onVolumeChange( int newvol )
{
	vcpu_onVolumeChanged( SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT( newvol ) );
	return 0;
}

int SystemObject::corecb_onEQStatusChange( int newval )
{
	vcpu_onEqChanged( SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT( newval ) );
	return 0;
}

int SystemObject::corecb_onEQPreampChange( int newval )
{
	vcpu_onEqPreAmpChanged( SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT( newval ) );
	return 0;
}

int SystemObject::corecb_onEQBandChange( int band, int newval )
{
	vcpu_onEqBandChanged( SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT( band ), MAKE_SCRIPT_INT( newval ) );
	return 0;
}

int SystemObject::corecb_onEQFreqChange( int newval )
{
	vcpu_onEqFreqChanged( SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT( newval ) );
	return 0;
}

int SystemObject::corecb_onStarted()
{
	vcpu_onPlay( SCRIPT_CALL, getScriptObject() );
	return 0;
}

int SystemObject::corecb_onStopped()
{
	vcpu_onStop( SCRIPT_CALL, getScriptObject() );
	return 0;
}

int SystemObject::corecb_onPaused()
{
	vcpu_onPause( SCRIPT_CALL, getScriptObject() );
	return 0;
}

int SystemObject::corecb_onUnpaused()
{
	vcpu_onResume( SCRIPT_CALL, getScriptObject() );
	return 0;
}

int SystemObject::corecb_onSeeked( int newpos )
{
	vcpu_onSeeked( SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT( newpos ) );
	return 0;
}

int SystemObject::corecb_onTitleChange( const wchar_t *title )
{
	vcpu_onTitleChange( SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING( title ) );
	return 0;
}

int SystemObject::corecb_onTitle2Change( const wchar_t *title )
{
	vcpu_onTitle2Change( SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING( title ) );
	return 0;
}

int SystemObject::corecb_onUrlChange( const wchar_t *url )
{
	vcpu_onUrlChange( SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING( url ) );
	return 0;
}

int SystemObject::corecb_onInfoChange( const wchar_t *info )
{
	vcpu_onInfoChange( SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING( info ) );
	return 0;
}

int SystemObject::corecb_onStatusMsg( const wchar_t *info )
{
	vcpu_onStatusMsg( SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING( info ) );
	return 0;
}
#endif

scriptVar SystemObject::vcpu_getRating( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	return MAKE_SCRIPT_INT( WASABI_API_MEDIACORE->core_getRating() );
#else
	RETURN_SCRIPT_ZERO;
#endif
}

scriptVar SystemObject::vcpu_setRating( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar v )
{
	SCRIPT_FUNCTION_INIT;
#ifdef WASABI_COMPILE_MEDIACORE
	WASABI_API_MEDIACORE->core_setRating( GET_SCRIPT_INT( v ) );
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SystemObject::vcpu_getDownloadPath( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	wa2.getDownloadPath( staticStr );
	return MAKE_SCRIPT_STRING( staticStr );
}

scriptVar SystemObject::vcpu_setDownloadPath( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar new_path )
{
	SCRIPT_FUNCTION_INIT;
	wa2.setDownloadPath( new_path.data.sdata );
	return MAKE_SCRIPT_VOID();
}

scriptVar SystemObject::vcpu_downloadURL( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar url, scriptVar filename, scriptVar title )
{
	SCRIPT_FUNCTION_INIT;
	wa2.DownloadFile( AutoChar( GET_SCRIPT_STRING( url ) ) );
	return MAKE_SCRIPT_VOID();
}

scriptVar SystemObject::vcpu_downloadMedia( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar url, scriptVar filepath, scriptVar storeInMl, scriptVar notifyDownloadsList )
{
	SCRIPT_FUNCTION_INIT;
	wa2.DownloadFile( AutoChar( GET_SCRIPT_STRING( url ) ), GET_SCRIPT_STRING( filepath ), GET_SCRIPT_BOOLEAN( storeInMl ), GET_SCRIPT_BOOLEAN( notifyDownloadsList ) );
	return MAKE_SCRIPT_VOID();
}

scriptVar SystemObject::vcpu_onDownloadFinished( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url, scriptVar success, scriptVar filename )
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS3( o, systemController, url, success, filename );
	SCRIPT_FUNCTION_CHECKABORTEVENT_SYS( o );
	SCRIPT_EXEC_EVENT3( o, url, success, filename );
}

scriptVar SystemObject::vcpu_getAlbumArt( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar filename )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_INT( !!wa2.GetAlbumArt( GET_SCRIPT_STRING( filename ) ) );
}

scriptVar SystemObject::vcpu_isWinampPro( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_INT( !!wa2.IsWinampPro() );
}

static PtrList<StringW> embed_guids;

static int embed_guid_enumProc( embedWindowState *ws, struct embedEnumStruct *param )
{
	if ( ws->flags & EMBED_FLAGS_GUID )
	{
		wchar_t guid[ 39 ] = { 0 };
		nsGUID::toCharW( GET_EMBED_GUID( ws ), guid );
		embed_guids.addItem( new StringW( guid ) );
	}
	return 0;
}
scriptVar SystemObject::vcpu_enumEmbedGUID( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _index )
{
	SCRIPT_FUNCTION_INIT;
	int index = GET_SCRIPT_INT( _index );
	if ( index == 0 ) // make the table
	{
		embed_guids.deleteAll();
		embedEnumStruct param;
		param.user_data = 0;
		param.enumProc = embed_guid_enumProc;
		SendMessageW( wa2.getMainWindow(), WM_WA_IPC, (WPARAM)&param, IPC_EMBED_ENUM );
	}
	if ( index >= 0 && index < embed_guids.getNumItems() )
		return MAKE_SCRIPT_STRING( embed_guids[ index ]->getValue() );

	return MAKE_SCRIPT_STRING( L"" );
}

scriptVar SystemObject::vcpu_getWinampVersion( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
/** This would have been the code to return a double.
	double r = atof(wa2.getVersion());
	return MAKE_SCRIPT_DOUBLE(r);
*/
	return MAKE_SCRIPT_STRING( applicationApi->main_getVersionNumString() );
}

scriptVar SystemObject::vcpu_getBuildNumber( SCRIPT_FUNCTION_PARAMS, ScriptObject *object )
{
	SCRIPT_FUNCTION_INIT;
	return MAKE_SCRIPT_INT( applicationApi->main_getBuildNumber() );
}

scriptVar SystemObject::vcpu_getFileSize( SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar _file )
{
	SCRIPT_FUNCTION_INIT;
	const wchar_t *file = GET_SCRIPT_STRING( _file );
	if ( file == NULL ) // make the table
	{
		return MAKE_SCRIPT_INT( 0 );
	}
	/*fileApi->fileOpen(file, 0);
	int size = fileApi->fileGetFileSize();
	fileApi->fileClose(;*/
	OSFILETYPE in = WFOPEN( file, WF_READONLY_BINARY );
	if ( in == OPEN_FAILED ) return MAKE_SCRIPT_INT( 0 );
	int size = (int)FGETSIZE( in );
	FCLOSE( in );
	return MAKE_SCRIPT_INT( size );
}

wchar_t SystemObject::staticStr[ 4096 ] = { 0 };
wchar_t SystemObject::translateStr[ 4096 ] = { 0 };
PtrList < ScriptObject > SystemObject::scriptobjects;

// END VCPU

// ---------------------------------------------------------------
// -- END SCRIPT -------------------------------------------------
// ---------------------------------------------------------------