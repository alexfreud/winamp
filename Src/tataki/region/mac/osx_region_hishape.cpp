#include <Carbon/Carbon.h>
#include <tataki/region/region.h>
#include <tataki/bitmap/bitmap.h>

RegionI::RegionI() : rgn(0)
{
}

RegionI::RegionI(const RECT *r) : rgn(0)
{
  setRect(r);
}

RegionI::RegionI(HIMutableShapeRef _rgn) : rgn(_rgn)
{
}

RegionI::RegionI(HIShapeRef _rgn)
{
  rgn=HIShapeCreateMutableCopy(_rgn);
}

RegionI::~RegionI()
{
  if (rgn)
    CFRelease(rgn); 
}

RegionI::RegionI(RgnHandle qdrgn)
{
  HIShapeRef shape = HIShapeCreateWithQDRgn(qdrgn);
  rgn = HIShapeCreateMutableCopy(shape);
  CFRelease(shape);
}

RegionI::RegionI(SkinBitmap *bitmap)
{
  // TODO: we need to find a much better way to do this
  RECT r;
  r.left=0;
  r.top=0;
  r.right=bitmap->getWidth();
  r.bottom=bitmap->getHeight();
  setRect(&r);
}


OSREGIONHANDLE RegionI::getOSHandle()
{
  if (!rgn)
    rgn = HIShapeCreateMutable();
  return rgn;
}

api_region *RegionI::clone()
{
  if (!rgn)
    return new RegionI();
  else
    return new RegionI(HIShapeCreateMutableCopy(rgn));
}

void RegionI::disposeClone(api_region *r)
{
  if (r) // yes we need to check for NULL here because r != static_cast<>(r) 
    delete static_cast<RegionI *>(r);
}

bool RegionI::ptInRegion(const POINT *pt)
{
  if (!rgn)
    return false;
  HIPoint hipt = HIPointFromPOINT(pt);
  return !!HIShapeContainsPoint(rgn, &hipt);
}

void RegionI::offset(int x, int y)
{
  if (!rgn)
    rgn = HIShapeCreateMutable();
  
  HIShapeOffset(rgn, x, y);
}

void RegionI::getBox(RECT *r)
{
  if (!rgn) // TODO: we could manually set r to 0,0,0,0
    rgn = HIShapeCreateMutable();
  
  HIRect rect;
  HIShapeGetBounds(rgn, &rect);
  *r = RECTFromHIRect(&rect);
}

void RegionI::subtractRegion(const api_region *r)
{
  if (rgn)
  {
    api_region *reg = const_cast<api_region *>(r);
    HIShapeRef sub = reg->getOSHandle();
    HIShapeDifference(rgn,sub, rgn);
  }
}

void RegionI::subtractRect(const RECT *r)
{
  if (rgn)
  {
    HIRect rect = HIRectFromRECT(r);
    HIShapeRef sub = HIShapeCreateWithRect(&rect);
    HIShapeDifference(rgn, sub, rgn);
  }
}

void RegionI::addRect(const RECT *r)
{
  if (!rgn)
    rgn = HIShapeCreateMutable();
     HIRect rect = HIRectFromRECT(r);
  HIShapeRef add = HIShapeCreateWithRect(&rect);
  HIShapeUnion(rgn, add, rgn);
}

void RegionI::addRegion(const api_region *r)
{
  if (!rgn)
    rgn = HIShapeCreateMutable();
  api_region *reg = const_cast<api_region *>(r);
  HIShapeRef add = reg->getOSHandle();
  HIShapeUnion(rgn, add, rgn);
}

void RegionI::andRegion(const api_region *r)
{
  if (rgn) // intersection with empty region will always be empty
  {
    api_region *reg = const_cast<api_region *>(r);
    HIShapeRef intersection = reg->getOSHandle();
    HIShapeIntersect(rgn, intersection, rgn);
  }
}

void RegionI::setRect(const RECT *r)
{
  if (rgn) 
    CFRelease(rgn);
     HIRect rect = HIRectFromRECT(r);
  HIShapeRef rectRgn = HIShapeCreateWithRect(&rect);
  rgn = HIShapeCreateMutableCopy(rectRgn);
  CFRelease(rectRgn);
}

void RegionI::empty()
{
  if (rgn) 
    CFRelease(rgn);
  rgn=0; 
}

int RegionI::isEmpty()
{
  if (!rgn)
    return 1;
  return !!HIShapeIsEmpty(rgn);
}

int RegionI::isRect()
{
  if (!rgn)
    return 1;
  return !!HIShapeIsRectangular(rgn);
}

int RegionI::intersectRgn(const api_region *r, api_region *intersection)
{
  intersection->empty();
  intersection->addRegion(this);
  intersection->andRegion(r);
  return !intersection->isEmpty();
}

int RegionI::intersectRect(const RECT *r, api_region *intersection)
{
  intersection->setRect(r);
  intersection->andRegion(this);
  return !intersection->isEmpty();  
}

#define CBCLASS RegionI
START_DISPATCH;
CB(REGION_GETOSHANDLE, getOSHandle);
CB(REGION_CLONE, clone);
VCB(REGION_DISPOSECLONE, disposeClone);
CB(REGION_PTINREGION, ptInRegion);
VCB(REGION_OFFSET, offset);
VCB(REGION_GETBOX, getBox);
VCB(REGION_SUBTRACTRGN, subtractRegion);
VCB(REGION_SUBTRACTRECT, subtractRect);
VCB(REGION_ADDRECT, addRect);
VCB(REGION_ADD, addRegion);
VCB(REGION_AND, andRegion);
VCB(REGION_SETRECT, setRect);
VCB(REGION_EMPTY, empty);
CB(REGION_ISEMPTY, isEmpty);
CB(REGION_ISRECT, isRect);
CB(REGION_INTERSECTRGN, intersectRgn);
CB(REGION_INTERSECTRECT, intersectRect);
END_DISPATCH;
#undef CBCLASS

#define CBCLASS RegionServerI
START_DISPATCH;
VCB(REGIONSERVER_ADDREF, addRef);
VCB(REGIONSERVER_DELREF, delRef);
CB(REGIONSERVER_GETREGION, getRegion);
END_DISPATCH;
#undef CBCLASS