#ifndef _WASABI_MULTIPATCH_H
#define _WASABI_MULTIPATCH_H
// (c) 2005 Nullsoft, Inc.

#include "dispatch.h"
/*
Author: Ben Allison <benski@nullsoft.com>

The purpose of the MultiPatch class is to allow a class to have multiple Dispatchable interfaces.

It is an alternative to the method of using a stub class with virtual functions.
(you see these classes all over Wasabi, either ending in I or X, depending on age)
Since each vtable entry costs 4 bytes per object (8 on 64bit), using this method adds some code size
(and, in some multiple-inheritance situations, memory per object for vtable overrides)

Of course, MultiPatch's dispatching function adds code size, too.  But the function shouldn't be any 
larger or slower than the dispatching in the api_X class anyway.

Using MultiPatch, each method call goes through this chain:    
   dispatch->multipatch->your class method

  as opposed to:
   dispatch->your class method   (single inheritance)

  or:
   dispatch->virtual function->your class method  (api_X method)

A clever compiler might be able to optimize away the _multipatch function call by 
pushing "patch" on the top of the parameter stack and jmp'ing.

For an example on usage, scroll down to the bottom of the page.
*/

template <int patch_t, class base_t>
class MultiPatch : public base_t
{
protected:
	virtual int _multipatch(int patch, int msg, void *retval, void **params=0, int nparam=0) = 0;

private:
	int _dispatch(int msg, void *retval, void **params=0, int nparam=0)
	{
		return _multipatch(patch_t, msg, retval, params, nparam);
	}

public:
	/* these helper functions are a direct copy from Dispatchable...  
	 * They need to be duplicated, because the compiler gets confused when trying to cast
	 * from Dispatchable to the child class (since that class has multiple Dispatchable parents)
   *
	 * They could potentially be eliminated by modifying Dispatchable's versions, but the risk of breakage
	 * isn't worth it.
  */
	template <class CLASSNAME, class RETVAL>
  void cb(RETVAL (CLASSNAME::*fn)(), void *retval, void **params) {
    *static_cast<RETVAL*>(retval) = (static_cast<CLASSNAME *>(this)->*fn)();
  }

  template <class CLASSNAME>
  void vcb(void (CLASSNAME::*fn)(), void *retval, void **params) {
    (static_cast<CLASSNAME *>(this)->*fn)();
  }

  template <class CLASSNAME, class RETVAL, class PARAM1>
  void cb(RETVAL (CLASSNAME::*fn)(PARAM1), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    *static_cast<RETVAL*>(retval) = (static_cast<CLASSNAME *>(this)->*fn)(*p1);
  }

  template <class CLASSNAME, class PARAM1>
  void vcb(void (CLASSNAME::*fn)(PARAM1), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    (static_cast<CLASSNAME *>(this)->*fn)(*p1);
  }

