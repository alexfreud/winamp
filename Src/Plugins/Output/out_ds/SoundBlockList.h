#ifndef NULLSOFT_OUT_DS_SOUNDBLOCKLIST_H
#define NULLSOFT_OUT_DS_SOUNDBLOCKLIST_H

#include "SoundBlock.h"

class SoundBlockList
{
public:

	SoundBlockList(int sil);
	~SoundBlockList();
	void AddBlock(void * data, size_t size);
	size_t DumpBlocks(void * out1, size_t size1);
	void Reset();
	size_t DataSize();
	
private:
	void connect(SoundBlock * b1, SoundBlock * b2) {if (b1) b1->next = b2;if (b2) b2->prev = b1;}
	void advance(SoundBlock * &b);
	void goback(SoundBlock * &b);
	void mount(SoundBlock * &first, SoundBlock * add);
	void mount2(SoundBlock * &first, SoundBlock * add, SoundBlock * &last);
	SoundBlock * remove(SoundBlock * &list);
	SoundBlock * removelast(SoundBlock * &first, SoundBlock * &last);

	size_t data_buffered;
	SoundBlock *b_first, *b_last, *b_free;
	int silence_filler;
};


#endif
