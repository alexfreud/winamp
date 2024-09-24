#include <precomp.h>
#include <wasabicfg.h>
#include "localesmgr.h"
#include <bfc/parse/pathparse.h>
#include <api/config/items/cfgitem.h>
#include <bfc/file/readdir.h>
#include <api/xml/XMLAutoInclude.h>
#include "../nu/regexp.h"
#include "../Agave/language/api_language.h"
#include <bfc/ptrlist.h>

struct StringEntry
{
	uint32_t id;
	wchar_t *string;
};

typedef PtrList<StringEntry> StringTable;
struct StringTableData
{
	wchar_t *id;
	StringTable entries;
};

typedef PtrList<StringTableData> StringTables;

StringTables stringTables;

void LocalesAcceleratorSectionXmlCallback::xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	const wchar_t *section = params->getItemValue(L"section");
	if (!section)
		LocalesManager::setAcceleratorSection(L"general");
	else
		LocalesManager::setAcceleratorSection(section);
}

void LocalesAcceleratorSectionXmlCallback::xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	LocalesManager::setAcceleratorSection(L"");
}

void StringTableXmlCallback::xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	const wchar_t *section = params->getItemValue(L"id");
	if (!section)
		LocalesManager::SetStringTable(L"nullsoft.wasabi");
	else
		LocalesManager::SetStringTable(section);
}

void StringTableXmlCallback::xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	LocalesManager::SetStringTable(L"");
}

void StringTableEntryXmlCallback::xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	const wchar_t *b = params->getItemValue(L"id");
	const wchar_t *a = params->getItemValue(L"string");
	if (b && a) 
		LocalesManager::AddString(WTOI(b), a);
}

/* ------------------------------------------------------------ */
void LocalesAcceleratorXmlCallback::xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	const wchar_t *b = params->getItemValue(L"bind");
	const wchar_t *a = params->getItemValue(L"action");
	if (b && a) 
		LocalesManager::addAccelerator(b, a);
}

/* ------------------------------------------------------------ */
void LocalesTranslationXmlCallback::xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	const wchar_t *f = params->getItemValue(L"from");
	const wchar_t *t = params->getItemValue(L"to");
	if (f && t) 
		LocalesManager::addTranslation(f, t);
}

int LocalesSkinCallback::skincb_onUnloading()
{
	LocalesManager::ResetStrings();
	LocalesManager::resetAll();
	return 0;
}

int LocalesSkinCallback::skincb_onReset()
{
	//
	return 0;
}

int LocalesSkinCallback::skincb_onReload()
{
	 //LocalesManager::load();
	return 0;
}

int LocalesSkinCallback::skincb_onBeforeLoadingElements()
{
	LocalesManager::load();
	return 0;
}

int LocalesSkinCallback::skincb_onGuiLoaded()
{
	LocalesManager::LoadStringTables();
	return 0;
}

int LocalesSkinCallback::skincb_onLoaded()
{
	// TODO: load string table?
	return 0;
}

/* ------------------------------------------------------------ */
void LocalesManager::init()
{
	WASABI_API_SYSCB->syscb_registerCallback(&localesSkinCallback);
	//load();
}

void LocalesManager::load()
{
#ifdef LOCALES_CUSTOM_LOAD
		
	LOCALES_CUSTOM_LOAD(localePath);
	// TODO: benski> don't load this here.  we should set up a syscallback for skin load (maybe skincb_onBeforeLoadingElements?)
	// and also we should call deinit() on skin unload (skincb_onReload, skincb_onReset, skincb_onUnloading?)
	StringPathCombine filetoload(localePath, L"Wasabi.xml");
	/*
	PathParserW pp(localeName);
	localeName = pp.getLastString();
	int p = localeName.lFindChar('.');
	if (p > 0) localeName.trunc(p);
	*/
#else
	wchar_t tmpbuf[WA_MAX_PATH] = L"english";

	WASABI_API_CONFIG->getStringPrivate(L"LocaleFile", tmpbuf, WA_MAX_PATH, L"english");

	// FG> ok I have no idea why it doesn't work when i read from cfg instead of stringprivate and frankly i don't have time for this
	// so for now it'll work with both a cfgitem and a stringprivate, i couldn't desync them so it should be ok in the meantime.
	/*  const GUID options_guid =
	{ 0x280876cf, 0x48c0, 0x40bc, { 0x8e, 0x86, 0x73, 0xce, 0x6b, 0xb4, 0x62, 0xe5 } };
	CfgItem *item = WASABI_API_CONFIG->config_getCfgItemByGuid(options_guid);
	if (item != NULL)
	item->getData("Language", tmpbuf, WA_MAX_PATH-1);*/

	localeName = tmpbuf;
	englishLocale = !WCSICMP(localeName, L"english");
	loadFile(L"english");
	StringW filetoload = localeName;
	if (englishLocale)
		filetoload.trunc(0);
#endif

	if (!filetoload.isempty()) 
		loadFile(filetoload);

#ifdef WASABI_API_WNDMGR
	if (WASABI_API_WNDMGR)
		WASABI_API_WNDMGR->wndTrackInvalidateAll();
#endif
}

