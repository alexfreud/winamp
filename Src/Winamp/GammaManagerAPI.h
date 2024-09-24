#pragma once
#include <api/skin/colorthemes.h>
#include <api/service/svcs/svc_skinfilter.h>
#include <vector>
#include <api/skin/api_colorthemes.h>

class ColorThemeGroupI : public ColorThemeGroup
{
public:
  ColorThemeGroupI(const wchar_t *pname, int pred, int pgreen, int pblue, int pgray, int pboost) 
		: name(0), red(pred), green(pgreen), blue(pblue), make_grayscale(pgray), boost_levels(pboost) 
	{
		name = _wcsdup(pname);
	}
~ColorThemeGroupI()
{
	free(name);
}
		ColorThemeGroupI(ColorThemeGroup &copy_group)
		{
			name = _wcsdup(copy_group.getName());
			red = copy_group.getRed();
			green = copy_group.getGreen();
			blue = copy_group.getBlue();
			make_grayscale = copy_group.getGray();
			boost_levels = copy_group.getBoost();
		}
    const wchar_t *getName() { return name; }
    int getRed() { return red; }
    int getGreen() { return green; }
    int getBlue() { return blue; }
    int getGray() { return make_grayscale; }
    int getBoost() { return boost_levels; }
    void setName(const wchar_t *pname) { free(name); name = _wcsdup(pname); }
    void setRed(int r) { red = r; }
    void setGreen(int g) { green = g; }
    void setBlue(int b) { blue = b; }
    void setGray(int g) { make_grayscale = g; }
    void setBoost(int b) { boost_levels = b; }
                                                                                          
  protected:
    RECVS_DISPATCH;

  wchar_t *name;
  int red;
  int green;
  int blue;
  int make_grayscale;
  int boost_levels;
};

static bool IsKeyword(const wchar_t *a, const wchar_t *b)
{
	return (CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE, a, -1, b, -1) == CSTR_EQUAL);
        
}
class GammaSet
{
	public:
	GammaSet( const wchar_t *pname ) : name( 0 ), generalgroup( L"General", 0, 0, 0, 0, 0 )
	{
		name = _wcsdup( pname );
	}

	~GammaSet()
	{
		//gammagroups.deleteAll();
		for ( ColorThemeGroupI *obj : gammagroups )
			delete obj;

		gammagroups.clear();

		free( name );
	}

	int haveGroup( const wchar_t *grp )
	{
		if ( !grp )
			return 0;

		for ( ColorThemeGroupI *obj : gammagroups )
		{
			if ( IsKeyword( grp, obj->getName() ) )
				return 1;
		}

		return 0;
	}

	void SetName( const wchar_t *newname )
	{
		free( name );
		name = _wcsdup( newname );
	}
	wchar_t *name;
	std::vector<ColorThemeGroupI *> gammagroups;
	ColorThemeGroupI generalgroup;
};


class GammaManagerAPI : public api_colorthemes
{
public:
	static const char *getServiceName() { return "Color Themes API"; }
	static const GUID getServiceGuid() { return ColorThemesAPIGUID; }	
	GammaManagerAPI();

	void StartTransaction();
	void EndTransaction();

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
	void addGammaGroup2(size_t gammaSetIndex, ColorThemeGroup *group);

	/* Active Gamma Set */
	const wchar_t *getGammaSet();
	void setGammaSet(const wchar_t *set);

	  protected:
    RECVS_DISPATCH;

private:
	void setGammaSetInternal(GammaSet *set);

	std::vector<GammaSet*> gammasets;
	GammaSet *curSetSel; // current color theme
	size_t inTransaction;
};

