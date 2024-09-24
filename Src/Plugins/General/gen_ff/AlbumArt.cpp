#include "precomp__gen_ff.h"

#include <api/core/api_core.h>
#include "main.h"
#include "AlbumArt.h"
#include "wa2frontend.h"
#include <api.h>
#include <tataki/bitmap/bitmap.h>
#include <api\wnd\notifmsg.h>
#include <api/script/scriptmgr.h>
#include <api/script/script.h>

#define ALBUMART_MAX_THREADS 4

const wchar_t albumArtXuiObjectStr[] = L"AlbumArt"; // This is the xml tag
char albumArtXuiSvcName[] = "Album Art XUI object"; // this is the name of the xuiservice

AlbumArtScriptController _albumartController;
AlbumArtScriptController *albumartController = &_albumartController;

BEGIN_SERVICES( wa2AlbumArt_Svcs );
DECLARE_SERVICE( XuiObjectCreator<AlbumArtXuiSvc> );
END_SERVICES( wa2AlbumArt_Svcs, _wa2AlbumArt_Svcs );

// --------------------------------------------------------
// Maki Script Object
// --------------------------------------------------------

// -- Functions table -------------------------------------
function_descriptor_struct AlbumArtScriptController::exportedFunction[] = {
	{L"refresh", 0, (void *)AlbumArt::script_vcpu_refresh },
	{L"onAlbumArtLoaded", 1, (void *)AlbumArt::script_vcpu_onAlbumArtLoaded },
	{L"isLoading", 0, (void *)AlbumArt::script_vcpu_isLoading },
};

const wchar_t *AlbumArtScriptController::getClassName()
{
	return L"AlbumArtLayer";
}

const wchar_t *AlbumArtScriptController::getAncestorClassName()
{
	return L"Layer";
}

ScriptObject *AlbumArtScriptController::instantiate()
{
	AlbumArt *a = new AlbumArt;

	ASSERT( a != NULL );

	return a->getScriptObject();
}

void AlbumArtScriptController::destroy( ScriptObject *o )
{
	AlbumArt *a = static_cast<AlbumArt *>( o->vcpu_getInterface( albumArtGuid ) );

	ASSERT( a != NULL );

	delete a;
}

void *AlbumArtScriptController::encapsulate( ScriptObject *o )
{
	return NULL; // no encapsulation yet
}

void AlbumArtScriptController::deencapsulate( void *o )
{}

int AlbumArtScriptController::getNumFunctions()
{
	return sizeof( exportedFunction ) / sizeof( function_descriptor_struct );
}

const function_descriptor_struct *AlbumArtScriptController::getExportedFunctions()
{
	return exportedFunction;
}

GUID AlbumArtScriptController::getClassGuid()
{
	return albumArtGuid;
}


XMLParamPair AlbumArt::params[] =
{
	{ALBUMART_NOTFOUNDIMAGE, L"NOTFOUNDIMAGE"},
	{ALBUMART_SOURCE,        L"SOURCE"},
	{ALBUMART_VALIGN,        L"VALIGN"},
	{ALBUMART_ALIGN,         L"ALIGN"},
	{ALBUMART_STRETCHED,     L"STRETCHED"},
	{ALBUMART_NOREFRESH,     L"NOAUTOREFRESH"},
};

class AlbumArtThreadContext
{
	public:
	AlbumArtThreadContext( const wchar_t *_filename, AlbumArt *_wnd )
	{
		/* lazy load these two handles */
		if ( !_wnd->hMainThread )
			_wnd->hMainThread = WASABI_API_APP->main_getMainThreadHandle();

		if ( !_wnd->thread_semaphore )
			_wnd->thread_semaphore = CreateSemaphore( 0, ALBUMART_MAX_THREADS, ALBUMART_MAX_THREADS, 0 );

		wnd      = _wnd;
		iterator = wnd->iterator;
		h        = w = 0;
		bits     = 0;
		filename = _wcsdup( _filename );
	}

	void FreeBits()
	{
		if ( bits )
			WASABI_API_MEMMGR->sysFree( bits );

		bits = 0;
	}

