#pragma once

#include "syscb/ifc_syscallback.h"
#include "foundation/mkncc.h"

namespace Service 
{
	// {215CDE06-22A6-424F-9C64-DEDC45D84455}
	static const GUID event_type = 
	{ 0x215cde06, 0x22a6, 0x424f, { 0x9c, 0x64, 0xde, 0xdc, 0x45, 0xd8, 0x44, 0x55 } };
	static const int on_register = 0;
	static const int on_deregister = 1;

	class SystemCallback : public ifc_sysCallback
	{
	protected:
		GUID WASABICALL SysCallback_GetEventType() { return event_type; }
		int WASABICALL SysCallback_Notify(int msg, intptr_t param1, intptr_t param2)
		{
			const GUID *service_id;

			switch(msg)
			{
			case on_register:
				service_id = (const GUID *)param1;
				return ServiceSystemCallback_OnRegister(*service_id, (void *)param2);
			case on_deregister:
				service_id = (const GUID *)param1;
				return ServiceSystemCallback_OnDeregister(*service_id, (void *)param2);
			default:
				return NErr_Success;
			}
		}
		virtual int WASABICALL ServiceSystemCallback_OnRegister(GUID service_id, void *service) { return NErr_Success; }
		virtual int WASABICALL ServiceSystemCallback_OnDeregister(GUID service_id, void *service) { return NErr_Success; }
	};

}
