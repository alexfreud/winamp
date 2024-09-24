#ifndef NULLSOFT_ML_DISC_REPLAYGAIN_H
#define NULLSOFT_ML_DISC_REPLAYGAIN_H

#include <windows.h>
#include "../ml_rg/obj_replaygain.h"

void CALLBACK StartGain(ULONG_PTR data);
void CALLBACK WriteGain(ULONG_PTR data);
void CALLBACK CalculateGain(ULONG_PTR data);
void CALLBACK CloseGain(ULONG_PTR data);
void CALLBACK QuitThread(ULONG_PTR data);
void CreateGain();

extern HANDLE rgThread;

#endif