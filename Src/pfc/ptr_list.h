#include "mem_block.h"

class ptr_list
{
private:
	mem_block_t<void *> data;
	int count;
	
protected:
	void * get_raw_ptr() {return data.get_ptr();}
	
public:
	ptr_list()                                       {count=0;}
	int get_count() const                            {return count;}
	void * get_item(int n) const
	{
		if (n>=0 && n<count) return data[n];
		else return 0;
	}
	int add_item(void * ptr)//returns index
	{
		count++;
		data.check_size(count);
		data[count-1] = ptr;
		return count-1;
	}
	int find_item(void * ptr)//returns index, -1 if not found
	{
		int n;
		for(n=0;n<count;n++)
		{
			if (data[n] == ptr) return n;
		}
		return -1;
	}

	bool have_item(void * ptr) {return find_item(ptr)>=0;}

	void remove_item(void * ptr)
	{
		int idx = find_item(ptr);
		if (idx>=0) remove_by_idx(idx);
	}
	void * remove_by_idx(int idx)
	{
		void * ptr = 0;
		if (idx>=0 && idx<count)
		{
			ptr = data[idx];
			int n;
			count--;
			for(n=idx;n<count;n++)
			{
				data[n] = data[n+1];
			}
		}
		return ptr;
	}

	void remove_all()
	{
		count=0;
	}

	int insert_item(void * ptr,int idx)	//returns index
	{
		if (idx>count || idx<0) idx = count;
		
		count++;
		data.check_size(count);
		int n;
		for(n=count-1;n>idx;n--)
		{
			data[n]=data[n-1];
		}
		data[idx] = ptr;

		return idx;
	}
	
	void * operator[](int idx) const {return get_item(idx);}
};

template<class T>
class ptr_list_t : protected ptr_list
{
public:
	int get_count() const            {return ptr_list::get_count();}

	T * get_item(int n) const        {return static_cast<T*>(ptr_list::get_item(n));}
	int add_item(T * ptr)            {return ptr_list::add_item(static_cast<void*>(ptr));}
	int find_item(T * ptr)           {return ptr_list::find_item(static_cast<void*>(ptr));}
	bool have_item(T * ptr)          {return ptr_list::have_item(static_cast<void*>(ptr));}
	void remove_item(T * ptr)        {ptr_list::remove_item(static_cast<void*>(ptr));}
	T * remove_by_idx(int idx)       {return static_cast<T*>(ptr_list::remove_by_idx(idx));}
	void remove_all()                {ptr_list::remove_all();}
	void * operator[](int idx) const {return get_item(idx);}
	int insert_item(int idx,T* ptr)  {return ptr_list::insert_item(idx,static_cast<void*>(ptr));}

	void delete_item(T * ptr)
	{
		remove_item(ptr);
		delete ptr;
	}

	void delete_by_idx(int idx)
	{
		T * ptr = remove_by_idx(idx);
		if (ptr) delete ptr;
	}

	void delete_all()
	{
		int n,max=get_count();
		for(n=0;n<max;n++)
		{
			T * ptr = get_item(n);
			if (ptr) delete ptr;
		}
		remove_all();
	}

	void sort(int (__cdecl *compare )(const T ** elem1, const T** elem2 ) )
	{
		qsort(get_raw_ptr(),get_count(),sizeof(void*),(int (__cdecl *)(const void *, const void *) )compare);
	}
};
