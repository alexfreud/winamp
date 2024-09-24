#ifndef _PFC_GROW_BUF_H_
#define _PFC_GROW_BUF_H_

class grow_buf
{
private:
	void * ptr;
	int size,used;
	void makespace(int);
public:
	grow_buf(int init_size = 0)
	{
		if (init_size<8) init_size=8;
		size = 0;
		used = 0;
		ptr = 0;
		makespace(init_size);
	}
	
	~grow_buf() {reset();}

	inline const void * get_ptr_c() const {return ptr;}
	inline void * get_ptr() {return ptr;}
	inline int get_size() const {return used;}
	inline void truncate(int z) {if (z<used) used=z;}

	void * finish();
	void reset();

	bool write(const void * data, size_t bytes);
	void write_ptr(const void * data, int bytes,int offset);

	inline void write_byte(BYTE b) {write(&b,1);}
	inline void write_word(WORD w) {write(&w,2);}
	inline void write_dword(DWORD dw) {write(&dw,4);}
	inline void write_byte_ptr(BYTE b,int ptr) {write_ptr(&b,1,ptr);}
	inline void write_word_ptr(WORD w,int ptr) {write_ptr(&w,2,ptr);}
	inline void write_dword_ptr(DWORD dw,int ptr) {write_ptr(&dw,4,ptr);}
};


#endif