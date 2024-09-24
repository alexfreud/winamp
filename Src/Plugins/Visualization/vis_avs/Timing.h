
#ifndef _TIMING_H_
#define _TIMING_H_


//#define TIMING



#if defined(TIMING) && !defined(__alpha)
#ifdef __cplusplus
extern "C" {
#endif
void _timingInit(void);
void _timingPrint(void);
void _timingEnter(int);
void _timingLeave(int);
#ifdef __cplusplus
}
#endif
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