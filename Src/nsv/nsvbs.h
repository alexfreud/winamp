/*
** nsvbs.h - NSV basic inline bitstream classes
**
** Copyright (C) 2001-2002 Nullsoft, Inc.
** Confidential Subject to NDA
**
** Note: these bitstream classes encode/decode everything in LSB.
** bits are stored from the lowest bit to the highest bit.
**  putbits(4,0xE) will result in getbits(1)=0, getbits(1)=1, 
**                                getbits(1)=1, getbits(1)=1
**    or of course, getbits(4) == 0xE :)
*/

#ifndef _NSVBS_H_
#define _NSVBS_H_

#include <stdlib.h>
#include <memory.h>
#include <bfc/platform/types.h>
#include "../nu/GrowBuf.h"

class nsv_OutBS
{
public:
	nsv_OutBS() { m_used = 0; m_curb=0; }
	~nsv_OutBS() { }

	void putbits(int nbits, unsigned int value)
	{
		while (nbits-- > 0)
		{
			m_curb|=(value&1)<<(m_used&7);
			if (!((++m_used)&7))
			{
				m_bits.add(&m_curb,1);
				m_curb=0;
			}
			value>>=1;
		}
	}

    // lets you put in any amount of data, but does not preserve endianness.
	void putdata(int nbits, void *data) 
	{
		unsigned char *c=(unsigned char *)data;
		if (!(m_used&7) && nbits >= 8)
		{
			m_bits.add(c,nbits/8);
			c+=nbits/8;
			m_used+=nbits&~7;
			nbits&=7;
		}
		while (nbits > 0)
		{
			int tb=nbits;
			if (tb > 8) tb=8;
			putbits(tb,*c++);
			nbits-=tb;
		}
	}

	int getlen() { return m_used; } // in bits

	void *get(int *len) // len is in bytes, forces to byte aligned.
	{
		if (m_used&7)
		{
			m_bits.add(&m_curb,1);
			m_used=(m_used+7)&~7;
			m_curb=0;
		}
		*len=m_used/8;
		return m_bits.get();
	}

	void clear()
	{
		m_used=0;
		m_curb=0;
		m_bits.resize(0);
	}

private:
	GrowBuf m_bits;
	int m_used; // bits
	unsigned char m_curb;
};

class nsv_InBS {
public:
	nsv_InBS() { m_bitpos=0; m_eof=0; }
	~nsv_InBS() { }

	void clear() 
	{
	    m_eof=0;
		m_bitpos=0;
		m_bits.resize(0);
	}

	void add(void *data, int len)
	{
		m_bits.add(data,len);
		m_eof=0;
	}

	void addbyte(unsigned char byte)
	{
		add(&byte,1);
	}

	void addint(unsigned int dword)
	{
		addbyte(dword&0xff);
		addbyte((dword>>8)&0xff);
		addbyte((dword>>16)&0xff);
		addbyte((dword>>24)&0xff);
	}
  
	void compact()
	{
		size_t bytepos=m_bitpos/8;
		if (bytepos)
		{
			unsigned char *t=(unsigned char *)m_bits.get();
			size_t l=m_bits.getlen()-bytepos;
			memcpy(t,t+bytepos,l);
			m_bits.resize(l);
			m_bitpos&=7;
		}
		m_eof=0;
	}

	void seek(ptrdiff_t nbits) 
	{
		if (nbits < 0 && ((size_t)(-nbits)) > m_bitpos)
			m_bitpos=0;
		else
			m_bitpos+=nbits;
		m_eof=m_bits.getlen()*8 < m_bitpos; 
	}

	void rewind() { m_bitpos=0; m_eof=0; }
	int eof() { return m_eof; }
	size_t avail() { if (m_eof) return 0; return m_bits.getlen()*8 - m_bitpos; }

	unsigned int getbits(size_t nbits)
	{
		unsigned int ret=0;
		if (!nbits) return ret;
		unsigned char *t=(unsigned char *)m_bits.get();

		if (!t || m_bits.getlen()*8 < m_bitpos+nbits) m_eof=1;
		else
		{
			t+=m_bitpos/8;
			for (size_t sh = 0; sh != nbits; sh ++)
			{
				ret|=((*t>>(m_bitpos&7))&1) << sh;
				if (!((++m_bitpos)&7)) t++;
			}
		}
		return ret;
	}

	int getdata(size_t nbits, void *data)
	{
		unsigned char *t=(unsigned char *)data;
		if (m_bits.getlen()*8 < m_bitpos+nbits) return 1;
		if (!(m_bitpos&7) && nbits >= 8)
		{
			char *bitptr=(char*)m_bits.get();
			bitptr+=(m_bitpos/8);
			memcpy(t,bitptr,nbits/8);
			m_bitpos+=nbits&~7;

			t+=nbits/8;
			nbits&=7;
		}
		while (nbits > 0)
		{
			size_t nb=nbits;
			if (nb > 8) nb=8;
			*t++=getbits(nb);
			nbits-=nb;
		}
		return 0;
	}

	void *getcurbyteptr()
	{
		char *t=(char*)m_bits.get();
		return (void *)(t+(m_bitpos/8));
	}

private:
	GrowBuf m_bits;
	size_t m_bitpos;
	int m_eof;
};

#endif//_NSVBS_H_