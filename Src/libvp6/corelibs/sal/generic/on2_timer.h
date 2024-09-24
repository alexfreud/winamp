#ifndef _ON2_TIMER_H_
#define _ON2_TIMER_H_

/************************************************************************/
/* on2Timer: cross-platform timer, works on win32 and linux so far     */
/* started: August 14, 2001										        */
/* codemonkey: TJF												        */
/************************************************************************/

#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct on2Timer_ {
	unsigned long baseMilli;
	unsigned long elapsedMilli;
} on2Timer;




/****************************************************************/
/*	void  on2Timer_Init ( on2Timer* pTimer )  					*/
/* 	initialize an allocated timer's members to 0 				*/
/****************************************************************/
void  on2Timer_Init ( on2Timer* pTimer );

/****************************************************************/
/*	unsigned long on2Timer_Start ( on2Timer* pTimer )			*/
/* 	start a timer: sets baseTime to currentTime in milliseconds	*/
/****************************************************************/
unsigned long on2Timer_Start ( on2Timer* pTimer );

/****************************************************************/
/*	unsigned long on2Timer_Stop ( on2Timer* pTimer )			*/
/* 	stop a timer: sets elapsed time and accounts for rollover	*/
/****************************************************************/
unsigned long on2Timer_Stop ( on2Timer* pTimer );

/********************************************************************************/
/*	unsigned long on2Timer_GetCurrentElapsedMilli ( on2Timer* pTimer )	        */
/* 	get current elapsed time: returns elapsed time and accounts for rollover	*/
/********************************************************************************/
unsigned long on2Timer_GetCurrentElapsedMilli ( on2Timer* pTimer );

/********************************************************************************/
/*	unsigned long on2Timer_GetMilliBeforeRollover ( unsigned long baseMilli )	*/
/* 	returns milliseconds elapsed since rollover occurred					    */
/********************************************************************************/
unsigned long on2Timer_GetMilliBeforeRollover ( unsigned long baseMilli );

/****************************************************************/
/*	unsigned long on2Timer_GetCurrentTimeSeconds ( void )		*/
/* 	returns seconds since midnight								*/
/****************************************************************/
unsigned long on2Timer_GetCurrentTimeSeconds ( void );

/****************************************************************/
/*	unsigned long on2Timer_GetCurrentTimeMilli ( void )			*/
/* 	returns milliseconds since midnight							*/
/****************************************************************/
unsigned long on2Timer_GetCurrentTimeMilli ( void );

/****************************************************************/
/*	void on2Timer_DeInit ( on2Timer* pTimer );					*/
/* 	on2_free's a pointer to a on2Timer struct, for the lazy ;-) */
/****************************************************************/
void on2Timer_DeInit ( on2Timer* pTimer );

/****************************************************************/
/*	void on2Timer_Sleep ( int msec );							*/
/* 	Sleeps for the passed in number of milliseconds				*/
/****************************************************************/
int on2Timer_Sleep( int msec );



#if defined(__cplusplus)
}
#endif

#endif /* #ifndef _ON2_TIMER_H_ */
