#ifndef _RESIZABLE_H
#define _RESIZABLE_H

// {834F172D-FE95-4324-B0EE-BCE29886D7BE}
static const GUID guiResizableGuid = 
{ 0x834f172d, 0xfe95, 0x4324, { 0xb0, 0xee, 0xbc, 0xe2, 0x98, 0x86, 0xd7, 0xbe } };

class GuiResizable {
public:
  virtual void beginMove()=0;
  virtual void beginScale()=0;
  virtual void beginResize()=0;
  virtual void endMove()=0;
  virtual void endScale()=0;
  virtual void endResize()=0;
  virtual ifc_window *guiresizable_getRootWnd()=0;
  virtual void setEndMoveResize(int w, int h)=0;
};


#endif
