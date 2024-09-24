#ifndef __SNAPPOINT_H
#define __SNAPPOINT_H

#include <bfc/ptrlist.h>
#include <bfc/string/StringW.h>
#include <api/skin/xmlobject.h>

#define SNAPPOINT_XMLPARENT XmlObjectI

class Container;
class Layout;
class SnapPoint;
class ifc_window;

class SnapPoint : public SNAPPOINT_XMLPARENT {

public:
  SnapPoint(Layout *l, Container *c);
  virtual ~SnapPoint();

  virtual int setXmlParam(const wchar_t *name, const wchar_t *strvalue);

  virtual void setParentContainer(Container *c);
  virtual void setParentLayout(Layout *l);
  virtual Container *getParentContainer();
  virtual Layout *getParentLayout();
  virtual const wchar_t *getId();

  virtual int getX();
  virtual int getY();

  static int match(ifc_window *master, RECT *z, ifc_window *slave, int flag, int *donex, int *doney, int w, int h); 
  static void removeAll();

private:
  int x;
  int y;
  int relatx;
  int relaty;
  StringW id;

  Container *pcontainer;
  Layout *playout;

  static PtrList<SnapPoint> points;
  static int do_match(SnapPoint *pmast, SnapPoint *pslav, RECT *z, int mask, int *donex, int *doney, int w, int h);
};

#endif