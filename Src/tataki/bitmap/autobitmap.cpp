#include <tataki/api__tataki.h>
#include "autobitmap.h"
#include <bfc/assert.h>
#define TIMER_ID_RESET 0x1664

#ifdef DROP_BITMAP_ON_IDLE
// these are in seconds
#define DROP_MINDELAY 3
#define DROP_MAXDELAY 15
#define DROP_INITIALBUMP -5
#define DROP_MINDELAYSINCELASTUSE 7
#endif

/*
#ifdef _WIN32
extern HINSTANCE hInstance;
#endif
*/

AutoSkinBitmap::AutoSkinBitmap(const wchar_t *_name)
{
	bitmap = NULL;
	use = 0;
	id = 0;
	colorgroup = 0;
	name = 0;
	resamplingMode = RESAMPLING_MODE_NONE;
#ifdef WIN32
	myInstance = 0;
#endif
#ifdef DROP_BITMAP_ON_IDLE
	lastuse = 0;
#endif
	setBitmap(_name);
}

AutoSkinBitmap::~AutoSkinBitmap()
{
#ifdef DROP_BITMAP_ON_IDLE
	timerclient_killTimer(TIMER_ID_RESET);
#endif
	if (bitmap) bitmap->Release();
	ASSERT(WASABI_API_SYSCB != NULL);
	WASABI_API_SYSCB->syscb_deregisterCallback(this);
	free(colorgroup);
	free(name);
}

const wchar_t *AutoSkinBitmap::setBitmap(const wchar_t *_name)
{
	if (_name == NULL) return NULL;
	if (name == NULL || wcscmp(name, _name))
	{
		reset();
		free(name);
		name = _wcsdup(_name);
	}
	return name;
}

int AutoSkinBitmap::setBitmap(int _id)
{
	if (_id == 0) return 0;
	if (_id != id)
	{
		reset();
		id = _id;
	}
	return id;
}

#ifdef _WIN32
void AutoSkinBitmap::setHInstance(HINSTANCE hinstance)
{
	myInstance = hinstance;
}
#endif

void AutoSkinBitmap::reset()
{
	if (bitmap) bitmap->Release(); bitmap = NULL;
#ifdef DROP_BITMAP_ON_IDLE
	timerclient_killTimer(TIMER_ID_RESET);
#endif
}
static int modrandom(int max) 
{
  int ret = AGAVE_API_RANDOM->GetPositiveNumber();
  ret %= max;
  return ret;
}
SkinBitmap *AutoSkinBitmap::getBitmap()
{
	//FG  ASSERT(name != NULL);
	if ((name == NULL || (name != NULL && *name == NULL)) && id == NULL) return NULL;
	if (bitmap == NULL)
	{
		if (name)
		{
			switch (resamplingMode)
			{
			case RESAMPLING_MODE_SUPERSAMPLING:
				bitmap = new HQSkinBitmap(name);
				break;
			case RESAMPLING_MODE_NONE:
			default:
				bitmap = new SkinBitmap(name);
				break;
			}
		}
		else
		{
#ifdef WIN32
			switch (resamplingMode)
			{
			case RESAMPLING_MODE_SUPERSAMPLING:
				bitmap = new HQSkinBitmap(myInstance, id, colorgroup);
				break;
			case RESAMPLING_MODE_NONE:
			default:
				bitmap = new SkinBitmap(myInstance, id, colorgroup);
				break;
			}
#endif
		}
		ASSERT(WASABI_API_SYSCB != NULL);
		if (bitmap)
			WASABI_API_SYSCB->syscb_registerCallback(this);
#ifdef DROP_BITMAP_ON_IDLE
		if (bitmap)
		{
			lastuse = GetTickCount() + DROP_INITIALBUMP;
			timerclient_setTimer(TIMER_ID_RESET, DROP_MINDELAY*1000 + modrandom((DROP_MAXDELAY - DROP_MINDELAY)*1000));
			return bitmap;
		}
#endif

	}
#ifdef DROP_BITMAP_ON_IDLE
	if (bitmap) lastuse = GetTickCount();
#endif
	return bitmap;
}

const wchar_t *AutoSkinBitmap::getBitmapName()
{
	return name;
}

int AutoSkinBitmap::skincb_onReset()
{
	reset();
	return 1;
}

void AutoSkinBitmap::setHInstanceBitmapColorGroup(const wchar_t *_colorgroup) 
{ 
	free(colorgroup);
	if (_colorgroup)
		colorgroup = _wcsdup(_colorgroup);
	else
		colorgroup=0;
}

#ifdef DROP_BITMAP_ON_IDLE
void AutoSkinBitmap::timerclient_timerCallback(int id)
{
	if (id == TIMER_ID_RESET)
	{
		tryUnload();
	}
	else TimerClientDI::timerclient_timerCallback(id);
}

void AutoSkinBitmap::tryUnload()
{
	DWORD now = GetTickCount();
	if (now < lastuse + DROP_MINDELAYSINCELASTUSE*1000) return ;
	reset();
}
#endif

void AutoSkinBitmap::setResamplingMode(int mode)
{
	this->resamplingMode = mode;
	this->reset();
}

int AutoSkinBitmap::getResamplingMode()
{
	return this->resamplingMode;
}


#define CBCLASS AutoSkinBitmap
START_DISPATCH;
  CB(SYSCALLBACK_GETEVENTTYPE, getEventType);
  CB(SYSCALLBACK_NOTIFY, notify);
END_DISPATCH;
#undef CBCLASS