	~AlbumArtThreadContext()
	{
		if ( wnd )
		{
			if ( wnd->thread_semaphore )
				ReleaseSemaphore( wnd->thread_semaphore, 1, 0 );

			wnd->isLoading--;
		}

		free( filename );
	}

	static void CALLBACK AlbumArtNotifyAPC( ULONG_PTR p );
	bool LoadArt();

	int       h;
	int       w;
	ARGB32   *bits;
	LONG      iterator;
	wchar_t  *filename;
	AlbumArt *wnd;
};


AlbumArt::AlbumArt()
{
	getScriptObject()->vcpu_setInterface( albumArtGuid, ( void * )static_cast<AlbumArt *>( this ) );
	getScriptObject()->vcpu_setClassName( L"AlbumArtLayer" );
	getScriptObject()->vcpu_setController( albumartController );

	WASABI_API_MEDIACORE->core_addCallback( 0, this );

	w                 = 0;
	h                 = 0;
	iterator          = 0;
	bits              = 0;
	hMainThread       = 0;
	thread_semaphore  = 0;
	artBitmap         = 0;
	valign            = 0;
	align             = 0;
	stretched         = false;
	missing_art_image = L"winamp.cover.notfound"; // default to this.
	src_file          = L"";
	forceRefresh      = false;
	noAutoRefresh     = false;
	noMakiCallback    = false;
	isLoading         = 0;

	/* register XML parameters */
	xuihandle = newXuiHandle();

	CreateXMLParameters( xuihandle );
}

void AlbumArt::CreateXMLParameters( int master_handle )
{
	//ALBUMART_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof( params ) / sizeof( params[ 0 ] );
	hintNumberOfParams( xuihandle, numParams );

	for ( int i = 0; i < numParams; i++ )
		addParam( xuihandle, params[ i ], XUI_ATTRIBUTE_IMPLIED );
}


AlbumArt::~AlbumArt()
{
	WASABI_API_SYSCB->syscb_deregisterCallback( static_cast<MetadataCallbackI *>( this ) );
	WASABI_API_MEDIACORE->core_delCallback( 0, this );

	// wait for all of our threads to finish
	InterlockedIncrement( &iterator ); // our kill switch (will invalidate iterator on all outstanding threads)
	if ( thread_semaphore )
	{
		for ( int i = 0; i < ALBUMART_MAX_THREADS; i++ )
		{
			if ( WaitForMultipleObjectsEx( 1, &thread_semaphore, FALSE, INFINITE, TRUE ) != WAIT_OBJECT_0 )
				i--;
		}
	}

	delete artBitmap;

	if ( bits )
		WASABI_API_MEMMGR->sysFree( bits );

	if ( thread_semaphore )
		CloseHandle( thread_semaphore );

	if ( hMainThread )
		CloseHandle( hMainThread );
}

bool AlbumArt::layer_isInvalid()
{
	return !bits;
}

void CALLBACK AlbumArtThreadContext::AlbumArtNotifyAPC( ULONG_PTR p )
{
	AlbumArtThreadContext *context = (AlbumArtThreadContext *)p;
	if ( context->wnd->iterator == context->iterator )
		context->wnd->ArtLoaded( context->w, context->h, context->bits );
	else
		context->FreeBits();

	delete context;
}

bool AlbumArtThreadContext::LoadArt()
{
	if ( wnd->iterator != iterator )
		return false;

	if ( AGAVE_API_ALBUMART->GetAlbumArt( filename, L"cover", &w, &h, &bits ) != ALBUMART_SUCCESS )
	{
		bits = 0;
		w    = 0;
		h    = 0;
	}

	if ( wnd->iterator == iterator ) // make sure we're still valid
	{
		QueueUserAPC( AlbumArtNotifyAPC, wnd->hMainThread, (ULONG_PTR)this );

		return true;
	}
	else
	{
		FreeBits();

		return false;
	}
}

static int AlbumArtThreadPoolFunc( HANDLE handle, void *user_data, intptr_t id )
{
	AlbumArtThreadContext *context = (AlbumArtThreadContext *)user_data;
	if ( context->LoadArt() == false )
	{
		delete context;
	}

	return 0;
}

