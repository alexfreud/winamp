#pragma once
#include <api/skin/skinitem.h>
#include "ParamList.h"
#include <tataki/region/region.h>
class ElementRegionServer : public RegionServerI
{
public:
	ElementRegionServer(api_region *r)
			: reg(r->getOSHandle())
	{}

	virtual api_region *getRegion()
	{
		return &reg;
	}

private:
	RegionI reg;
};


struct SkinBitmapElement : public SkinItemI
{
public:

	SkinBitmapElement(const wchar_t *_id, const wchar_t *_filename, const wchar_t *_rootpath,
	                  int _x, int _y, int _w, int _h, 
	                  ifc_xmlreaderparams *pars = NULL, int script_id = -1, int secondarycounter = 0, const wchar_t *colorgrp = NULL);
	virtual ~SkinBitmapElement();

	const wchar_t *getId() { return id; }
	const wchar_t *getFilename() { return filename; }
	int getX() { return x; }
	int getY() { return y; }
	int getW() { return w; }
	int getH() { return h; }
	int getSecCount() { return seccount; }
	const wchar_t *getColorGroup() { return colorgroup; }
	ElementRegionServer *getRegionServer() { return region; }
	void setRegionServer(ElementRegionServer *s) { region = s; }

	virtual const wchar_t *getXmlRootPath() { return rootpath; }
	virtual const wchar_t *getName() { return L"bitmap"; }
	virtual ifc_xmlreaderparams *getParams() { return &params; }
	virtual int getSkinPartId() { return scriptid; }
	virtual SkinItem *getAncestor();

private:

	StringW id;
	StringW filename;
	StringW rootpath;
	int x;
	int y;
	int w;
	int h;
	int scriptid;
	int seccount;
	ParamList params;
	StringW colorgroup;
	ElementRegionServer *region;
};


class SortSkinBitmapElement
{
public:
	static int compareItem(SkinBitmapElement *p1, SkinBitmapElement *p2)
	{
		int r = WCSICMP(p1->getId(), p2->getId());
		if (!r)
		{
			if (p1->getSkinPartId() < p2->getSkinPartId()) return -1;
			if (p1->getSkinPartId() > p2->getSkinPartId()) return 1;
			if (p1->getSecCount() < p2->getSecCount()) return -1;
			if (p1->getSecCount() > p2->getSecCount()) return 1;
			return 0;
		}
		return r;
	}
	static int compareAttrib(const wchar_t *attrib, SkinBitmapElement *item)
	{
		return WCSICMP(attrib, item->getId());
	}
};
