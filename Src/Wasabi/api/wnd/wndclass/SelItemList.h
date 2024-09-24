#ifndef NULLSOFT_SELITEMLISTH
#define NULLSOFT_SELITEMLISTH

class ListWnd;

#define SELITEMEXPAND 2040

//note to whoever redid this as a bitlist
// a) this is pointlessly slow as a bitlist given the memory used
// b) perhaps you should investigate bitlist.h
class SelItemList
{
public:
  SelItemList(ListWnd *parent);

  void setSelected(int pos, int selected, int cb=1);
  int isSelected(int pos);
  int getNumSelected();

  void deleteByPos(int pos);
protected:
friend ListWnd;
  void deselectAll();

private:
  ListWnd *listwnd;
  MemBlock<char> list;
  int num_selected;
};

#endif