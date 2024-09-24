#ifndef NULLSOFT_OUT_DS_SOUNDBLOCK_H
#define NULLSOFT_OUT_DS_SOUNDBLOCK_H

class SoundBlock
{
public:
	SoundBlock *next, *prev;
	SoundBlock();
	~SoundBlock();
	void SetData(void *new_data, size_t new_size);
	void Advance(size_t d);
	const void *GetData();
	size_t GetDataSize();
	size_t Dump(void * out, size_t out_size);
	void Clear();
private:
	void *data;
	size_t size, used, start;
};

#endif
