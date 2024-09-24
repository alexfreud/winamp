#include "LinkedQueue.h"

LinkedQueue::LinkedQueue()
{
	size=0;
	head=NULL;
	tail=NULL;
	bm=NULL;
	bmpos=0;
	InitializeCriticalSection(&cs);
}

void LinkedQueue::lock()
{
	EnterCriticalSection(&cs);
	//wchar_t buf[100]; wsprintf(buf,L"Lock taken by %x",GetCurrentThreadId()); OutputDebugString(buf);
}
void LinkedQueue::unlock()
{
	LeaveCriticalSection(&cs);
	//wchar_t buf[100]; wsprintf(buf,L"Lock released by %x",GetCurrentThreadId()); OutputDebugString(buf);
}

LinkedQueue::~LinkedQueue()
{
	lock();
	QueueElement * q=head;
	while(q) { QueueElement *p=q; q=q->next; delete p; }
	unlock();
	DeleteCriticalSection(&cs);
}

void LinkedQueue::Offer(void * e)
{
	lock();
	if(size==0) { size++; head=tail=new QueueElement(e); unlock(); return; }
	tail->next=new QueueElement(e);
	tail->next->prev=tail;
	tail=tail->next;
	size++;
	bm=NULL;
	unlock();
}

void * LinkedQueue::Poll()
{
	lock();
	if(size == 0) { unlock(); return NULL; }
	size--;
	void * r = head->elem;
	QueueElement * q = head;
	head=head->next;
	if(head!=NULL) head->prev=NULL;
	else tail=NULL;
	delete q;
	bm=NULL;
	unlock();
	return r;
}

void * LinkedQueue::Peek()
{
	lock();
	void * ret=head?head->elem:NULL;
	unlock();
	return ret;
}

QueueElement * LinkedQueue::Find(int x)
{
	if(x>=size || x<0) return NULL;
	if(x == 0) return head;
	if(x == size-1) return tail;
	if(!bm) { bm=head; bmpos=0; }
	int diffh = x;
	int difft = (size-1) - x;
	int diffbm = x - bmpos;
	diffbm>0?diffbm:-diffbm;
	if(diffh < difft && diffh < diffbm) { bm=head; bmpos=0; }
	else if(diffh >= difft && diffbm >= difft) { bm=tail; bmpos=size-1; }
	while(bmpos > x && bm) { bm=bm->prev; bmpos--; }
	while(bmpos < x && bm) { bm=bm->next; bmpos++; }
	return bm;
}

void * LinkedQueue::Get(int pos)
{
	lock();
	QueueElement * e = Find(pos);
	unlock();
	return e?e->elem:NULL;
}

void LinkedQueue::Set(int pos, void * val)
{
	lock();
	QueueElement * e = Find(pos);
	if(e) e->elem=val;
	unlock();
}

void* LinkedQueue::Del(int pos)
{
	lock();
	QueueElement * e = Find(pos);
	if(!e) { unlock(); return NULL; }
	else if(size == 1) head=tail=NULL;
	else if(e==head)
	{
		head=head->next;
		head->prev=NULL;
	}
	else if(e==tail)
	{
		tail=tail->prev;
		tail->next=NULL;
	}
	else
	{
		e->prev->next = e->next;
		e->next->prev = e->prev;
	}
	size--;
	bm=NULL;
	unlock();
	void * ret = e->elem;
	delete e;
	return ret;
}

int LinkedQueue::GetSize()
{
	return size;
	/*
	lock();
	int s = size;
	unlock();
	return s;*/
}