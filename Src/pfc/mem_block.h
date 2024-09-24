#ifndef _PFC_MEM_BLOCK_H_
#define _PFC_MEM_BLOCK_H_

class mem_block
{
private:
	void * data;
	int size;
public:
	mem_block() {data=0;size=0;}
	~mem_block() {if (data) free(data);}

	int get_size() const {return size;}

	void * get_ptr() const {return data;}

	void * set_size(int new_size)
	{
		if (data==0) data = malloc(size = new_size);
		else if (size!=new_size)
		{
			void * new_data = realloc(data,new_size);
			if (!new_data) return 0;
			data = new_data;
			size = new_size;
		}
		return data;
	}

	void * check_size(int new_size)
	{
		if (size<new_size) return set_size(new_size);
		else return data;
	}

	operator void * () const {return data;}

};

template<class T>
class mem_block_t : private mem_block
{
public:
	int get_size() const {return mem_block::get_size()/sizeof(T);}

	T* get_ptr() const {return static_cast<T*>(mem_block::get_ptr());}

	T* set_size(int new_size) {return static_cast<T*>(mem_block::set_size(new_size*sizeof(T)));}

	T* check_size(int new_size) {return static_cast<T*>(mem_block::check_size(new_size*sizeof(T)));}

	operator T * () const {return get_ptr();}
};

#endif