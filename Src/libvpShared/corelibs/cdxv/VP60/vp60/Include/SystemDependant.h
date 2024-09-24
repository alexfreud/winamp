/****************************************************************************
*
*   Module Title :     SystemDependant.h
*
*   Description  :     Miscellaneous system dependant functions header
*
****************************************************************************/
#ifndef __INC_SYSTEMDEPENDANT_H
#define __INC_SYSTEMDEPENDANT_H

/****************************************************************************
*  Exports
****************************************************************************/
extern void VP6_IssueWarning ( char * WarningMessage );
extern void PauseProcess ( unsigned int SleepMs );

// System dynamic memory allocation
char *SytemGlobalAlloc ( unsigned int Size );   
void SystemGlobalFree ( char * MemPtr );

#endif
