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
#ifndef _UNDO_H_
#define _UNDO_H_

class C_UndoStack;

class C_UndoItem
{
  friend C_UndoStack;
  public:
    C_UndoItem();
    ~C_UndoItem();
    C_UndoItem(const C_UndoItem& T);
    C_UndoItem(void *data, int length, bool isdirty);

    C_UndoItem & operator = (const C_UndoItem& T);
    bool operator == (const C_UndoItem& T) const;

    void set(void *data, int length, bool isdirty);
    void *get() const { return data; }
    int size() const { return length; }

  private:
    void *data;
    int length;
    bool isdirty;
};

class C_UndoStack
{
  public:
    static void saveundo(int is2=0);
    static void cleardirty();
    static bool isdirty();

    static void undo();
    static void redo();

    static int can_undo();
    static int can_redo();

    static void clear();

  private:

    // sorry to do this mig, but that doubly linked lists made me scared. I think it
    // wasn't actually the source of my bug (I later fixed it), but doubly linked lists
    // are just plain hard to get right. :)
    static int list_pos;
    static C_UndoItem *list[256]; // only keep 256 elements in list at a time
};

#endif//_UNDO_H_