#ifndef _GAMMAMGR_H
#define _GAMMAMGR_H

#include <api/xml/xmlreader.h>
#include <api/service/svcs/svc_skinfilter.h>

#include <bfc/string/StringW.h>
#include <api/skin/colorthemes.h>


class ColorThemeGroupI : public ColorThemeGroup
{
public:
  ColorThemeGroupI(const wchar_t *pname, int pred, int pgreen, int pblue, int pgray, int pboost) 
		: name(pname), red(pred), green(pgreen), blue(pblue), make_grayscale(pgray), boost_levels(pboost) { }
		ColorThemeGroupI(ColorThemeGroup &copy_group)
		{
			name = copy_group.getName();
			red = copy_group.getRed();
			green = copy_group.getGreen();
			blue = copy_group.getBlue();
			make_grayscale = copy_group.getGray();
			boost_levels = copy_group.getBoost();
		}
    virtual const wchar_t *getName() { return name; }
    virtual int getRed() { return red; }
    virtual int getGreen() { return green; }
    virtual int getBlue() { return blue; }
    virtual int getGray() { return make_grayscale; }
    virtual int getBoost() { return boost_levels; }
    virtual void setName(const wchar_t *pname) { name = pname; }
    virtual void setRed(int r) { red = r; }
    virtual void setGreen(int g) { green = g; }
    virtual void setBlue(int b) { blue = b; }
    virtual void setGray(int g) { make_grayscale = g; }
    virtual void setBoost(int b) { boost_levels = b; }
                                                                                          
  protected:
    RECVS_DISPATCH;

  StringW name;
  int red;
  int green;
  int blue;
  int make_grayscale;
  int boost_levels;
};




class GammaMgrXmlReader : public XmlReaderCallbackI 
{
public:
  void xmlReaderOnStartElementCallback(const wchar_t *xmltag, skin_xmlreaderparams *params);
};

class GammaMgr 
{
public:
  static void init();
  static void deinit();

  static void onBeforeLoadingGammaGroups();
  static void onAfterLoadingGammaGroups();
  static int gammaEqual(const wchar_t *id1, const wchar_t *id2);
  
  static void xmlReaderOnStartElementCallback(const wchar_t *xmltag, skin_xmlreaderparams *params);

  static int filterBitmap(uint8_t *bits, int w, int h, int bpp, const wchar_t *element_id, const wchar_t *forcegroup=NULL);
  static ARGB32 filterColor(ARGB32 color, const wchar_t *element_id, const wchar_t *forcegroup=NULL);
/*
  static void setGammaSet(const wchar_t *set);
  static int getNumGammaSets();
  static const wchar_t *enumGammaSet(int n);
  static int getNumGammaGroups(const wchar_t *gammaset);
  static const wchar_t *enumGammaGroup(const wchar_t *gammaset, int n);
  static ColorThemeGroup *enumColorThemeGroup(int colorset, int colorgroup);
  static ColorThemeGroup *getColorThemeGroup(const wchar_t *colorset, const wchar_t *colorgroup);
  static void newGammaSet(const wchar_t *set);
  static void updateGammaSet(const wchar_t *set);
  static void renameGammaSet(const wchar_t *set, const wchar_t *newname);
  static void deleteGammaSet(const wchar_t *set);*/
  static void loadDefault();

private:
	static void onGammaSet();
  static GammaMgrXmlReader xmlreader;
  static int curset; // current while parsing xml
};
#endif
