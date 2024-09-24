#ifndef _SVC_COLLECTION_H
#define _SVC_COLLECTION_H

#include <bfc/dispatch.h>
#include <bfc/ptrlist.h>
#include <bfc/pair.h>
#include <api/xml/xmlreader.h>
#include <api/service/services.h>

class CollectionElement;
class svc_collectionI;

class NOVTABLE svc_collection : public Dispatchable 
{
public:
  static FOURCC getServiceType() { return WaSvc::COLLECTION; }
  int testTag(const wchar_t *xmltag);
  void addElement(const wchar_t *id, const wchar_t *includepath, int removalid, skin_xmlreaderparams *params); 
  void removeElement(int recored_value);
  void removeAllElements();
  int getNumElements();
  int getNumElementsUnique();
  CollectionElement *enumElement(int n, int *ancestor);
  CollectionElement *enumElementUnique(int n, int *ancestor);
  CollectionElement *getElement(const wchar_t *id, int *ancestor);
  CollectionElement *getAncestor(CollectionElement *e);

  enum 
	{
    COLLECTION_TESTTAG=10,
    COLLECTION_ADDELEMENT=20,
    COLLECTION_REMOVEELEMENT=30,
    COLLECTION_REMOVEALLELEMENTS=35,
    COLLECTION_GETNUMELEMENTS=40,
    COLLECTION_GETNUMELEMENTSUNIQUE=50,
    COLLECTION_ENUMELEMENT=60,
    COLLECTION_ENUMELEMENTUNIQUE=70,
    COLLECTION_GETELEMENT=80,
  };
};

inline int svc_collection::testTag(const wchar_t *xmltag) {
  return _call(COLLECTION_TESTTAG, 0, xmltag);
}

inline void svc_collection::addElement(const wchar_t *id, const wchar_t *includepath, int incrementalremovalid, skin_xmlreaderparams *params) {
  _voidcall(COLLECTION_ADDELEMENT, id, includepath, incrementalremovalid, params);
}

inline void svc_collection::removeElement(int removalid) {
  _voidcall(COLLECTION_REMOVEELEMENT, removalid);
}

inline void svc_collection::removeAllElements() {
  _voidcall(COLLECTION_REMOVEALLELEMENTS);
}

inline int svc_collection::getNumElements() {
  return _call(COLLECTION_GETNUMELEMENTS, 0);
}

inline int svc_collection::getNumElementsUnique() {
  return _call(COLLECTION_GETNUMELEMENTSUNIQUE, 0);
}

inline CollectionElement *svc_collection::enumElement(int n, int *ancestor) {
  return _call(COLLECTION_ENUMELEMENT, (CollectionElement *)NULL, n, ancestor);
}

inline CollectionElement *svc_collection::enumElementUnique(int n, int *ancestor) {
  return _call(COLLECTION_ENUMELEMENTUNIQUE, (CollectionElement *)NULL, n, ancestor);
}

inline CollectionElement *svc_collection::getElement(const wchar_t *id, int *ancestor) {
  return _call(COLLECTION_GETELEMENT, (CollectionElement *)NULL, id, ancestor);
}

class SortPairString {
public:
  static int compareItem(Pair<StringW, StringW> *p1, Pair<StringW, StringW> *p2) {
    return WCSICMP(p1->a, p2->a);
  }
  static int compareAttrib(const wchar_t *attrib, Pair<StringW, StringW> *item) {
    return WCSICMP(attrib, item->a);
  }
};

class CollectionElement : public Dispatchable {
  public:
    const wchar_t *getId();
    const wchar_t *getParamValue(const wchar_t *param, CollectionElement **item=NULL);
    int getParamValueInt(const wchar_t *param);
    const wchar_t *getIncludePath(const wchar_t *param=NULL); 
    CollectionElement *getAncestor();

    enum {
      COLLECTIONELEMENT_GETID=10,
      COLLECTIONELEMENT_GETPARAMVALUE=20,
      COLLECTIONELEMENT_GETPARAMVALUEINT=30,
      COLLECTIONELEMENT_GETANCESTOR=40,
      COLLECTIONELEMENT_GETINCLUDEPATH=50,
    };
};

inline const wchar_t *CollectionElement::getId() {
  return _call(COLLECTIONELEMENT_GETID, (const wchar_t *)NULL);
}

