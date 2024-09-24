#ifndef _LINKEDQUEUE_H_
#define _LINKEDQUEUE_H_

#include <windows.h>

class LinkedQueue;
class QueueElement;

class QueueElement {
public:
  QueueElement * next;
  QueueElement * prev;
  void * elem;
  QueueElement(void * e) { next=NULL; prev=NULL; elem=e; }
};


class LinkedQueue {
protected:
  QueueElement * head;
  QueueElement * tail;
  QueueElement * bm;
  int bmpos;
  int size;
  QueueElement * Find(int pos);
  CRITICAL_SECTION cs;
public:
  LinkedQueue();
  ~LinkedQueue();
  int GetSize();
  void Offer(void * e);
  void *Poll();
  void *Peek();
  void *Get(int pos);
  void Set(int pos, void * val);
  void *Del(int pos);
  void lock();
  void unlock();
};

#endif //_LINKEDQUEUE_H_