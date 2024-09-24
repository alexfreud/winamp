#include "GammaFilter.h"
#include "api.h"

int GammaFilter::filterBitmap(uint8_t *bits, int w, int h, int bpp, const wchar_t *element_id, const wchar_t *forcegroup)
{
	int r = 0, g = 0, b = 0;
	int gray = 0;
	int boost = 0;

	const wchar_t *gammagroup;

	if (forcegroup && *forcegroup)
		gammagroup = forcegroup;
	else
		gammagroup = WASABI_API_PALETTE->getGammaGroupFromId(element_id);

	WASABI_API_COLORTHEMES->getGammaForGroup(gammagroup, &r, &g, &b, &gray, &boost);

	if (!r && !g && !b && !gray && !boost) return 1;

	if (bpp != 32) return 0;

	int *p;
	int l = w * h;
	int c;

	if (gray == 1)
	{
		int r, g, b;
		p = (int *)bits;
		while (l--)
		{
			r = (*p & 0xff0000) >> 16;
			g = (*p & 0xff00) >> 8;
			b = (*p & 0xff);
			c = MAX(MAX(r, g), b);
			c = (c << 16) | (c << 8) | c;
			*p = (*p & 0xff000000) | c;
			p++;
		}
	}

	if (gray == 2)
	{
		int r, g, b;
		p = (int *)bits;
		while (l--)
		{
			r = (*p & 0xff0000) >> 16;
			g = (*p & 0xff00) >> 8;
			b = (*p & 0xff);
			c = (r + g + b) / 3;
			c = (c << 16) | (c << 8) | c;
			*p = (*p & 0xff000000) | c;
			p++;
		}
	}

	if (boost)
	{
		l = w * h;
		int r, g, b, a;
		p = (int *)bits;
		while (l--)
		{
			a = (*p & 0xff000000) >> 25;
			r = ((*p & 0xff0000) >> 17) + a;
			g = ((*p & 0xff00) >> 9) + a;
			b = ((*p & 0xff) >> 1) + a;
			*p = (*p & 0xff000000) | (r << 16) | (g << 8) | b;
			p++;
		}
	}

	int rm = 65535 + (r << 4);
	int gm = 65535 + (g << 4);
	int bm = 65535 + (b << 4);

	l = w * h;

	p = (int *)bits;
	while (l--)
	{
		r = ((((*p & 0xff0000) >> 16) * rm)) & 0xffff0000;
		c = MAX(0, MIN(r, 0xFF0000));
		r = ((((*p & 0xff00) >> 8) * gm) >> 8) & 0xffff00;
		c |= MAX(0, MIN(r, 0xFF00));
		r = (((*p & 0xff) * bm) >> 16) & 0xffff;
		c |= MAX(0, MIN(r, 0xFF));
		c = (c & 0xFFFFFF) | (*p & 0xFF000000);
		*p = c;
		p++;
	}

	return 1;
}

ARGB32 GammaFilter::filterColor(ARGB32 color, const wchar_t *element_id, const wchar_t *forcegroup)
{
	int r = 0, g = 0, b = 0, gray = 0, boost = 0;

	const wchar_t *gammagroup;
	if (forcegroup && *forcegroup)
		gammagroup = forcegroup;
	else
		gammagroup = WASABI_API_PALETTE->getGammaGroupFromId(element_id);

	WASABI_API_COLORTHEMES->getGammaForGroup(gammagroup, &r, &g, &b, &gray, &boost);

	if (!r && !g && !b && !gray && !boost) return color;

	ARGB32 c;

	if (gray == 1)
	{
		int r, g, b;
		r = (color & 0xff0000) >> 16;
		g = (color & 0xff00) >> 8;
		b = (color & 0xff);
		c = MAX(MAX(r, g), b);
		c = (c << 16) | (c << 8) | c;
		color = (color & 0xff000000) | c;
	}

	if (gray == 2)
	{
		int r, g, b;
		r = (color & 0xff0000) >> 16;
		g = (color & 0xff00) >> 8;
		b = (color & 0xff);
		c = (r + g + b) / 3;
		c = (c << 16) | (c << 8) | c;
		color = (color & 0xff000000) | c;
	}

	if (boost)
	{
		int r, g, b;
		r = ((color & 0xff0000) >> 17) + 0x80;
		g = ((color & 0xff00) >> 9) + 0x80;
		b = ((color & 0xff) >> 1) + 0x80;
		color = (color & 0xff000000) | (r << 16) | (g << 8) | b;
	}

	int bm = 65536 + (r << 4);
	int gm = 65536 + (g << 4);
	int rm = 65536 + (b << 4);

	r = ((((color & 0xff0000) >> 16) * rm)) & 0xffff0000;
	c = MAX(0, MIN(r, 0xFF0000));
	r = ((((color & 0xff00) >> 8) * gm) >> 8) & 0xffff00;
	c |= MAX(0, MIN(r, 0xFF00));
	r = (((color & 0xff) * bm) >> 16) & 0xffff;
	c |= MAX(0, MIN(r, 0xFF));
	c = (c & 0xFFFFFF) | (color & 0xFF000000);
	return c;
}

#define CBCLASS GammaFilter
START_DISPATCH;
  CB(FILTERBITMAP, filterBitmap);
  CB(FILTERCOLOR, filterColor);
END_DISPATCH;
#undef CBCLASS


static GammaFilter gammaFilterService;
// {B092B4BD-32E1-4c35-B4AE-90B34565D9D6}
static const GUID gammaFilterGUID = 
{ 0xb092b4bd, 0x32e1, 0x4c35, { 0xb4, 0xae, 0x90, 0xb3, 0x45, 0x65, 0xd9, 0xd6 } };

FOURCC GammaFilterFactory::GetServiceType()
{
	return gammaFilterService.getServiceType();
}

const char *GammaFilterFactory::GetServiceName()
{
	return gammaFilterService.getServiceName();
}

GUID GammaFilterFactory::GetGUID()
{
	return gammaFilterGUID;
}

void *GammaFilterFactory::GetInterface(int global_lock)
{
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return &gammaFilterService;
}

int GammaFilterFactory::SupportNonLockingInterface()
{
	return 1;
}

int GammaFilterFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	return 1;
}

const char *GammaFilterFactory::GetTestString()
{
	return 0;
}

int GammaFilterFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS GammaFilterFactory
START_DISPATCH;
CB(WASERVICEFACTORY_GETSERVICETYPE, GetServiceType)
CB(WASERVICEFACTORY_GETSERVICENAME, GetServiceName)
CB(WASERVICEFACTORY_GETGUID, GetGUID)
CB(WASERVICEFACTORY_GETINTERFACE, GetInterface)
CB(WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface) 
CB(WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface)
CB(WASERVICEFACTORY_GETTESTSTRING, GetTestString)
CB(WASERVICEFACTORY_SERVICENOTIFY, ServiceNotify)
END_DISPATCH;
#undef CBCLASS