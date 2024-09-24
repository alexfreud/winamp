#ifndef NULLSOFT_WASABI_MESSAGEPROCESSORI_H
#define NULLSOFT_WASABI_MESSAGEPROCESSORI_H

#include <api/application/ifc_messageprocessor.h>

class ifc_messageprocessorI: public ifc_messageprocessor
{
public:
	virtual bool ProcessMessage(MSG *msg)=0;	
protected:
	RECVS_DISPATCH;
};
#endif