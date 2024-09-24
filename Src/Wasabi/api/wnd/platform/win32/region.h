#ifndef __REGION_H
#define __REGION_H

#include <api/wnd/bitmap.h>
#include <bfc/dispatch.h>

class BaseWnd;
class Canvas;
class api_region;
class RegionServer;

#include "api_region.h"

class RegionI : public api_region 
{
public:
  RegionI();
	RegionI(const RegionI *copy);
  RegionI(const RECT *r);
  RegionI(int l, int t, int r, int b);
  RegionI(OSREGIONHANDLE region);
  RegionI(SkinBitmap *bitmap, RECT *r=NULL, int xoffset=0, int yoffset=0, bool inverted=false, int dothreshold=0, __int8 threshold=0, int threversed=0, int minalpha=1); 
  RegionI(Canvas *c, RECT *defboundbox=NULL);
  virtual ~RegionI();

  api_region *clone();
  void disposeClone(api_region *r);
  bool ptInRegion(const POINT *pt);
  void offset(int x, int y);
  void getBox(RECT *r);
  void subtractRegion(const api_region *reg);
  void subtractRect(const RECT *r);
  void addRect(const RECT *r);
  void addRegion(const api_region *r);
  void andRegion(const api_region *r);
  void setRect(const RECT *r);
  void empty();
  int isEmpty();
  int equals(const api_region *r);
  int enclosed(const api_region *r, api_region *outside=NULL);
  int intersectRgn(const api_region *r, api_region *intersection);
	int doesIntersectRgn(const api_region *r);
  int intersectRect(const RECT *r, api_region *intersection);
	int doesIntersectRect(const RECT *r);
  int isRect();
  void scale(double sx, double sy, bool round=0);
  void debug(int async=0);

  // NONPORTABLE

  OSREGIONHANDLE makeWindowRegion(); // gives you a handle to a clone of the OSREGION object so you can insert it into a window's region with SetWindowRgn. ANY other use is prohibited
  OSREGIONHANDLE getOSHandle(); // avoid as much as you can, should be used only by WIN32-dependant classes

  // END NONPORTABLE

  int getNumRects();
  int enumRect(int n, RECT *r);

  OSREGIONHANDLE alphaToRegionRect(void *pbits32, int bmX, int bmY, int bmWidth, int bmHeight, int fullw, int fullh, int xoffset, int yoffset, bool portion, int _x, int _y, int _w, int _h, bool inverted, int dothreshold, unsigned __int8 threshold, int thinverse, int minalpha);

private:

  inline void init();
  void optimize();
  void deoptimize();

  OSREGIONHANDLE hrgn;
  OSREGIONHANDLE alphaToRegionRect(SkinBitmap *bitmap, int xoffset, int yoffset, bool portion, int _x, int _y, int _w, int _h, bool inverted=false, int dothreshold=0, unsigned __int8 threshold=0, int thinverse=0, int minalpha=1/* 1..255*/);
  RECT overlay;
  int clonecount;
  RegionI *lastdebug;
  RegionServer *srv;
  RECT optrect;
  int optimized;

protected:

  RECVS_DISPATCH;
};

class RegionServer : public Dispatchable {

  protected:
    RegionServer() {}
    virtual ~RegionServer() {}

  public:

    void addRef(void *client);
    void delRef(void *client);
    api_region *getRegion();

  enum {
      REGIONSERVER_ADDREF = 500,
      REGIONSERVER_DELREF = 550,
      REGIONSERVER_GETREGION = 600,
  };
};

inline void RegionServer::addRef(void *client) {
  _voidcall(REGIONSERVER_ADDREF, (api_region *)NULL, client);
}

inline void RegionServer::delRef(void *client) {
  _voidcall(REGIONSERVER_DELREF, client);
}

inline api_region * RegionServer::getRegion() {
  return _call(REGIONSERVER_GETREGION, (api_region *)NULL);
}

class RegionServerI : public RegionServer {
  public :

    RegionServerI() { numrefs = 0; }
    virtual ~RegionServerI() {}
    
    virtual void addRef(void *client) { numrefs++; }
    virtual void delRef(void *client) { numrefs--; }
    virtual api_region *getRegion()=0;

    virtual int getNumRefs() { return numrefs; }

  protected:

    RECVS_DISPATCH;

  private:

    int numrefs;
};

#endif


