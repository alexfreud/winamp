#include "out_ds.h"
#include <dinput.h>
#include <math.h>
static IDirectInput8 * pDI;
static IDirectInputDevice8 * pDev;


#ifndef HAVE_JOY
#error nein!
#endif

static BOOL CALLBACK eCallback(LPCDIDEVICEINSTANCE dev,void * foop)
{
	*(GUID*)foop=dev->guidInstance;
	return DIENUM_STOP;
}

static bool captured;

#define joy_id JOYSTICKID1

#define POLL 5




void wa2_hack_setpitch(double d);

static double joy_read()
{
	DIJOYSTATE2 stat;
	if (SUCCEEDED(pDev->GetDeviceState(sizeof(stat),&stat)))
	{
		return pow(2,(double)stat.lX/(double)0x8000)/2.0;
	}
	else return 1;
}

void wa2_hack_joy_update()
{
	wa2_hack_setpitch(joy_read());
}

static HANDLE hThread;
static bool die;

static DWORD _stdcall joy_thread(void*)
{
	while(!die)
	{
		wa2_hack_setpitch(joy_read());
		Sleep(10);		
	}
	return 0;
}

void wa2_hack_joy_init()
{
	if (!hThread)
	{
		DirectInput8Create(mod.hDllInstance,DIRECTINPUT_VERSION,IID_IDirectInput8,(void**)&pDI,0);
		if (pDI)
		{
			GUID foop;
			pDI->EnumDevices(DI8DEVCLASS_GAMECTRL,eCallback,&foop,DIEDFL_ATTACHEDONLY);
			pDI->CreateDevice(foop,&pDev,0);
			if (pDev)
			{
				pDev->SetDataFormat(&c_dfDIJoystick2);
				DIPROPDWORD dw;
				dw.dwData=1000;
				dw.diph.dwSize=sizeof(DIPROPDWORD);
				dw.diph.dwHeaderSize=sizeof(DIPROPHEADER);
				dw.diph.dwObj=0;
				dw.diph.dwHow=DIPH_DEVICE;
				pDev->SetProperty(DIPROP_DEADZONE,&dw.diph);
				pDev->SetCooperativeLevel(mod.hMainWindow,DISCL_BACKGROUND);
				pDev->Acquire();

				die=0;
				DWORD id;
				hThread=CreateThread(0,0,joy_thread,0,0,&id);
				SetThreadPriority(hThread,THREAD_PRIORITY_TIME_CRITICAL);
			}
			else {pDI->Release();pDI=0;}
		}
	}
}

void wa2_hack_joy_deinit()
{
	if (hThread)
	{
		die=1;
		WaitForSingleObject(hThread,INFINITE);
		CloseHandle(hThread);
		hThread=0;
		if (pDev) {pDev->Unacquire();pDev->Release();pDev=0;}
		if (pDI) {pDI->Release();pDI=0;}
	}
}