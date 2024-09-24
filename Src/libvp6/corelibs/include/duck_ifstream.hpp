#if !defined(_duck_ifstream_h)
#define _duck_ifstream_h


#include "duck_io.h"




#include <stdio.h>
#include <sstream>
#include <ios>
#include <assert.h>



#include "iduck_ifstream.hpp"




class duck_ifstream_ifstream : duck_ifstream
{
	public:
	
	bool operator!() const
	{
		return (m_fd == 0);
	}
	
	
	bool is_open()
	{
		return (m_fd != 0);
	}
	
	
	int length()
	{
		if (m_length < 0)
		{
			FILE* fp = (FILE* ) m_fd;
			long off = ftell(fp);
			
			fseek(fp, 0,SEEK_END);
			
			long off2 = ftell(fp);
			
			fseek(fp, off, SEEK_SET);
			
			m_length = (int ) off2;
			
			return (int) off2;
		}
		else
		{
			return m_length;
		}
	}
	
	
	void time_out(unsigned long milli)
	{
		;
	}
	
	
	bool eof()
	{
	   return feof(m_fd);
	}
	
	
	operator bool () const
	{
		return (m_fd != 0);
	}
	
	
	void open(const char* filename, std::ios_base::openmode mode)
	{
		m_length = -1;
		m_fd = fopen(filename, "rb");
	}
	
	
	void open(void *src, void* ignore)
	{
		assert(0);
	}
	
	void close()
	{
		if (m_fd)
		{
			fclose(m_fd);
		}
	}
	
	void read(void *buffer, size_t len)
	{
		fread((unsigned  char* ) buffer, sizeof(char), len, m_fd);
	}
	
	void get(char& c)
	{
		fread((unsigned  char* ) &c, sizeof(char), 1, m_fd);
	}
	
	
	void seekg(long position)
	{
		fseek(m_fd, position, SEEK_SET);
	}
	
	
	
	void seekg(long offset, int origin)
	{
	
		switch (origin)
		{
			case std::ios_base::cur :
				fseek(m_fd, offset , SEEK_CUR);
		    	break;
		    case std::ios_base::end :
		        fseek(m_fd, offset, SEEK_END);
		        break;
		    case std::ios_base::beg :
		        fseek(m_fd, offset, SEEK_SET);
		        break;
		        
		    default :	
		    	assert(0);
		    	break;
		}
	}
	
	
	void ignore(long offset)
	{
		fseek(m_fd, offset, SEEK_CUR);
	}
	
	
	long tellg()
	{
	    const std::streamoff off = ftell(m_fd);
		return std::streampos(off);
	}


	private:
		FILE* m_fd;
		
		int m_length;
		
};



extern "C" { 
void MessageBox(char* title, char* msg); 
}



#include "duck_io.h"




class duck_ifstream_http : duck_ifstream
{
	public:
	
	bool operator!() const
	{
		return (m_fd <= 0);
	}
	
	
	bool is_open()
	{
		return (m_fd > 0);
	}
	
	
	~duck_ifstream_http()
	{                                            
	   duck_exit_http(m_fd);
	}
	
	
	operator bool () const
	{
		return (m_fd >= 0);
	}
	
	
	int length()
	{
		return duck_length((int ) m_fd);
	}
	

	
	
	void time_out(unsigned long milli)
	{
		duck_http_timeout(m_fd, milli);
	}
	
	
	
	bool eof()
	{
	    return duck_eof(m_fd);
	}
	
	
	
	
	
	void open(const char* url, std::ios_base::openmode mode)
	{
          
			m_fd = (int) duck_init_http(url);


			if (duck_open((char *) m_fd, 0) < 0)
			{
			    if (m_fd)
			    {
			        duck_close((int ) m_fd);
			        m_fd = -1;
			    }  
			    
			}
		
	}

	
	void open(void *src, void* ignore)
	{
		assert(0);
	}
	
	void close()
	{
		if (m_fd >= 0)
		{
			duck_close(m_fd);
		}
	}
	
	void read(void *buffer, size_t len)
	{
		size_t x;
		
		x = duck_read(m_fd, (unsigned  char* ) buffer, (long ) len);
		
		if (x != len)
		{
			MessageBox("Error", "NSV Read Failed");
		}
	}
	
	void get(char& c)
	{
			duck_read(m_fd, (unsigned char *) &c, 1);
	}
	
	
	void seekg(long position)
	{
		long o = position - duck_tell(m_fd);
		
		if (o >= 0)
		{
			duck_seek(m_fd, o, SEEK_CUR);
		}
		else
		{
			duck_close(m_fd);
			duck_open((char *) m_fd, 0);
			
			duck_seek(m_fd, position, SEEK_CUR);
		}
	}
	
	
	
	void seekg(long offset, int origin)
	{
	
		switch (origin)
		{
			case std::ios_base::cur :
				duck_seek(m_fd, offset, SEEK_CUR);
		    	break;
		    default :	
		        /* don't do it ! */
		    	/* assert(0); */
		    	break;
		}
	}
	
	
	void ignore(long offset)
	{
		duck_seek(m_fd, offset, SEEK_CUR);
	}
	
	
	long tellg()
	{
	    long off = duck_tell(m_fd);
		return off;
	}


	private:
		int m_fd;
		
		
		#if 0	
	    /* disable copying ! */
	    duck_ifstream_http(const duck_ifstream_http& );
	
	
	    /* disable copying ! */
	    operator= (const duck_ifstream_http& );
        #endif	
		
};



#endif