inline const wchar_t *CollectionElement::getParamValue(const wchar_t *param, CollectionElement **item) {
  return _call(COLLECTIONELEMENT_GETPARAMVALUE, (const wchar_t *)NULL, param, item);
}

inline int CollectionElement::getParamValueInt(const wchar_t *param) {
  return _call(COLLECTIONELEMENT_GETPARAMVALUEINT, 0, param);
}

inline CollectionElement *CollectionElement::getAncestor() {
  return _call(COLLECTIONELEMENT_GETANCESTOR, (CollectionElement *)NULL);
}

inline const wchar_t *CollectionElement::getIncludePath(const wchar_t *param) {
  return _call(COLLECTIONELEMENT_GETINCLUDEPATH, (const wchar_t *)NULL, param);
}

class CollectionElementI : public CollectionElement {
  public:
    CollectionElementI(svc_collectionI *collectionI, const wchar_t *id, skin_xmlreaderparams *params, int seccount, const wchar_t *includepath);
    virtual ~CollectionElementI();

    virtual const wchar_t *getId();
    virtual const wchar_t *getParamValue(const wchar_t *param, CollectionElement **item=NULL);
    virtual int getParamValueInt(const wchar_t *param);
    virtual CollectionElement *getAncestor();
    const wchar_t *getIncludePath(const wchar_t *param=NULL);  // null returns last override's include path

    int getSecCount();

  protected:
    RECVS_DISPATCH;

    PtrListQuickSorted < Pair < StringW, StringW >, SortPairString > params;
    StringW id;
    int seccount;
    svc_collectionI *collection;
    StringW path;
};

class SortCollectionElementsI {
public:
  static int compareItem(CollectionElementI *p1, CollectionElementI *p2) {
    int r = WCSICMP(p1->getId(), p2->getId());
    if (r == 0) {
      if (p1->getSecCount() < p2->getSecCount()) return -1;
      if (p1->getSecCount() > p2->getSecCount()) return 1;
      return 0;
    }
    return r;
  }
  static int compareAttrib(const wchar_t *attrib, CollectionElementI *item) {
    return WCSICMP(attrib, item->getId());
  }
};

// derive from this one
class svc_collectionI : public svc_collection {
public:
  svc_collectionI();
  virtual ~svc_collectionI();
  virtual int testTag(const wchar_t *xmltag)=0;
  virtual void addElement(const wchar_t *id, const wchar_t *includepath, int incrementalremovalid, skin_xmlreaderparams *params);
  virtual void removeElement(int removalid);
  virtual void removeAllElements();
  virtual int getNumElements();
  virtual int getNumElementsUnique();
  virtual CollectionElement *enumElement(int n, int *ancestor);
  virtual CollectionElement *enumElementUnique(int n, int *ancestor);
  virtual CollectionElement *getElement(const wchar_t *id, int *ancestor);
  virtual CollectionElement *getAncestor(CollectionElement *e);

protected:
  RECVS_DISPATCH;

  PtrListQuickMultiSorted < CollectionElementI, SortCollectionElementsI > elements;
  int count;
};

#include <api/service/servicei.h>
template <class T>
class CollectionCreator : public waServiceFactoryTSingle<svc_collection, T> {};

template <wchar_t TAG[]>
class CollectionSvc : public svc_collectionI {
  public:
    int testTag(const wchar_t *xmltag) {
      if (!WCSICMP(xmltag, TAG)) return 1;
      return 0;
    }
  static const char *getServiceName() { return StringPrintf("Collection Service for \"%S\"", TAG); }
};

template <class T>
class CollectionSvc2 : public svc_collectionI {
  public:
    int testTag(const wchar_t *xmltag) {
      if (STRCASEEQL(xmltag, T::collection_getXmlTag())) return 1;
      return 0;
    }
  static const char *getServiceName() { return StringPrintf("Collection Service for \"%S\"", T::collection_getXmlTag()); }
};


#include <api/service/svc_enum.h>
#include <bfc/string/StringW.h>
class CollectionSvcEnum : public SvcEnumT<svc_collection> 
{
public:
  CollectionSvcEnum(const wchar_t *xmltag) : tag(xmltag) {}
protected:
  virtual int testService(svc_collection *svc) {
    return (svc->testTag(tag));
  }
private:
  StringW tag;
};

#endif
