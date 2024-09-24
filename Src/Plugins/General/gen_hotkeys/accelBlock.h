#ifndef NULLSOFT_WINAMP_ACCELERATOR_BLOCKER_HEADER
#define NULLSOFT_WINAMP_ACCELERATOR_BLOCKER_HEADER

#include <wtypes.h>
#include <api\application\ifc_messageprocessor.h>

class AcceleratorBlocker : public ifc_messageprocessor
{
public:
	AcceleratorBlocker(HWND hwndToBlock);
	~AcceleratorBlocker();

public:
	bool ProcessMessage(MSG *pMsg); 
	HWND GetHwnd() { return hwnd; }
protected:
	RECVS_DISPATCH;

protected:
	HWND hwnd;

};

#endif// NULLSOFT_WINAMP_ACCELERATOR_BLOCKER_HEADER