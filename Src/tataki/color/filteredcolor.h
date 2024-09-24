#ifndef TATAKI_FILTEREDCOLOR_H
#define TATAKI_FILTEREDCOLOR_H

#include <tataki/export.h>

class TATAKIAPI FilteredColor 
{
public:
  FilteredColor(ARGB32 _color=0, const wchar_t *colorgroupname=L"");
  virtual ~FilteredColor();

  virtual void setColor(ARGB32 _color);
  virtual void setColorGroup(const wchar_t *group);
  ARGB32 getColor();
  ARGB32 *getColorRef();
  virtual const wchar_t *getColorName() { return NULL; }

private:
  void ensureFiltered();

  ARGB32 color;
  ARGB32 filteredcolor;
  wchar_t *group;
  int need_filter;
  int latest_iteration;
};


#endif
