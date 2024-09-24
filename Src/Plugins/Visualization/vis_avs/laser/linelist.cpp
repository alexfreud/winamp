/*
  LICENSE
  -------
Copyright 2005 Nullsoft, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer. 

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution. 

  * Neither the name of Nullsoft nor the names of its contributors may be used to 
    endorse or promote products derived from this software without specific prior written permission. 
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#ifdef LASER
#include <windows.h>
#include "linelist.h"

class C_LineList : public C_LineListBase 
{
private:
  int max;
  int used;
  LineType *linelist;
public:
  C_LineList() { max=16384; linelist=(LineType *)malloc(16384*sizeof(LineType)); used=0; };
  virtual ~C_LineList() { free(linelist); }
  virtual int GetMaxLines(void) { return max; };
  virtual int GetUsedLines(void) { return used; };
  virtual void SetUsedLines(int usedlines) { used=usedlines; }
  virtual void SetLines(LineType *list, int start, int length) { memcpy(linelist+start,list,length*sizeof(LineType)); }
  virtual void SetMaxLines(int m);
  virtual void ClearLineList(void);
  virtual LineType *GetLineList(void);
  virtual void AddLine(LineType *line);

  void swapcontents(C_LineList *other)
  {
    {
      int tmp;
      tmp=max;
      max=other->max;
      other->max=tmp;

      tmp=used;
      used=other->used;
      other->used=tmp;
    }
    LineType *tmp;
    tmp=linelist;
    linelist=other->linelist;
    other->linelist=tmp;
  }

};

C_LineListBase *createLineList(void)
{
  return (C_LineListBase *)new C_LineList();
}

void LineListSwap(C_LineListBase *item1, C_LineListBase *item2)
{
  C_LineList *i1=(C_LineList*)item1;
  C_LineList *i2=(C_LineList*)item2;
  i1->swapcontents(i2);
}

void C_LineList::SetMaxLines(int m)
{
  if (m > 16384) m = 16384;
  max=m;
  if (used > max) ClearLineList();
}

void C_LineList::ClearLineList(void)
{
  used=0;
}

LineType *C_LineList::GetLineList(void)
{
  return linelist;
}

void C_LineList::AddLine(LineType *line)
{
  if (used >= max) return;
  memcpy(linelist+used,line,sizeof(LineType));
  used++;
}
#endif