#ifndef _LOCALESMGR_H
#define _LOCALESMGR_H

#include "../xml/obj_xml.h"
#include <bfc/ptrlist.h>
#include <bfc/string/bfcstring.h>
#include <api/skin/skinparse.h>

#include "LocalesInfo.h"

class LocTrans
{
public:
	LocTrans(const wchar_t *pfrom, const wchar_t *pto) : from(pfrom), to(pto)
	{
		//from.toupper();
	}
	const wchar_t *getFrom() { return from; }
	const wchar_t *getTo() { return to; }
private:
	StringW from;
	StringW to;
};

class PLS_LocTrans 
{
public:
	// comparator for sorting
	static int compareItem(LocTrans *p1, LocTrans* p2) {
		return wcscmp(p1->getFrom(), p2->getFrom());
	}
	// comparator for searching
	static int compareAttrib(const wchar_t *attrib, LocTrans *item) {
		return wcscmp(attrib, item->getFrom());
	}
};

class LocAccel
{
public:
	LocAccel(const wchar_t *psec, const wchar_t *pkey, const wchar_t *paction) 
		: section(psec), key(pkey), realkey(pkey), action(paction)
	{
		key.tolower();
		actionnum = SkinParser::getAction(paction);
	}
	const wchar_t *getKey() { return key; }
	const wchar_t *getRealKey() { return realkey; }
	const wchar_t *getAction() { return action; }
	int getActionNum() { return actionnum; }
	const wchar_t *getSection() { return section; }
private:
	StringW section;
	StringW key;
	StringW realkey;
	StringW action;
	int actionnum;
};

class PLS_LocAccel
{
public:
	// comparator for sorting
	static int compareItem(LocAccel *p1, LocAccel *p2)
	{
		return wcscmp(p1->getKey(), p2->getKey());
	}
	// comparator for searching
	static int compareAttrib(const wchar_t *attrib, LocAccel *item)
	{
		return wcscmp(attrib, item->getKey());
	}
};

class LocalesAcceleratorSectionXmlCallback : public ifc_xmlreadercallbackI
{
	 void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	 void xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag);
};

class LocalesAcceleratorXmlCallback : public ifc_xmlreadercallbackI
{
	 void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
};

class LocalesTranslationXmlCallback : public ifc_xmlreadercallbackI
{
	 void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
};

class StringTableXmlCallback : public ifc_xmlreadercallbackI
{
	 void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	 void xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag);
};

class StringTableEntryXmlCallback : public ifc_xmlreadercallbackI
{
	 void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
};

class LocalesSkinCallback : public SkinCallbackI
{
public:
	int skincb_onUnloading();
  int skincb_onReset();
  int skincb_onReload();
  int skincb_onBeforeLoadingElements();
  int skincb_onGuiLoaded();
  int skincb_onLoaded();

};

class LocalesManager
{
public:
	static void init();
	static void load();
	static void deinit();

	static void loadFile(const wchar_t *name);

	static void addTranslation(const wchar_t *from, const wchar_t *to);
	static const wchar_t *getTranslation(const wchar_t *from); // will return the from parameter if not found
	static const wchar_t *lookupString(const wchar_t *from); // will return the from parameter if not found

	static void setAcceleratorSection(const wchar_t *section);
	static void addAccelerator(const wchar_t *bind, const wchar_t *action);
	static void addAcceleratorFromSkin(const wchar_t *bind, const wchar_t *action);
	static const wchar_t *getBindFromAction(int action);
	static const wchar_t *translateAccelerator(const wchar_t *section, const wchar_t *key);

	//static const wchar_t *enumLoadableLocales(int num);
	//static int getNumLocales();
	//static void setNewLocaleFile(const wchar_t *name);
	//static void setNewLocaleNum(int num);
	//static int isCurrentLocaleNum(int num) { return num == curLocaleNum; }

	static void resetAll();

	static const wchar_t *getLocaleRoot();

	static void LoadStringTables();
	static void LoadStringTable(const wchar_t *filename);
	static void SetStringTable(const wchar_t *table);
	static void ResetStrings();
	static const wchar_t *GetString(const wchar_t *table, uint32_t id);
	static void AddString(const wchar_t *table, uint32_t id, const wchar_t *string);
	static void AddString(uint32_t id, const wchar_t *string);
private:
	static LocalesAcceleratorXmlCallback accelXmlCallback;
	static LocalesAcceleratorSectionXmlCallback accelSectionXmlCallback;
	static LocalesTranslationXmlCallback transXmlCallback;

	static PtrListSorted<LocTrans, PLS_LocTrans, QuickSorted<LocTrans, PLS_LocTrans> > translationsList;

	static PtrListSorted<LocAccel, PLS_LocAccel, QuickSorted<LocAccel, PLS_LocAccel> > acceleratorsList;

	static StringW localePath;

	//static PtrList<LocaleItem> loadableLocalesList;

	//static int localeListLoaded, curLocaleNum, englishLocale;
	static StringW curSection;

	static StringW curTable;

	static LocalesSkinCallback localesSkinCallback;
	static StringTableXmlCallback stringTableXmlCallback;
	static StringTableEntryXmlCallback stringTableXmlEntryCallback;
};

#endif//_LOCALESMGR_H