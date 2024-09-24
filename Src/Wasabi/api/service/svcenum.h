#ifndef _SVCENUM_H
#define _SVCENUM_H


/*<?<autoheader/>*/
class waServiceFactory;
/*?>*/

// abstract base class
class NOVTABLE SvcEnum {
protected:
  SvcEnum();

  void *_getNext(int global_lock = TRUE);
  void reset();

  virtual int _testService(void *)=0;

public:
#ifdef ASSERTS_ENABLED
  static int release(waServiceFactory *ptr) { ASSERTALWAYS("never ever call release() with a waServiceFactory * !!!"); return 0; }
#endif
  static int release(void *ptr);

  waServiceFactory *getLastFactory();

protected:
  FOURCC type;

private:
  int pos;
  waServiceFactory * factory;
};


#endif // _SVCENUM_H