void AlbumArt::ArtLoaded( int _w, int _h, ARGB32 *_bits )
{
	if ( bits )
		WASABI_API_MEMMGR->sysFree( bits );

	if ( artBitmap )
	{
		delete artBitmap;
		artBitmap = 0;
	}

	bits = _bits;
	w    = _w;
	h    = _h;

	if ( !bits )
	{
		SkinBitmap *albumart = missing_art_image.getBitmap();
		if ( albumart )
		{
			w = albumart->getWidth();
			h = albumart->getHeight();
		}
	}

	deleteRegion();
	makeRegion();
	notifyParent( ChildNotify::AUTOWHCHANGED );
	invalidate();

	// notify maki scripts that albumart has been found or not
	if ( !noMakiCallback && _bits )
	{
		AlbumArt::script_vcpu_onAlbumArtLoaded( SCRIPT_CALL, this->getScriptObject(), MAKE_SCRIPT_BOOLEAN( (int)bits ) );
	}
}

void AlbumArt::onSetVisible( int show )
{
	if ( show )
	{
		corecb_onUrlChange( wa2.GetCurrentFile() );
	}
	else
	{
		if ( bits )
		{
			WASABI_API_MEMMGR->sysFree( bits );

			delete artBitmap;
			artBitmap = 0;
		}

		bits = 0;
	}
}

int AlbumArt::corecb_onUrlChange( const wchar_t *filename )
{
	// Martin> if we call this from maki we want to do a refresh regardless of the albumartlayer being visible or not
	if ( forceRefresh || ( !noAutoRefresh && isVisible() ) )
	{
		isLoading++;

		// Martin > do a check for a specific file, defined via source param
		if ( WCSICMP( src_file, L"" ) )
			filename = src_file;

		InterlockedIncrement( &iterator );
		AlbumArtThreadContext *context = new AlbumArtThreadContext( filename, this );

		// make sure we have an available thread free (wait for one if we don't)
		while ( WaitForMultipleObjectsEx( 1, &thread_semaphore, FALSE, INFINITE, TRUE ) != WAIT_OBJECT_0 )
		{}

		//	int vis__ = isVisible();
		WASABI_API_THREADPOOL->RunFunction( 0, AlbumArtThreadPoolFunc, context, 0, api_threadpool::FLAG_LONG_EXECUTION );
	}

	return 1;
}

int AlbumArt::onInit()
{
	int r = ALBUMART_PARENT::onInit();
	WASABI_API_SYSCB->syscb_registerCallback( static_cast<MetadataCallbackI *>( this ) );

	AlbumArt::corecb_onUrlChange( wa2.GetCurrentFile() );

	return r;
}

int AlbumArt::skincb_onColorThemeChanged( const wchar_t *newcolortheme )
{
	ALBUMART_PARENT::skincb_onColorThemeChanged( newcolortheme );
	invalidate();

	return 0;
}

SkinBitmap *AlbumArt::getBitmap()
{
	if ( artBitmap )
		return artBitmap;

	if ( bits )
	{
		artBitmap = new HQSkinBitmap( bits, w, h );  //TH WDP2-212

		return artBitmap;
	}

	return missing_art_image.getBitmap();
}

