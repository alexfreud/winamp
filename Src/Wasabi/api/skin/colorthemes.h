#ifndef __COLORTHEMEGROUP_H
#define __COLORTHEMEGROUP_H

#include <bfc/dispatch.h>

class ColorThemeGroup : public Dispatchable 
{
  public:
    const wchar_t *getName();
    int getRed();
    int getGreen();
    int getBlue();
    int getGray();
    int getBoost();

    void setName(const wchar_t *name);
    void setRed(int r);
    void setGreen(int g);
    void setBlue(int b);
    void setGray(int g);
    void setBoost(int b);

  enum {
    COLORTHEMEGROUPGETNAME=10,
    COLORTHEMEGROUPGETRED=20,
    COLORTHEMEGROUPGETGREEN=30,
    COLORTHEMEGROUPGETBLUE=40,
    COLORTHEMEGROUPGETGRAY=50,
    COLORTHEMEGROUPGETBOOST=60,
    COLORTHEMEGROUPSETNAME=70,
    COLORTHEMEGROUPSETRED=80,
    COLORTHEMEGROUPSETGREEN=90,
    COLORTHEMEGROUPSETBLUE=100,
    COLORTHEMEGROUPSETGRAY=110,
    COLORTHEMEGROUPSETBOOST=120,
  };
};

inline const wchar_t *ColorThemeGroup::getName() {
  return _call(COLORTHEMEGROUPGETNAME, (const wchar_t *)NULL);
}

inline int ColorThemeGroup::getRed() {
  return _call(COLORTHEMEGROUPGETRED, 0);
}

inline int ColorThemeGroup::getGreen() {
  return _call(COLORTHEMEGROUPGETGREEN, 0);
}

inline int ColorThemeGroup::getBlue() {
  return _call(COLORTHEMEGROUPGETBLUE, 0);
}

inline int ColorThemeGroup::getGray() {
  return _call(COLORTHEMEGROUPGETGRAY, 0);
}

inline int ColorThemeGroup::getBoost() {
  return _call(COLORTHEMEGROUPGETBOOST, 0);
}

inline void ColorThemeGroup::setName(const wchar_t *name) {
  _voidcall(COLORTHEMEGROUPSETNAME, name);
}

inline void ColorThemeGroup::setRed(int r) {
  _voidcall(COLORTHEMEGROUPSETRED, r);
}

inline void ColorThemeGroup::setGreen(int g) {
  _voidcall(COLORTHEMEGROUPSETGREEN, g);
}

inline void ColorThemeGroup::setBlue(int b) {
  _voidcall(COLORTHEMEGROUPSETBLUE, b);
}

inline void ColorThemeGroup::setGray(int g) {
  _voidcall(COLORTHEMEGROUPSETGRAY, g);
}

inline void ColorThemeGroup::setBoost(int b) {
  _voidcall(COLORTHEMEGROUPSETBOOST, b);
}

#endif
