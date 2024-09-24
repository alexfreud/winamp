#pragma once

#include <bfc/dispatch.h>
#include <windows.h> //need this for Sleep()

class ifc_authcallback : public Dispatchable
{
protected:
	ifc_authcallback() {}
	~ifc_authcallback() {}
public:
	int OnConnecting();
	int OnSending();
	int OnReceiving();
	// pump your message loop for a little while
	int OnIdle();

	enum
	{
		ONCONNECTING=0,
		ONSENDING=1,
		ONRECEIVING=2,
		ONIDLE=3,
	};
};

inline int ifc_authcallback::OnConnecting()
{
	return _call(ONCONNECTING, (int)0);
}
inline int ifc_authcallback::OnSending()
{
	return _call(ONSENDING, (int)0);
}
inline int ifc_authcallback::OnReceiving()
{
	return _call(ONRECEIVING, (int)0);
}
inline int ifc_authcallback::OnIdle()
{
	int retval;
  if (_dispatch(ONIDLE, &retval)) 
		return retval;
	else
	{
		// default implementation
		Sleep(50);
		return 0; 
	}

}
