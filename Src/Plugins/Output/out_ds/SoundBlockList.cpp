#include "SoundBlockList.h"
#include <memory.h>

void SoundBlockList::AddBlock(void * data, size_t size)
{
	SoundBlock * b = remove(b_free);
	if (!b) b=new SoundBlock;

	b->SetData(data,size);

	mount2(b_first,b,b_last);

	data_buffered+=size;
#ifdef USE_LOG
	char boo[256] = {0};
	wsprintf(boo,"AddBlock: %u, +%u",data_buffered,size);
	log_write(boo);
#endif
}

size_t SoundBlockList::DumpBlocks(void * out1, size_t size1)
{
	char * p1=(char*)out1;
	size_t rv=0;
	while(b_last && size1)
	{
		if (size1)
		{
			size_t d=b_last->Dump(p1,size1);
			p1+=d;
			size1-=d;
			rv+=d;
		}
		if (b_last->GetDataSize()==0)
		{
			mount(b_free,removelast(b_first,b_last));
		}
	}
	if (size1) memset(p1,silence_filler,size1);

	data_buffered-=rv;
	return rv;
}

void SoundBlockList::Reset()
{
	if (b_first)
	{
		connect(b_last,b_free);
		b_free=b_first;
	}

	b_first=0;
	b_last=0;
	data_buffered=0;
}

SoundBlockList::~SoundBlockList()
{
	SoundBlock * p;
	while(b_first)
	{
		p=b_first;
		b_first=b_first->next;
		delete p;
	}
	while(b_free)
	{
		p=b_free;
		b_free=b_free->next;
		delete p;
	}
}

size_t SoundBlockList::DataSize()
{
	return data_buffered;
}

SoundBlockList::SoundBlockList(int sil)
{
	b_first = 0;b_last = 0;b_free = 0;data_buffered = 0;silence_filler = sil;
}

void SoundBlockList::advance(SoundBlock * &b)
{
	b = b->next;
}

void SoundBlockList::goback(SoundBlock * &b)
{
	b = b->prev;
}

void SoundBlockList::mount(SoundBlock * &first, SoundBlock * add)
{
	connect(0, add);
	connect(add, first);
	first = add;
}
void SoundBlockList::mount2(SoundBlock * &first, SoundBlock * add, SoundBlock * &last)
{
	mount(first, add);
	if (!last) last = add;
}
SoundBlock *SoundBlockList::remove(SoundBlock * &list)
{
	SoundBlock * b = list;
	if (b)
	{
		advance(list);
		connect(0, list);
		connect(b, 0);
	}
	return b;
}

SoundBlock *SoundBlockList::removelast(SoundBlock * &first, SoundBlock * &last)
{
	SoundBlock * b = last;
	if (b)
	{
		goback(last);
		connect(last, 0);
		connect(0, b);
		if (!last) first = 0;
	}
	return b;
}
