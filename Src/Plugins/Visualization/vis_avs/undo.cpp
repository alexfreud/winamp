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
#include <windows.h>
#include "undo.h"
#include "render.h"

int C_UndoStack::list_pos;
C_UndoItem *C_UndoStack::list[256];

C_UndoItem::C_UndoItem() : data(NULL), length(0), isdirty(true)
{
}

C_UndoItem::C_UndoItem(const C_UndoItem& T) : data(NULL), length(0), isdirty(true)
{
  *this = T;
}

C_UndoItem::C_UndoItem(void *_data, int _length, bool _isdirty) : data(NULL), length(length), isdirty(_isdirty)
{
  data = GlobalAlloc(GPTR, length);
  memcpy(data, _data, length);
}

C_UndoItem::~C_UndoItem()
{
  if (data) 
  {
    GlobalFree(data);
  }
}

C_UndoItem & C_UndoItem::operator = (const C_UndoItem& T)
{
  length = T.length;
  isdirty = T.isdirty;
  if (data) 
  {
    GlobalFree(data);
  }
  data = GlobalAlloc(GPTR, length);
  memcpy(data, T.data, length);
  return *this;
}

bool C_UndoItem::operator == (const C_UndoItem& T) const
{
  bool retval = false;
  if (length == T.length)
  {
    retval = (memcmp(data, T.data, length) == 0);
  }
  return retval;
}

void C_UndoItem::set(void *_data, int _length, bool _isdirty)
{
  length = _length;
  isdirty = _isdirty;
  if (data) 
  {
    GlobalFree(data);
  }
  data = GlobalAlloc(GPTR, length);
  memcpy(data, _data, length);
}


void C_UndoStack::saveundo(int is2)
{
  // Save to the undo buffer (sets the dirty bit for this item)
  C_UndoItem *item = new C_UndoItem;
  C_UndoItem *old = list[list_pos];

  if (is2) 
  {
    g_render_effects2->__SavePresetToUndo(*item);
  }
  else 
  {
    g_render_effects->__SavePresetToUndo(*item);
  }

  // Only add it to the stack if it has changed.
  if (!old || !(*old == *item))
  {
    if (list_pos == sizeof(list)/sizeof(list[0])-1)
    {
      delete list[0];
      memcpy(list,list+1,sizeof(list)/sizeof(list[0])-1);
      list_pos--;
    }
    list[++list_pos]=item;
  }
  else delete item;
}

void C_UndoStack::cleardirty()
{
  // If we're clearing the dirty bit, we only clear it on the current item.
  if (list_pos >= 0 && list_pos < sizeof(list)/sizeof(list[0]) && list[list_pos])
  {
    list[list_pos]->isdirty = 0;
  }
}

bool C_UndoStack::isdirty()
{
  if (list_pos >= 0 && list_pos < sizeof(list)/sizeof(list[0]) && list[list_pos])
    return list[list_pos]->isdirty;
  return false;
}

int C_UndoStack::can_undo()
{
  return (list_pos>0 && list[list_pos-1]);
}

int C_UndoStack::can_redo()
{
  return list_pos < sizeof(list)/sizeof(list[0])-1 && list[list_pos+1];
}


void C_UndoStack::undo()
{
  if (list_pos>0 && list[list_pos-1])
  {
    g_render_transition->LoadPreset(NULL,0,list[--list_pos]);
  }
}

void C_UndoStack::redo()
{
  if (list_pos < sizeof(list)/sizeof(list[0])-1 && list[list_pos+1])
  {
    g_render_transition->LoadPreset(NULL,0,list[++list_pos]);
  }
}

void C_UndoStack::clear()
{
  list_pos=0;
  int x;
  for (x = 0; x < sizeof(list)/sizeof(list[0]); x ++)
  {
    delete list[x];
    list[x]=0;
  }
}
