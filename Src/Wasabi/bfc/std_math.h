#ifndef _STD_MATH_H
#define _STD_MATH_H

// FG> doesn't work for me without this include (error C2039: 'sin' : is not a member of '`global namespace'')
#include <math.h>
#include <bfc/platform/types.h>
#ifdef __cplusplus
static inline double SIN(double a) { return ::sin(a); }
static inline double COS(double a) { return ::cos(a); }
static inline double SQRT(double a) { return ::sqrt(a); }

#else
#define SIN(a) sin(a)
#define COS(a) sin(a)
#define SQRT(a) sqrt(a)
unsigned long COMEXP BSWAP_C(unsigned long input);
#endif

#ifdef __cplusplus
// neat trick from C++ book, p. 161
template<class T> inline T MAX(T a, T b) { return a > b ? a : b; }
template<class T> inline T MIN(T a, T b) { return a > b ? b : a; }
template<class T> inline T MINMAX(T a, T minval, T maxval) {
  return (a < minval) ? minval : ( (a > maxval) ? maxval : a );
}

// and a couple of my own neat tricks :) BU
template<class T> inline T ABS(T a) { return a < 0 ? -a : a; }
template<class T> inline T SQR(T a) { return a * a; }
template<class T> inline int CMP3(T a, T b) {
  if (a < b) return -1;
  if (a == b) return 0;
  return 1;
}

static inline RGB24 RGBTOBGR(RGB24 col) {
  return ((col & 0xFF00FF00) | ((col & 0xFF0000) >> 16) | ((col & 0xFF) << 16));
}
static inline RGB24 BGRTORGB(RGB24 col) { return RGBTOBGR(col); }
static inline ARGB32 BGRATOARGB(ARGB32 col) { return RGBTOBGR(col); }

void premultiplyARGB32(ARGB32 *words, int nwords=1);

#else // not __cplusplus
void COMEXP premultiplyARGB32(ARGB32 *words, int nwords);
#endif

#endif