  template <class CLASSNAME, class PARAM1, class PARAM2>
  void vcb(void (CLASSNAME::*fn)(PARAM1, PARAM2), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2);
  }

  template <class CLASSNAME, class RETVAL, class PARAM1, class PARAM2>
  void cb(RETVAL (CLASSNAME::*fn)(PARAM1, PARAM2), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    *static_cast<RETVAL*>(retval) = (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2);
  }

  // 3 params
  template <class CLASSNAME, class PARAM1, class PARAM2, class PARAM3>
  void vcb(void (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3);
  }

  template <class CLASSNAME, class RETVAL, class PARAM1, class PARAM2, class PARAM3>
  void cb(RETVAL (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    *static_cast<RETVAL*>(retval) = (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3);
  }

  // 4 params
  template <class CLASSNAME, class PARAM1, class PARAM2, class PARAM3, class PARAM4>
  void vcb(void (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3, PARAM4), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    PARAM4 *p4 = static_cast<PARAM4*>(params[3]);
    (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3, *p4);
  }

  template <class CLASSNAME, class RETVAL, class PARAM1, class PARAM2, class PARAM3, class PARAM4>
  void cb(RETVAL (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3, PARAM4), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    PARAM4 *p4 = static_cast<PARAM4*>(params[3]);
    *static_cast<RETVAL*>(retval) = (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3, *p4);
  }

  // 5 params
  template <class CLASSNAME, class PARAM1, class PARAM2, class PARAM3, class PARAM4, class PARAM5>
  void vcb(void (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    PARAM4 *p4 = static_cast<PARAM4*>(params[3]);
    PARAM5 *p5 = static_cast<PARAM5*>(params[4]);
    (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3, *p4, *p5);
  }

  template <class CLASSNAME, class RETVAL, class PARAM1, class PARAM2, class PARAM3, class PARAM4, class PARAM5>
  void cb(RETVAL (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    PARAM4 *p4 = static_cast<PARAM4*>(params[3]);
    PARAM5 *p5 = static_cast<PARAM5*>(params[4]);
    *static_cast<RETVAL*>(retval) = (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3, *p4, *p5);
  }

  // 6 params
  template <class CLASSNAME, class PARAM1, class PARAM2, class PARAM3, class PARAM4, class PARAM5, class PARAM6>
  void vcb(void (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    PARAM4 *p4 = static_cast<PARAM4*>(params[3]);
    PARAM5 *p5 = static_cast<PARAM5*>(params[4]);
    PARAM6 *p6 = static_cast<PARAM6*>(params[5]);
    (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3, *p4, *p5, *p6);
  }

  template <class CLASSNAME, class RETVAL, class PARAM1, class PARAM2, class PARAM3, class PARAM4, class PARAM5, class PARAM6>
  void cb(RETVAL (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    PARAM4 *p4 = static_cast<PARAM4*>(params[3]);
    PARAM5 *p5 = static_cast<PARAM5*>(params[4]);
    PARAM6 *p6 = static_cast<PARAM6*>(params[5]);
    *static_cast<RETVAL*>(retval) = (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3, *p4, *p5, *p6);
  }

  // 7 params
  template <class CLASSNAME, class PARAM1, class PARAM2, class PARAM3, class PARAM4, class PARAM5, class PARAM6, class PARAM7>
  void vcb(void (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6, PARAM7), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    PARAM4 *p4 = static_cast<PARAM4*>(params[3]);
    PARAM5 *p5 = static_cast<PARAM5*>(params[4]);
    PARAM6 *p6 = static_cast<PARAM6*>(params[5]);
    PARAM7 *p7 = static_cast<PARAM7*>(params[6]);
    (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3, *p4, *p5, *p6, *p7);
  }

  template <class CLASSNAME, class RETVAL, class PARAM1, class PARAM2, class PARAM3, class PARAM4, class PARAM5, class PARAM6, class PARAM7>
  void cb(RETVAL (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6, PARAM7), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    PARAM4 *p4 = static_cast<PARAM4*>(params[3]);
    PARAM5 *p5 = static_cast<PARAM5*>(params[4]);
    PARAM6 *p6 = static_cast<PARAM6*>(params[5]);
    PARAM7 *p7 = static_cast<PARAM7*>(params[6]);
    *static_cast<RETVAL*>(retval) = (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3, *p4, *p5, *p6, *p7);
  }

  // 8 params
  template <class CLASSNAME, class PARAM1, class PARAM2, class PARAM3, class PARAM4, class PARAM5, class PARAM6, class PARAM7, class PARAM8>
  void vcb(void (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6, PARAM7, PARAM8), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    PARAM4 *p4 = static_cast<PARAM4*>(params[3]);
    PARAM5 *p5 = static_cast<PARAM5*>(params[4]);
    PARAM6 *p6 = static_cast<PARAM6*>(params[5]);
    PARAM7 *p7 = static_cast<PARAM7*>(params[6]);
    PARAM8 *p8 = static_cast<PARAM8*>(params[7]);
    (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8);
  }

  template <class CLASSNAME, class RETVAL, class PARAM1, class PARAM2, class PARAM3, class PARAM4, class PARAM5, class PARAM6, class PARAM7, class PARAM8>
  void cb(RETVAL (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6, PARAM7, PARAM8), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    PARAM4 *p4 = static_cast<PARAM4*>(params[3]);
    PARAM5 *p5 = static_cast<PARAM5*>(params[4]);
    PARAM6 *p6 = static_cast<PARAM6*>(params[5]);
    PARAM7 *p7 = static_cast<PARAM7*>(params[6]);
    PARAM8 *p8 = static_cast<PARAM8*>(params[7]);
    *static_cast<RETVAL*>(retval) = (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8);
  }

  // 9 params
  template <class CLASSNAME, class PARAM1, class PARAM2, class PARAM3, class PARAM4, class PARAM5, class PARAM6, class PARAM7, class PARAM8, class PARAM9>
  void vcb(void (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6, PARAM7, PARAM8, PARAM9), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    PARAM4 *p4 = static_cast<PARAM4*>(params[3]);
    PARAM5 *p5 = static_cast<PARAM5*>(params[4]);
    PARAM6 *p6 = static_cast<PARAM6*>(params[5]);
    PARAM7 *p7 = static_cast<PARAM7*>(params[6]);
    PARAM8 *p8 = static_cast<PARAM8*>(params[7]);
    PARAM9 *p9 = static_cast<PARAM9*>(params[8]);
    (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9);
  }

  template <class CLASSNAME, class RETVAL, class PARAM1, class PARAM2, class PARAM3, class PARAM4, class PARAM5, class PARAM6, class PARAM7, class PARAM8, class PARAM9>
  void cb(RETVAL (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6, PARAM7, PARAM8, PARAM9), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    PARAM4 *p4 = static_cast<PARAM4*>(params[3]);
    PARAM5 *p5 = static_cast<PARAM5*>(params[4]);
    PARAM6 *p6 = static_cast<PARAM6*>(params[5]);
    PARAM7 *p7 = static_cast<PARAM7*>(params[6]);
    PARAM8 *p8 = static_cast<PARAM8*>(params[7]);
    PARAM9 *p9 = static_cast<PARAM9*>(params[8]);
    *static_cast<RETVAL*>(retval) = (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9);
  }

  // 10 params
  template <class CLASSNAME, class PARAM1, class PARAM2, class PARAM3, class PARAM4, class PARAM5, class PARAM6, class PARAM7, class PARAM8, class PARAM9, class PARAM10>
  void vcb(void (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6, PARAM7, PARAM8, PARAM9, PARAM10), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    PARAM4 *p4 = static_cast<PARAM4*>(params[3]);
    PARAM5 *p5 = static_cast<PARAM5*>(params[4]);
    PARAM6 *p6 = static_cast<PARAM6*>(params[5]);
    PARAM7 *p7 = static_cast<PARAM7*>(params[6]);
    PARAM8 *p8 = static_cast<PARAM8*>(params[7]);
    PARAM9 *p9 = static_cast<PARAM9*>(params[8]);
    PARAM10 *p10 = static_cast<PARAM10*>(params[9]);
    (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10);
  }

  template <class CLASSNAME, class RETVAL, class PARAM1, class PARAM2, class PARAM3, class PARAM4, class PARAM5, class PARAM6, class PARAM7, class PARAM8, class PARAM9, class PARAM10>
  void cb(RETVAL (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6, PARAM7, PARAM8, PARAM9, PARAM10), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    PARAM4 *p4 = static_cast<PARAM4*>(params[3]);
    PARAM5 *p5 = static_cast<PARAM5*>(params[4]);
    PARAM6 *p6 = static_cast<PARAM6*>(params[5]);
    PARAM7 *p7 = static_cast<PARAM7*>(params[6]);
    PARAM8 *p8 = static_cast<PARAM8*>(params[7]);
    PARAM9 *p9 = static_cast<PARAM9*>(params[8]);
    PARAM10 *p10 = static_cast<PARAM10*>(params[9]);
    *static_cast<RETVAL*>(retval) = (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10);
  }

  // 14 params
  template <class CLASSNAME, class PARAM1, class PARAM2, class PARAM3, class PARAM4, class PARAM5, class PARAM6, class PARAM7, class PARAM8, class PARAM9, class PARAM10, class PARAM11, class PARAM12, class PARAM13, class PARAM14>
  void vcb(void (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6, PARAM7, PARAM8, PARAM9, PARAM10, PARAM11, PARAM12, PARAM13, PARAM14), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    PARAM4 *p4 = static_cast<PARAM4*>(params[3]);
    PARAM5 *p5 = static_cast<PARAM5*>(params[4]);
    PARAM6 *p6 = static_cast<PARAM6*>(params[5]);
    PARAM7 *p7 = static_cast<PARAM7*>(params[6]);
    PARAM8 *p8 = static_cast<PARAM8*>(params[7]);
    PARAM9 *p9 = static_cast<PARAM9*>(params[8]);
    PARAM10 *p10 = static_cast<PARAM10*>(params[9]);
    PARAM11 *p11 = static_cast<PARAM11*>(params[10]);
    PARAM12 *p12 = static_cast<PARAM12*>(params[11]);
    PARAM13 *p13 = static_cast<PARAM13*>(params[12]);
    PARAM14 *p14 = static_cast<PARAM14*>(params[13]);
    (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10, *p11, *p12, *p13, *p14);
  }

  // 16 params
  template <class CLASSNAME, class PARAM1, class PARAM2, class PARAM3, class PARAM4, class PARAM5, class PARAM6, class PARAM7, class PARAM8, class PARAM9, class PARAM10, class PARAM11, class PARAM12, class PARAM13, class PARAM14, class PARAM15, class PARAM16>
  void vcb(void (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6, PARAM7, PARAM8, PARAM9, PARAM10, PARAM11, PARAM12, PARAM13, PARAM14, PARAM15, PARAM16), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    PARAM4 *p4 = static_cast<PARAM4*>(params[3]);
    PARAM5 *p5 = static_cast<PARAM5*>(params[4]);
    PARAM6 *p6 = static_cast<PARAM6*>(params[5]);
    PARAM7 *p7 = static_cast<PARAM7*>(params[6]);
    PARAM8 *p8 = static_cast<PARAM8*>(params[7]);
    PARAM9 *p9 = static_cast<PARAM9*>(params[8]);
    PARAM10 *p10 = static_cast<PARAM10*>(params[9]);
    PARAM11 *p11 = static_cast<PARAM11*>(params[10]);
    PARAM12 *p12 = static_cast<PARAM12*>(params[11]);
    PARAM13 *p13 = static_cast<PARAM13*>(params[12]);
    PARAM14 *p14 = static_cast<PARAM14*>(params[13]);
    PARAM15 *p15 = static_cast<PARAM15*>(params[14]);
    PARAM16 *p16 = static_cast<PARAM16*>(params[15]);
    (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10, *p11, *p12, *p13, *p14, *p15, *p16);
  }

  // 17 params
  template <class CLASSNAME, class PARAM1, class PARAM2, class PARAM3, class PARAM4, class PARAM5, class PARAM6, class PARAM7, class PARAM8, class PARAM9, class PARAM10, class PARAM11, class PARAM12, class PARAM13, class PARAM14, class PARAM15, class PARAM16, class PARAM17>
  void vcb(void (CLASSNAME::*fn)(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6, PARAM7, PARAM8, PARAM9, PARAM10, PARAM11, PARAM12, PARAM13, PARAM14, PARAM15, PARAM16, PARAM17), void *retval, void **params) {
    PARAM1 *p1 = static_cast<PARAM1*>(params[0]);
    PARAM2 *p2 = static_cast<PARAM2*>(params[1]);
    PARAM3 *p3 = static_cast<PARAM3*>(params[2]);
    PARAM4 *p4 = static_cast<PARAM4*>(params[3]);
    PARAM5 *p5 = static_cast<PARAM5*>(params[4]);
    PARAM6 *p6 = static_cast<PARAM6*>(params[5]);
    PARAM7 *p7 = static_cast<PARAM7*>(params[6]);
    PARAM8 *p8 = static_cast<PARAM8*>(params[7]);
    PARAM9 *p9 = static_cast<PARAM9*>(params[8]);
    PARAM10 *p10 = static_cast<PARAM10*>(params[9]);
    PARAM11 *p11 = static_cast<PARAM11*>(params[10]);
    PARAM12 *p12 = static_cast<PARAM12*>(params[11]);
    PARAM13 *p13 = static_cast<PARAM13*>(params[12]);
    PARAM14 *p14 = static_cast<PARAM14*>(params[13]);
    PARAM15 *p15 = static_cast<PARAM15*>(params[14]);
    PARAM16 *p16 = static_cast<PARAM16*>(params[15]);
    PARAM17 *p17 = static_cast<PARAM17*>(params[16]);
    (static_cast<CLASSNAME *>(this)->*fn)(*p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10, *p11, *p12, *p13, *p14, *p15, *p16, *p17);
  }

};


#define RECVS_MULTIPATCH int _multipatch(int patch, int msg, void *retval, void **params=0, int nparam=0)
#define START_MULTIPATCH \
  int CBCLASS::_multipatch(int patch, int msg, void *retval, void **params, int nparam) { \
    switch (patch) {
//FINISH      case DESTRUCT: delete this; return 1;
#define END_MULTIPATCH \
      default: return 0; \
    } \
    return 1; \
  }
#define FORWARD_MULTIPATCH(x) \
      default: return x::_multipatch(patch, msg, retval, params, nparam); \
    } \
    return 1; \
  }

#define START_PATCH(x) case x: switch(msg) {
#define END_PATCH default: return 0; } break;
#define NEXT_PATCH(x) END_PATCH START_PATCH(x)

#define M_CB(p, c, x, y) case (x): MultiPatch<p,c>::cb(&CBCLASS::y, retval, params); break;
#define M_VCB(p, c, x, y) case (x): MultiPatch<p,c>::vcb(&CBCLASS::y, retval, params); break;


#define MULTIPATCH_CODES enum
#endif


/* use example:

enum 
{
	Test_Patch1 = 10,
	Test_Patch2 = 20,
};

class Test : public MultiPatch<Test_Patch1, api_class1>,
	           public MultiPatch<Test_Patch2, api_class2>
{
public:
	int TestFunc1();
	void TestFunc2();
	void TestFunc3();
protected:
	RECVS_MULTIPATCH;
};

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS Test
START_MULTIPATCH;
 START_PATCH(Test_Patch1)
        M_CB(Test_Patch1, api_class1, API_CLASS1_FUNC1, TestFunc1);
  NEXT_PATCH(Test_Patch2)
       M_VCB(Test_Patch2, api_class2, API_CLASS1_FUNC1, TestFunc2);
       M_VCB(Test_Patch2, api_class2, API_CLASS1_FUNC2, TestFunc3);
   END_PATCH
END_MULTIPATCH;
#undef CBCLASS

*/