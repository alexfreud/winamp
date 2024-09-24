#pragma once

#include <api/wnd/cursor.h>
#include <api/skin/skinitem.h>
#include "ParamList.h"
#include <api/skin/api_skin.h> // for OSCURSOR, probably should put this in a better place

class SkinCursorElement : public SkinItemI
{
	friend class SortSkinCursorElement;
public:
	SkinCursorElement(const wchar_t *_id, const wchar_t *_bitmapid, int _x, int _y, int script_id = -1, int secondarycounter = 0, const wchar_t *path = NULL, ifc_xmlreaderparams *params = NULL);
	virtual ~SkinCursorElement();

	const wchar_t *getId() { return id; }
	const wchar_t *getBitmapId() { return bitmap; }
	int getHotspotX() { return x; }
	int getHotspotY() { return y; }
	int getScriptId() { return scriptid; }
	int getSecId() { return seccount; }

	virtual OSCURSOR getCursor();

	virtual const wchar_t *getXmlRootPath() { return rootpath; }
	virtual const wchar_t *getName() { return L"cursor"; }
	virtual ifc_xmlreaderparams *getParams() { return &params; }
	virtual int getSkinPartId() { return scriptid; }
	virtual SkinItem *getAncestor();

private:

	void makeCursor();

	StringW id;
	StringW bitmap;
	StringW rootpath;
	ParamList params;
	int x;
	int y;
	int scriptid;
	int seccount;
	OSCURSOR icon;
};

class SortSkinCursorElement
{
public:
	static int compareItem(SkinCursorElement *p1, SkinCursorElement *p2)
	{
		int r = WCSICMP(p1->id, p2->id);
		if (!r)
		{
			if (p1->scriptid < p2->scriptid) return -1;
			if (p1->scriptid > p2->scriptid) return 1;
			if (p1->seccount < p2->seccount) return -1;
			if (p1->seccount > p2->seccount) return 1;
			return 0;
		}
		return r;
	}
	static int compareAttrib(const wchar_t *attrib, SkinCursorElement *item)
	{
		return WCSICMP(attrib, item->id);
	}
};
