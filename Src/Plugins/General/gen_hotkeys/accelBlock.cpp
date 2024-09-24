#include "gen_hotkeys.h"
#include "api__gen_hotkeys.h"
#include "./accelBlock.h"

static BOOL RegisterMessageProcessor(ifc_messageprocessor *processor, BOOL bRegister)
{
	if (NULL == WASABI_API_APP)
		return FALSE;
	
	if (bRegister)
		WASABI_API_APP->app_addMessageProcessor(processor);
	else 
		WASABI_API_APP->app_removeMessageProcessor(processor);
		
	return TRUE;
}

AcceleratorBlocker::AcceleratorBlocker(HWND hwndToBlock) : hwnd(hwndToBlock)
{
	RegisterMessageProcessor(this, TRUE);
}
AcceleratorBlocker::~AcceleratorBlocker()
{
	RegisterMessageProcessor(this, FALSE);
}

bool AcceleratorBlocker::ProcessMessage(MSG *pMsg)
{
	if (pMsg->hwnd != hwnd)
		return false;

	switch(pMsg->message)
	{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			TranslateMessage(pMsg);
			DispatchMessageW(pMsg);
			return true;
	}
	return false;
}


#define CBCLASS AcceleratorBlocker
START_DISPATCH;
CB(IFC_MESSAGEPROCESSOR_PROCESS_MESSAGE, ProcessMessage)
END_DISPATCH;