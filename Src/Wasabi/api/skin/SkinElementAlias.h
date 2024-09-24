#pragma once
#include <api/skin/skinitem.h>
#include <api/xml/xmlparamsi.h>
struct SkinElementAlias : public SkinItemI
{
public:

	SkinElementAlias(const wchar_t *_aliasname, const wchar_t *_idtarget, int _scriptid = -1, int _secondarycounter = 0)
			: aliasname(_aliasname), idtarget(_idtarget), scriptid(_scriptid), seccount(_secondarycounter) //, rootpath(path)
	{
		params = NULL;
		/*
		if (p != NULL) {
		  params = new XmlReaderParamsI();
		  for (int i=0;i<p->getNbItems();i++) {
		    params->addItem(p->getItemName(i), p->getItemValue(i));
		  }
		}
		*/
	}
	virtual ~SkinElementAlias()
	{
		delete params;
	}

	const wchar_t *getAliasName() { return aliasname; }
	const wchar_t *getTargetId() { return idtarget; }
	int getSecCount() { return seccount; }

	virtual const wchar_t *getXmlRootPath() { return rootpath; }
	virtual const wchar_t *getName() { return L"elementalias"; }
	virtual ifc_xmlreaderparams *getParams() { return params; }
	virtual int getSkinPartId() { return scriptid; }
	virtual SkinItem *getAncestor();

private:
	StringW aliasname;
	StringW idtarget;
	int scriptid;
	int seccount;
	XmlReaderParamsI *params;
	StringW rootpath;
};

class SortSkinElementAlias
{
public:
	static int compareItem(SkinElementAlias *p1, SkinElementAlias *p2)
	{
		int r = WCSICMP(p1->getAliasName(), p2->getAliasName());
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
	static int compareAttrib(const wchar_t *attrib, SkinElementAlias *item)
	{
		return WCSICMP(attrib, item->getAliasName());
	}
};