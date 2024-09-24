#ifndef _EDITWNDSTRING_H
#define _EDITWNDSTRING_H

#include <api/wnd/wndclass/editwnd.h>
#include <bfc/memblock.h>

class EditWndString : public EditWnd 
{
public:
	void setBuffer(wchar_t *buffer, int len=0) 
	{
		b.setSize(len+1);
		wchar_t *bufmem=b.getMemory();
		if(len) 
			wcsncpy(bufmem,buffer,len);
		bufmem[len]=0;
		EditWnd::setBuffer(bufmem,len);
	}
	const wchar_t *getBuffer() { return b.getMemory(); }
private:
	MemBlock<wchar_t> b;
};

#endif//_EDITWNDSTRING_H