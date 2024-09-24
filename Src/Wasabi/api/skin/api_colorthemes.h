#pragma once
#include <bfc/dispatch.h>
#include <api/skin/colorthemes.h>

class api_colorthemes : public Dispatchable
{
protected:
	api_colorthemes(){}
	~api_colorthemes(){}
public:
	/* Gamma Sets */
	size_t getNumGammaSets();
	const wchar_t *enumGammaSet(size_t n);
	void deleteGammaSet(const wchar_t *set);
	void deleteAllGammaSets();
	void resetGammaSet(const wchar_t *set);
	void renameGammaSet(const wchar_t *set, const wchar_t *newname);
	size_t newGammaSet(const wchar_t *set); // returns index of your new gamma group
	void updateGammaSet(const wchar_t *set);
	
	/* Gamma Groups */
	int getNumGammaGroups(const wchar_t *gammaset);
	const wchar_t *enumGammaGroup(const wchar_t *gammaset, int n);
	ColorThemeGroup *enumColorThemeGroup(int colorset, int colorgroup);
	ColorThemeGroup *getColorThemeGroup(const wchar_t *colorset, const wchar_t *colorgroup);
	int getGammaForGroup(const wchar_t *group, int *r, int *g, int *b, int *gray, int *boost);
	void addGammaGroup(const wchar_t *set, ColorThemeGroup *group);
	void addGammaGroup(size_t gammaSetIndex, ColorThemeGroup *group);

	/* Active Gamma Set */
	const wchar_t *getGammaSet();
	void setGammaSet(const wchar_t *set);

	/* Call these if you are loading a whole bunch of color themes at once */
	void StartTransaction();
	void EndTransaction();

	enum
	{
		API_COLORTHEMES_GETNUMGAMMASETS     = 0,
		API_COLORTHEMES_ENUMGAMMASET        = 1,
		API_COLORTHEMES_DELETEGAMMASET      = 2,
		API_COLORTHEMES_DELETEALLGAMMASETS  = 3,
		API_COLORTHEMES_RESETGAMMASET       = 4,
		API_COLORTHEMES_RENAMEGAMMASET      = 5,
		API_COLORTHEMES_NEWGAMMASET         = 6,
		API_COLORTHEMES_UPDATEGAMMASET      = 7,
		API_COLORTHEMES_GETNUMGAMMAGROUPS   = 8,
		API_COLORTHEMES_ENUMGAMMAGROUP      = 9,
		API_COLORTHEMES_ENUMCOLORTHEMEGROUP = 10,
		API_COLORTHEMES_GETCOLORTHEMEGROUP  = 11,
		API_COLORTHEMES_GETGAMMAFORGROUP    = 12,
		API_COLORTHEMES_ADDGAMMAGROUP       = 13,
		API_COLORTHEMES_ADDGAMMAGROUP2      = 14,
		API_COLORTHEMES_GETGAMMASET         = 15,
		API_COLORTHEMES_SETGAMMASET         = 16,
		API_COLORTHEMES_STARTTRANSACTION    = 17,
		API_COLORTHEMES_ENDTRANSACTION      = 18,
	};
};

inline size_t api_colorthemes::getNumGammaSets()
{
	return _call(API_COLORTHEMES_GETNUMGAMMASETS, (size_t)0);
}

inline const wchar_t *api_colorthemes::enumGammaSet(size_t n)
{
	return _call(API_COLORTHEMES_ENUMGAMMASET, (const wchar_t *)0, n);
}

inline void api_colorthemes::deleteGammaSet(const wchar_t *set)
{
	_voidcall(API_COLORTHEMES_DELETEGAMMASET, set);
}

inline void api_colorthemes::deleteAllGammaSets()
{
	_voidcall(API_COLORTHEMES_DELETEALLGAMMASETS);
}

inline void api_colorthemes::resetGammaSet(const wchar_t *set)
{
	_voidcall(API_COLORTHEMES_RESETGAMMASET, set);
}

inline void api_colorthemes::renameGammaSet(const wchar_t *set, const wchar_t *newname)
{
	_voidcall(API_COLORTHEMES_RENAMEGAMMASET, set, newname);
}

inline size_t api_colorthemes::newGammaSet(const wchar_t *set)
{
	return _call(API_COLORTHEMES_NEWGAMMASET, (size_t)-1, set);
}

inline void api_colorthemes::updateGammaSet(const wchar_t *set)
{
	_voidcall(API_COLORTHEMES_UPDATEGAMMASET, set);
}

inline int api_colorthemes::getNumGammaGroups(const wchar_t *gammaset)
{
	return _call(API_COLORTHEMES_GETNUMGAMMAGROUPS, (int)0, gammaset);
}

inline const wchar_t *api_colorthemes::enumGammaGroup(const wchar_t *gammaset, int n)
{
	return _call(API_COLORTHEMES_ENUMGAMMAGROUP, (const wchar_t *)0, gammaset, n);
}

inline ColorThemeGroup *api_colorthemes::enumColorThemeGroup(int colorset, int colorgroup)
{
	return _call(API_COLORTHEMES_ENUMCOLORTHEMEGROUP, (ColorThemeGroup *)0, colorset, colorgroup);
}

inline ColorThemeGroup *api_colorthemes::getColorThemeGroup(const wchar_t *colorset, const wchar_t *colorgroup)
{
	return _call(API_COLORTHEMES_GETCOLORTHEMEGROUP, (ColorThemeGroup *)0, colorset, colorgroup);
}

inline int api_colorthemes::getGammaForGroup(const wchar_t *group, int *r, int *g, int *b, int *gray, int *boost)
{
	return _call(API_COLORTHEMES_GETGAMMAFORGROUP, (int)0, group, r, g, b, gray, boost);
}

inline void api_colorthemes::addGammaGroup(const wchar_t *set, ColorThemeGroup *group)
{
	_voidcall(API_COLORTHEMES_ADDGAMMAGROUP, set, group);
}

inline void api_colorthemes::addGammaGroup(size_t gammaSetIndex, ColorThemeGroup *group)
{
	_voidcall(API_COLORTHEMES_ADDGAMMAGROUP2, gammaSetIndex, group);
}

inline const wchar_t *api_colorthemes::getGammaSet()
{
	return _call(API_COLORTHEMES_GETGAMMASET, (const wchar_t *)0);
}

inline void api_colorthemes::setGammaSet(const wchar_t *set)
{
	_voidcall(API_COLORTHEMES_SETGAMMASET, set);
}

inline void api_colorthemes::StartTransaction()
{
		_voidcall(API_COLORTHEMES_STARTTRANSACTION);
}

inline void api_colorthemes::EndTransaction()
{
	_voidcall(API_COLORTHEMES_ENDTRANSACTION);
}

// {A3AAB98E-1634-4763-81A7-8D397F9E3154}
static const GUID ColorThemesAPIGUID= 
{ 0xa3aab98e, 0x1634, 0x4763, { 0x81, 0xa7, 0x8d, 0x39, 0x7f, 0x9e, 0x31, 0x54 } };

extern api_colorthemes *colorThemesApi;
