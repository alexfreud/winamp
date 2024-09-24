#ifndef __REGION_H
#define __REGION_H

#include <tataki/export.h>
#include <Carbon/Carbon.h>
#include <bfc/platform/platform.h>
#include <tataki/region/api_region.h>

class SkinBitmap;

class TATAKIAPI RegionI : public api_region
{
public:
  RegionI();
  RegionI(const RECT *r);
  RegionI(RgnHandle qdrgn);
  RegionI(HIShapeRef _rgn);
  RegionI(SkinBitmap *bitmap);
  ~RegionI();
  
  // api_region
	OSREGIONHANDLE getOSHandle();
  api_region *clone();
  void disposeClone(api_region *r);
	bool ptInRegion(const POINT *pt);
	void offset(int x, int y);
	void getBox(RECT *r);
  void subtractRegion(const api_region *r);
	void subtractRect(const RECT *r);
	void addRect(const RECT *r);  
  void addRegion(const api_region *r);
	void andRegion(const api_region *r);
	void setRect(const RECT *r);
	void empty();  
  int isEmpty();
  int equals(const api_region *r);
	int enclosed(const api_region *r, api_region *outside = NULL);
	int intersectRgn(const api_region *r, api_region *intersection);
	int doesIntersectRgn(const api_region *r);
	int intersectRect(const RECT *r, api_region *intersection);
  
	int isRect();
	void scale(double sx, double sy, bool round = 0);
	void debug(int async = 0);
	OSREGIONHANDLE makeWindowRegion(); // gives you a handle to a clone of the OSREGION object so you can insert it into a window's region with SetWindowRgn. ANY other use is prohibited
  
	// this is how you can enumerate the subrects that compose to make up the
	// entire region
	int getNumRects();
	int enumRect(int n, RECT *r);
  
  
private:
    RegionI(HIMutableShapeRef _rgn);
  HIMutableShapeRef rgn;
  
protected:
    RECVS_DISPATCH;
};


// TODO: we could take of advantage of HIShapeRef's built in reference counting to implement this
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

class TATAKIAPI RegionServerI : public RegionServer {
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


