#include <precomp.h>

#include "svc_collection.h"

// an named xml overridable collection of objects

#define CBCLASS svc_collectionI
START_DISPATCH;
  CB(COLLECTION_TESTTAG,              testTag);
  VCB(COLLECTION_ADDELEMENT,          addElement);
  VCB(COLLECTION_REMOVEELEMENT,       removeElement);
  VCB(COLLECTION_REMOVEALLELEMENTS,   removeAllElements);
  CB(COLLECTION_GETNUMELEMENTS,       getNumElements);
  CB(COLLECTION_GETNUMELEMENTSUNIQUE, getNumElementsUnique);
  CB(COLLECTION_ENUMELEMENT,          enumElement);
  CB(COLLECTION_ENUMELEMENTUNIQUE,    enumElementUnique);
  CB(COLLECTION_GETELEMENT,           getElement);
END_DISPATCH;
#undef CBCLASS

#define CBCLASS CollectionElementI
START_DISPATCH;
  CB(COLLECTIONELEMENT_GETID,            getId);
  CB(COLLECTIONELEMENT_GETPARAMVALUE,    getParamValue);
  CB(COLLECTIONELEMENT_GETPARAMVALUEINT, getParamValueInt);
  CB(COLLECTIONELEMENT_GETINCLUDEPATH,   getIncludePath);
END_DISPATCH;
#undef CBCLASS


svc_collectionI::svc_collectionI() {
  count = 0;
  elements.setAutoSort(1);
}

svc_collectionI::~svc_collectionI() {
}

void svc_collectionI::addElement(const char *id, const char *includepath, int incrementalremovalid, skin_xmlreaderparams *params) {
  CollectionElementI *cei = new CollectionElementI(this, id, params, incrementalremovalid, includepath);
  elements.addItem(cei);
}

void svc_collectionI::removeElement(int removalid) {
  for (int i=0;i<elements.getNumItems();i++) {
    CollectionElementI *e = elements.enumItem(i);
    if (e->getSecCount() == removalid) {
      elements.removeItem(e);
      delete e;
      i--;
    }
  }
}

void svc_collectionI::removeAllElements() {
  elements.deleteAll();
}

int svc_collectionI::getNumElements() {
  return elements.getNumItems();
}

int svc_collectionI::getNumElementsUnique() {
  int i=0;
  int n=0;
  const char *previous = NULL;
  for (i=0;i<elements.getNumItems();i++) {
    const char *id = elements.enumItem(i)->getId();
    if (!STRCASEEQLSAFE(id, previous))
      n++;
    previous = id;
  }
  return n;
}

CollectionElement *svc_collectionI::enumElementUnique(int n, int *ancestor) {
  int i=0;
  int _n=-1;
  CollectionElement *e=NULL;
  CollectionElement *previous = NULL;
  elements.sort(1);
  for (i=0;i<elements.getNumItems();i++) {
    CollectionElement *c = elements.enumItem(i);
    if (!STRCASEEQLSAFE(c->getId(), previous ? previous->getId() : NULL)) {
      if (_n == n) 
        break;
      _n++;
    }
    previous = c;
  }
  if (_n == n) 
    e = previous;
  else
    e = NULL;
  if (ancestor != NULL) {
    if (e != NULL) {
      int pos=-1;
      elements.findItem(static_cast<CollectionElementI*>(e), &pos);
      if (pos > 0) {
        CollectionElement *f = elements.enumItem(pos-1);
        if (!STRCASEEQLSAFE(f ? f->getId() : NULL, e->getId())) *ancestor = -1;
      } else {
        *ancestor = -1;
        e = NULL;
      }
    } else
      *ancestor = -1;
  }
  return e;
}

CollectionElement *svc_collectionI::enumElement(int n, int *ancestor) {
  CollectionElement *e = elements.enumItem(n);
  if (ancestor != NULL) {
    CollectionElement *a = elements.enumItem(n-1);
    if (!STRCASEEQL(a->getId(), e->getId())) *ancestor = -1;
    *ancestor = n-1;
  }
  return e;
}

CollectionElement *svc_collectionI::getElement(const char *id, int *ancestor) {
  int pos=-1;
  CollectionElement *e = elements.findLastItem(id, &pos);
  if (ancestor != NULL) {
    CollectionElement *a = elements.enumItem(pos-1);
    if (!STRCASEEQL(a->getId(), e->getId())) *ancestor = -1;
    *ancestor = pos-1;
  }
  return e;
}

CollectionElement *svc_collectionI::getAncestor(CollectionElement *e) {
  int pos=-1;
  CollectionElementI *ei = static_cast<CollectionElementI *>(e);
  elements.findItem(ei, &pos);
  if (pos >= 0) {
    pos--;
    if (STRCASEEQL(elements.enumItem(pos)->getId(), e->getId())) return elements.enumItem(pos);
  }
  return NULL;
}

CollectionElementI::CollectionElementI(svc_collectionI *col, const char *_id, skin_xmlreaderparams *p, int _seccount, const char *_path) {
  id = _id;
  for (int i=0;i<p->getNbItems();i++) {
    Pair < String, String > *pr = new Pair < String, String >("","");
    pr->a = p->getItemName(i);
    pr->b = p->getItemValue(i);
    params.addItem(pr);
  }
  seccount = _seccount;
  collection = col;
  path = _path;
}

CollectionElementI::~CollectionElementI() {
  params.deleteAll();
}

const char *CollectionElementI::getId() {
  return id;
}

const char *CollectionElementI::getParamValue(const char *param, CollectionElement **item){
  CollectionElement *e = getAncestor();
  const char *a = e ? e->getParamValue(param) : NULL;
  Pair<String, String> *p = params.findItem(param);
  a = p ? p->b.getValue() : a;
  if (item && p != NULL) *item = this;
  return a;
}

int CollectionElementI::getParamValueInt(const char *param){
  const char *a = getParamValue(param);
  return ATOI(a);
}

int CollectionElementI::getSecCount() {
  return seccount;
}

CollectionElement *CollectionElementI::getAncestor() {
  return collection->getAncestor(this);
}

const char *CollectionElementI::getIncludePath(const char *param/* =NULL */) {
  if (param == NULL) return path;
  CollectionElement *i;
  if (!getParamValue(param, &i)) return NULL;
  if (i != NULL) 
    return i->getIncludePath(NULL);
  return NULL;
}