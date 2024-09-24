#ifndef _BITLIST_H
#define _BITLIST_H

#include "memblock.h"

class BitList {
public:
  BitList(unsigned int size=0) {
    setSize(size);
  }

  int getitem(int n) const { 
    if (n < 0 || n >= m_size) return 0; 
    return (m_list[n>>3]>>(n&7))&1;
  }
  void setitem(int n, int v) {
    if (n >= 0 && n < m_size) {
      int lv=1<<(n&7);
      if (v) m_list[n>>3]|=lv;
      else m_list[n>>3]&=~lv;
    }
  }
  
  int operator[](int n) const { return getitem(n); }

  int getSize() const {	// in bits
    return m_size;
  }
  void setSize(unsigned int newsize) {
    m_list.setSize((newsize+7)>>3);
    m_size = newsize;
  }
  
  int getsize() const { return m_size; }

private:
  MemBlock< uint8_t> m_list;
  int m_size;
};

#endif
