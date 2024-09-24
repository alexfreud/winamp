// The authors have released ID3Lib as Public Domain(PD) and claim no copyright,
// patent or other intellectual property protection in this work. This means that
// it may be modified, redistributed and used in commercial and non-commercial
// software and hardware without restrictions. ID3Lib is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
// 
// The ID3Lib authors encourage improvements and optimisations to be sent to the
// ID3Lib coordinator, currently Dirk Mahoney(dirk@id3.org). Approved
// submissions may be altered, and will be included and released under these terms.
// 
// Mon Nov 23 18:34:01 1998
// improved/optimized/whatEVER jan-08-2006 benski

#include "id3_tag.h"
#include "id3_misc_support.h"
#include "../nu/AutoWide.h"

ID3_Elem *ID3_Tag::Find(ID3_Frame *frame)
{
  ID3_Elem *cur = frameList;

  while(cur)
  {
    if (cur->frame == frame) 
      return cur;
    cur = cur->next;
  }

  return NULL;
}


ID3_Frame *ID3_Tag::Find(ID3_FrameID id)
{
  ID3_Elem *cur = findCursor;

  if (cur == NULL)
    findCursor = cur = frameList;

  while(cur)
  {
    if (cur->frame && cur->frame->GetID() == id)
    {
      findCursor = cur->next;
      return cur->frame;
    }
    cur = cur->next;

    if (cur == NULL) cur = frameList;
    if (cur == findCursor) break;
  }

  return NULL;
}

ID3_Frame *ID3_Tag::Find(ID3_FrameID id, ID3_FieldID fld, const wchar_t *data)
{
  ID3_Elem *cur = findCursor;

  if (cur == NULL)
    findCursor = cur = frameList;

  while(cur)
  {
    if (cur->frame && cur->frame->GetID() == id)
    {
      if (data && /*lstrlenW(data) &&*/ BS_ISSET(cur->frame->fieldBits, fld))
      {
        wchar_t *buffer;
        luint size;

        size = cur->frame->Field(fld).BinSize();

        if (buffer =(wchar_t*)calloc(size, sizeof(wchar_t)))
        {
          buffer[0]=0;
          cur->frame->Field(fld).GetUnicode(buffer, size);

          if (wcscmp(buffer, data) == 0)
          {
            free(buffer);
            findCursor = cur->next;
            return cur->frame;
          }

          free(buffer);
        }
      }
    }
    cur = cur->next;

    if (cur == NULL) cur = frameList;

    if (cur == findCursor) break;
  }

  return NULL;
}



ID3_Frame *ID3_Tag::Find(ID3_FrameID id, ID3_FieldID fld, const char *data)
{
  ID3_Elem *cur = findCursor;

  if (cur == NULL)
    findCursor = cur = frameList;

  while(cur)
  {
    if (cur->frame && cur->frame->GetID() == id)
    {
      if (data && /*lstrlenW(data) &&*/ BS_ISSET(cur->frame->fieldBits, fld))
      {
        char *buffer;
        luint size;

        size = cur->frame->Field(fld).BinSize();

        if (buffer =(char*)calloc(size, sizeof(char)))
        {
          buffer[0]=0;
          cur->frame->Field(fld).GetLocal(buffer, size);

          if (strcmp(buffer, data) == 0)
          {
            free(buffer);
            findCursor = cur->next;
            return cur->frame;
          }

          free(buffer);
        }
      }
    }
    cur = cur->next;

    if (cur == NULL) cur = frameList;

    if (cur == findCursor) break;
  }

  return NULL;
}



ID3_Frame *ID3_Tag::Find(ID3_FrameID id, ID3_FieldID fld, luint data)
{
  ID3_Elem *cur = findCursor;

  if (cur == NULL)
    findCursor = cur = frameList;

  while(cur)
  {
    if (cur->frame &&(cur->frame->GetID() == id))
    {
      if (cur->frame->Field(fld).Get() == data)
      {
        findCursor=cur->next;
        return cur->frame;
      }
    }
    cur = cur->next;

    if (cur == NULL) cur = frameList;
    if (cur == findCursor) break;
  }

  return NULL;
}


ID3_Frame *ID3_Tag::GetFrameNum(luint num)
{
  luint curNum=0;
  ID3_Elem *cur = frameList;

  while(cur)
  {
    if (num == curNum)
    {
      return cur->frame;
    }
    curNum++;
    cur = cur->next;
  }

  return 0;
}


ID3_Frame *ID3_Tag::operator[](luint num)
{
  return GetFrameNum(num);
}


