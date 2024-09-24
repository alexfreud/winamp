#ifndef TATAKI_SKINCLR_H
#define TATAKI_SKINCLR_H

#include <tataki/export.h>
#include "filteredcolor.h"

// note: only pass in a const char *
class TATAKIAPI SkinColor : public FilteredColor 
{
public:
  explicit SkinColor(const wchar_t *name=NULL, const wchar_t *colorgroup=NULL);
	~SkinColor();
  virtual void setColor(ARGB32 c);

  ARGB32 v(ARGB32 defaultColor=0xFFFF00FF);
  operator int() { return v(); }

  void setElementName(const wchar_t *name);
  const wchar_t *operator =(const wchar_t *name);
  virtual const wchar_t *getColorName();

  int iteratorValid();	// if FALSE, color might have changed

	// if you just need to do a one-off skin color query, use this function
// because SkinColor class does some malloc'ing
	static ARGB32 GetColor(const wchar_t *name, const wchar_t *group = 0, ARGB32 defaultColor=0xFFFF00FF);
	static bool TryGetColor(ARGB32 *color, const wchar_t *name, const wchar_t *group = 0);
private:
  wchar_t *name;
  ARGB32 *val;
  int latest_iteration;
  const wchar_t *ovr_grp;
  int color_override;
  int dooverride;
};



#endif
