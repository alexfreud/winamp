#ifndef _TIMING_H_
#define _TIMING_H_


//#define TIMING


#if defined(TIMING) && !defined(__alpha)
void _timingPrint(void);
void _timingInit(void);
void _timingLeave(int);
void _timingEnter(int);
#define timingPrint() _timingPrint()
#define timingInit() _timingInit()
#define timingLeave(x) _timingLeave(x)
#define timingEnter(x) _timingEnter(x)
#else
#define timingPrint()
#define timingInit()
#define timingLeave(x)
#define timingEnter(x)
#endif

#endif