void LocalesManager::deinit()
{
	resetAll();
	WASABI_API_SYSCB->syscb_deregisterCallback(&localesSkinCallback);
}

void LoadXmlFile(obj_xml *parser, const wchar_t *filename);

void LocalesManager::LoadStringTables()
{
	StringPathCombine genericStringTable(localePath, L"stringtable.xml");
	LoadStringTable(genericStringTable);

	StringPathCombine skinStringTable(localePath, WASABI_API_SKIN->getSkinName());
	skinStringTable.AppendPath(L"stringtable.xml");
	LoadStringTable(skinStringTable);
}

void LocalesManager::LoadStringTable(const wchar_t *name)
{
	waServiceFactory *parserFactory = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
	{
		obj_xml *parser = (obj_xml *)parserFactory->getInterface();

		if (parser)
		{
			{
				const wchar_t *file = Wasabi::Std::filename(name);
				int fnlen = wcslen(file);
				StringW path = name;
				path.trunc( -fnlen);
				XMLAutoInclude include(parser, path);
				parser->xmlreader_registerCallback(L"WinampLocaleDefinition\fStringTable", &stringTableXmlCallback);
				parser->xmlreader_registerCallback(L"WinampLocaleDefinition\fStringTable\fStringEntry", &stringTableXmlEntryCallback);
				parser->xmlreader_open();
#ifdef LOCALES_CUSTOM_LOAD
				LoadXmlFile(parser, name);
#else
				LoadXmlFile(parser, StringPrintfW(L"Locales/%s.xml", name));
#endif
				parser->xmlreader_unregisterCallback(&stringTableXmlCallback);
				parser->xmlreader_unregisterCallback(&stringTableXmlEntryCallback);
			}
			parser->xmlreader_close();
			parserFactory->releaseInterface(parser);
			parser = 0;
		}
	}
}

void LocalesManager::loadFile(const wchar_t *name)
{
	waServiceFactory *parserFactory = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
	{
		obj_xml *parser = (obj_xml *)parserFactory->getInterface();

		if (parser)
		{
			{
				const wchar_t *file = Wasabi::Std::filename(name);
				int fnlen = wcslen(file);
				StringW path = name;
				path.trunc( -fnlen);
				XMLAutoInclude include(parser, path);
				parser->xmlreader_registerCallback(L"WinampLocaleDefinition\faccelerators", &accelSectionXmlCallback);
				parser->xmlreader_registerCallback(L"WinampLocaleDefinition\faccelerators\faccelerator", &accelXmlCallback);
				parser->xmlreader_registerCallback(L"WinampLocaleDefinition\ftranslations\ftranslation", &transXmlCallback);
				parser->xmlreader_open();
#ifdef LOCALES_CUSTOM_LOAD
				LoadXmlFile(parser, name);
#else
				LoadXmlFile(parser, StringPrintfW(L"Locales/%s.xml", name));
#endif
				parser->xmlreader_unregisterCallback(&accelSectionXmlCallback);
				parser->xmlreader_unregisterCallback(&accelXmlCallback);
				parser->xmlreader_unregisterCallback(&transXmlCallback);
			}
			parser->xmlreader_close();
			parserFactory->releaseInterface(parser);
			parser = 0;
		}
	}
}

void LocalesManager::addTranslation(const wchar_t *from, const wchar_t *to)
{
	LocTrans *t = translationsList.findItem(from);
	if (t)
		translationsList.removeItem(t);
	translationsList.addItem(new LocTrans(from, to));
}


const wchar_t *LocalesManager::lookupString(const wchar_t *from)
{
	if (from == NULL) 
		return NULL;

	if (*from == L'@')
	{

		const wchar_t *findPound = wcschr(from, L'#');
		if (findPound && (findPound-from) < 128)
		{
			wchar_t table[128] = {0};
			memcpy(table, from+1, sizeof(wchar_t)*(findPound-from-1));
			table[findPound-from-1]=0;
			const wchar_t *string = GetString(table, WTOI(findPound+1));
			if (string)
				return string;
		}
	}
	return from;
}

const wchar_t *LocalesManager::getTranslation(const wchar_t *from)
{
	if (!from)
		return NULL;

	LocTrans *t = translationsList.findItem(from);
	if (t == NULL)
	{
		return from;
	}
	return t->getTo();
}

void LocalesManager::addAccelerator(const wchar_t *bind, const wchar_t *action)
{
	LocAccel *a = acceleratorsList.findItem(bind);
	if (a) // Martin> shouldn't we also check here if it is the same section?
			// Hm, now that i look closer, we search our list for a wchar_t but we store LocAccels in the list - does this work?
		acceleratorsList.removeItem(a);
	acceleratorsList.addItem(new LocAccel(curSection, bind, action));
}

