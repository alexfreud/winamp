#ifndef NULLSOFT_GEN_FF_SKINCOLORELEMENT_H
#define NULLSOFT_GEN_FF_SKINCOLORELEMENT_H

#include <api/skin/skinitem.h>
#include "ParamList.h"

struct SkinColorElement : public SkinItem
{
public:
	SkinColorElement(const wchar_t *_id, ARGB32 v, int script_id = -1, int secondarycounter = 0, const wchar_t *colorgrp = NULL, const wchar_t *path = NULL, ifc_xmlreaderparams *p = NULL)
			: id(_id), value(v), scriptid(script_id), seccount(secondarycounter), colorgroup(colorgrp), rootpath(path)
	{
		if (p != NULL)
		{
			for (size_t i = 0;i != p->getNbItems();i++)
				params.addItem(p->getItemName(i), p->getItemValue(i));
		}
	}

	const wchar_t *getXmlRootPath() { return rootpath; }
	const wchar_t *getName() { return L"color"; }
	ifc_xmlreaderparams *getParams() { return &params; }
	int getSkinPartId() { return scriptid; }
	SkinItem *getAncestor();
	const wchar_t *getId() { return id; }
	ARGB32 getColor() { return value; }
	ARGB32 *getColorRef() { return &value; }
	int getSecCount() { return seccount; }
	const wchar_t *getColorGroup() { return colorgroup; }

private:
	RECVS_DISPATCH;
	StringW id;
	ARGB32 value;
	int scriptid;
	int seccount;
	StringW colorgroup;
	StringW rootpath;
	ParamList params;
};

class SortSkinColorElement
{
public:
	static int compareItem(SkinColorElement *p1, SkinColorElement *p2)
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
	static int compareAttrib(const wchar_t *attrib, SkinColorElement *item)
	{
		return WCSICMP(attrib, item->getId());
	}
};


#endif
