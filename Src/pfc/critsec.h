#ifndef _PFC_CRITSEC_H_
#define _PFC_CRITSEC_H_

class critical_section : public CRITICAL_SECTION
{
public:
	inline void enter() {EnterCriticalSection(this);}
	inline void leave() {LeaveCriticalSection(this);}
	critical_section() {InitializeCriticalSection(this);}
	~critical_section() {DeleteCriticalSection(this);}
	//BOOL TryEnter() {return TryEnterCriticalSection(this);}
};

#endif