void LocalesManager::addAcceleratorFromSkin(const wchar_t *bind, const wchar_t *action)
{
	//TODO> use binary search
	int l = acceleratorsList.getNumItems();
	for (int i = 0;i < l;i++)
	{
		if (0 == WCSICMP(acceleratorsList[i]->getSection(), curSection) &&
			0 == WCSICMP(acceleratorsList[i]->getKey(), bind))
			return;
	}
	acceleratorsList.addItem(new LocAccel(curSection, bind, action));
}

const wchar_t *LocalesManager::getBindFromAction(int action)
{
	//TODO> use binary search
	int l = acceleratorsList.getNumItems();
	for (int i = 0;i < l;i++)
	{
		if (acceleratorsList[i]->getActionNum() == action && action != ACTION_NONE)
			return acceleratorsList[i]->getRealKey();
	}
	return NULL;
}

const wchar_t *LocalesManager::translateAccelerator(const wchar_t *section, const wchar_t *key)
{
	//TODO> use binary search
	int l = acceleratorsList.getNumItems();
	for (int i = 0;i < l;i++)
	{
		if (!WCSICMP(acceleratorsList[i]->getSection(), section))
			if (!WCSICMP(acceleratorsList[i]->getKey(), key))
				return acceleratorsList[i]->getAction();
	}
	return NULL;
}

#if 0
void LocalesManager::setNewLocaleFile(const wchar_t *name)
{
	//WASABI_API_CONFIG->setStringPrivate(L"LocaleFile", name);
	resetAll();
	init();
}
#endif

void LocalesManager::resetAll()
{
	translationsList.deleteAll();
	acceleratorsList.deleteAll();
}

void LocalesManager::SetStringTable(const wchar_t *table)
{
	curTable = table;
}

void LocalesManager::setAcceleratorSection(const wchar_t *section)
{
	curSection = section;
}

const wchar_t *LocalesManager::getLocaleRoot()
{
	return localePath;
}

void LocalesManager::ResetStrings()
{
	for (int i=0;i!=stringTables.getNumItems();i++)
	{
		FREE(stringTables[i]->id);
		for (int j=0;j!=stringTables[i]->entries.getNumItems();j++)
		{
			FREE(stringTables[i]->entries[j]->string);
		}
	}
	stringTables.removeAll();
}

const wchar_t *LocalesManager::GetString(const wchar_t *table, uint32_t id)
{
	if (!table)
		return 0;

	if (!_wcsicmp(table, L"gen_ff"))
		return WASABI_API_LNGSTRINGW(id);

	for (int i=0;i!=stringTables.getNumItems();i++)
	{
		if (!wcscmp(table, stringTables[i]->id))
		{
			for (int j=0;j!=stringTables[i]->entries.getNumItems();j++)
			{
				if (id == stringTables[i]->entries[j]->id)
					return stringTables[i]->entries[j]->string;
			}
		}
	}
	return 0;	
}

void LocalesManager::AddString(const wchar_t *table, uint32_t id, const wchar_t *string)
{
	for (int i=0;i!=stringTables.getNumItems();i++)
	{
		if (!wcscmp(table, stringTables[i]->id))
		{
			for (int j=0;j!=stringTables[i]->entries.getNumItems();j++)
			{
				if (id == stringTables[i]->entries[j]->id)
				{
					FREE(stringTables[i]->entries[j]->string);
					stringTables[i]->entries[j]->string=WCSDUP(string);
					return;
				}
			}
			StringEntry *newEntry = new StringEntry;
			newEntry->id = id;
			newEntry->string = WCSDUP(string);
			stringTables[i]->entries.addItem(newEntry);
			return;
		}
	}
	StringTableData *newTable = new StringTableData;
	newTable->id = WCSDUP(table);

	StringEntry *newEntry = new StringEntry;
	newEntry->id = id;
	newEntry->string = WCSDUP(string);
	stringTables.addItem(newTable);
	newTable->entries.addItem(newEntry);
}

void LocalesManager::AddString(uint32_t id, const wchar_t *string)
{
	AddString(curTable, id, string);
}

LocalesAcceleratorXmlCallback LocalesManager::accelXmlCallback;
LocalesAcceleratorSectionXmlCallback LocalesManager::accelSectionXmlCallback;
LocalesTranslationXmlCallback LocalesManager::transXmlCallback;
PtrListSorted<LocTrans, PLS_LocTrans, QuickSorted<LocTrans, PLS_LocTrans> > LocalesManager::translationsList;
PtrListSorted<LocAccel, PLS_LocAccel, QuickSorted<LocAccel, PLS_LocAccel> > LocalesManager::acceleratorsList;
StringW LocalesManager::localePath;
//int LocalesManager::localeListLoaded = 0, LocalesManager::curLocaleNum = -1, LocalesManager::englishLocale;
//PtrList<LocaleItem> LocalesManager::loadableLocalesList;
StringW LocalesManager::curSection;
LocalesSkinCallback LocalesManager::localesSkinCallback;
StringW LocalesManager::curTable;
StringTableXmlCallback LocalesManager::stringTableXmlCallback;
StringTableEntryXmlCallback LocalesManager::stringTableXmlEntryCallback;