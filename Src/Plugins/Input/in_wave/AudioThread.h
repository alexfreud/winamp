#ifndef NULLSOFT_AUDIOTHREADH
#define NULLSOFT_AUDIOTHREADH

#include <windows.h>

VOID CALLBACK APCSeek( ULONG_PTR p_data );
VOID CALLBACK APCPause( ULONG_PTR p_data );
VOID CALLBACK APCStart( ULONG_PTR p_data );
VOID CALLBACK APCStop( ULONG_PTR p_data );

void Kill();
void AudioThreadInit();
void AudioThreadQuit();

extern HANDLE audioThread;
extern HANDLE stopped;
extern HANDLE events[2];

#endif