void AlbumArt::layer_adjustDest( RECT *r )
{
	if ( !w || !h )
		return;

	if ( stretched )
		return;

	//getClientRect(r);
	// maintain 'square' stretching
	int dstW    = r->right - r->left;
	int dstH    = r->bottom - r->top;
	double aspX = (double)( dstW ) / (double)w;
	double aspY = (double)( dstH ) / (double)h;
	double asp  = min( aspX, aspY );
	int newW    = (int)( w * asp );
	int newH    = (int)( h * asp );

	// Align
	int offsetX = ( dstW - newW ) / 2;
	if ( align == 1 )
		offsetX *= 2;
	else if ( align == -1 )
		offsetX = 0;

	// Valign
	int offsetY = ( dstH - newH ) / 2;
	if ( valign == 1 )
		offsetY *= 2;
	else if ( valign == -1 )
		offsetY = 0;

	r->left   += offsetX;
	r->right   = r->left + newW;
	r->top    += offsetY;
	r->bottom  = r->top + newH;

	// This prevents parts of the image being cut off (if the img has the same dimensions as the rect) on moving/clicking winamp
	// (they will just flicker, but at least they won't stay now)
	// benski> CUT!!! no no no no no this is very bad because this gets called inside layer::onPaint  
	//invalidate();
}
/*
int AlbumArt::getWidth()
{
	RECT r;
	getClientRect(&r);
	getDest(&r);
	return r.right-r.left;
}

int AlbumArt::getHeight()
{
	RECT r;
	getClientRect(&r);
	getDest(&r);
	return r.bottom-r.top;
}
*/
/*
int AlbumArt::onPaint(Canvas *canvas)
{
	ALBUMART_PARENT::onPaint(canvas);
	if (bits)
	{
		SkinBitmap albumart(bits, w, h);
		RECT dst;
		getBufferPaintDest(&dst);
		albumart.stretchToRectAlpha(canvas, &dst, getPaintingAlpha());
	}
	return 1;
}
*/
int AlbumArt::setXuiParam( int _xuihandle, int attrid, const wchar_t *name, const wchar_t *strval )
{
	if ( xuihandle != _xuihandle )
		return ALBUMART_PARENT::setXuiParam( _xuihandle, attrid, name, strval );

	switch ( attrid )
	{
		case ALBUMART_NOTFOUNDIMAGE:
			missing_art_image = strval;
			if ( !bits )
			{
				noMakiCallback = true;
				ArtLoaded( 0, 0, 0 );
				noMakiCallback = false;
			}
			break;
		case ALBUMART_SOURCE:
			src_file = strval;
			AlbumArt::corecb_onUrlChange( wa2.GetCurrentFile() ); // This Param should _always_ hold our current file
			break;
		case ALBUMART_VALIGN:
			if ( !WCSICMP( strval, L"top" ) )
				valign = -1;
			else if ( !WCSICMP( strval, L"bottom" ) )
				valign = 1;
			else
				valign = 0;

			deferedInvalidate();
			break;
		case ALBUMART_ALIGN:
			if ( !WCSICMP( strval, L"left" ) )
				align = -1;
			else if ( !WCSICMP( strval, L"right" ) )
				align = 1;
			else
				align = 0;

			deferedInvalidate();
			break;
		case ALBUMART_STRETCHED:
			if ( !WCSICMP( strval, L"0" ) || !WCSICMP( strval, L"" ) )
				stretched = false;
			else
				stretched = true;

			deferedInvalidate();
			break;
		case ALBUMART_NOREFRESH:
			if ( !WCSICMP( strval, L"0" ) || !WCSICMP( strval, L"" ) )
				noAutoRefresh = false;
			else
				noAutoRefresh = true;
			break;
		default:
			return 0;
	}
	return 1;
}

void AlbumArt::metacb_ArtUpdated( const wchar_t *filename )
{
	// it'd be nice to do this, but we can't guarantee that our file didn't get updated in the process
	//const wchar_t *curFn = wa2.GetCurrentFile();
//	if (curFn && filename && *filename && *curFn && !_wcsicmp(filename, curFn))
	AlbumArt::corecb_onUrlChange( wa2.GetCurrentFile() );
}


scriptVar AlbumArt::script_vcpu_refresh( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
	AlbumArt *a = static_cast<AlbumArt *>( o->vcpu_getInterface( albumArtGuid ) );
	if ( a )
	{
		a->forceRefresh = true;
		a->corecb_onUrlChange( wa2.GetCurrentFile() );
		a->forceRefresh = false;
	}
	RETURN_SCRIPT_VOID;
}

scriptVar AlbumArt::script_vcpu_isLoading( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
	AlbumArt *a = static_cast<AlbumArt *>( o->vcpu_getInterface( albumArtGuid ) );
	if ( a )
	{
		return MAKE_SCRIPT_BOOLEAN( !!a->isLoading );
	}

	return MAKE_SCRIPT_BOOLEAN( false );
}

scriptVar AlbumArt::script_vcpu_onAlbumArtLoaded( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar success )
{
	SCRIPT_FUNCTION_INIT
		PROCESS_HOOKS1( o, albumartController, success );
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT1( o, success );
}