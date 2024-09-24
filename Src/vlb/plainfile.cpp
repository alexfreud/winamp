/* $Header: /cvs/root/winamp/vlb/plainfile.cpp,v 1.1 2009/04/28 20:21:10 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: plainfile.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: tiny file reader input class
 *
\***************************************************************************/

#include "plainfile.h"
#include <memory.h>
#include <stddef.h> // for intptr_t
const unsigned long CPlainFile::m_ValidMask [32] =
{
  0x00000000, 0x00000001, 0x00000003, 0x00000007,
  0x0000000F, 0x0000001F, 0x0000003F, 0x0000007F,
  0x000000FF, 0x000001FF, 0x000003FF, 0x000007FF,
  0x00000FFF, 0x00001FFF, 0x00003FFF, 0x00007FFF,
  0x0000FFFF, 0x0001FFFF, 0x0003FFFF, 0x0007FFFF,
  0x000FFFFF, 0x001FFFFF, 0x003FFFFF, 0x007FFFFF,
  0x00FFFFFF, 0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF,
  0x0FFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF    
} ;

// public

CPlainFile::CPlainFile (DataIOControl	*aacData)
  : CDolbyBitStream ()
{
  m_InputData = aacData ;
  if (m_InputData->DICGetLastError()) throw EDoesNotExist () ;

  m_ValidBits = 0 ;
  m_Data = 0 ;

  // detect adif header
  m_sendadifbits=0;

#if 0 // FUCK ADIF HEADERS, THEY BLOW DONKEYBALLZ
  memset(m_tmpadifbits,0,sizeof(m_tmpadifbits));

  m_InputData->IO(m_tmpadifbits, 1, 4) ;
  //m_InputData->Seek(0,SEEK_SET) ;
 
  char *adif=m_tmpadifbits;
  
  if ((adif [0] == 'A') &&
      (adif [1] == 'D') &&
      (adif [2] == 'I') &&
      (adif [3] == 'F')
     )
  {
    m_AdifHeaderPresent = true ;
    m_sendadifbits=0;
  }
  else
	  if ((adif [0] == 'R') &&
		  (adif [1] == 'I') &&
		  (adif [2] == 'F') &&
		  (adif [3] == 'F')
		 )
		{
			m_RiffHeaderPresent = true ;
      m_sendadifbits=0;
		}
#endif

}

CPlainFile::~CPlainFile ()
{
  //fclose (m_InputFile) ;
}

long CPlainFile::Get (int n)
{
  if (m_ValidBits <= 16)
  {
    m_Data = (m_Data << 16) | GetShort () ;
    m_ValidBits += 16 ;
  }

  m_ValidBits -= n ;
  return ((m_Data >> m_ValidBits) & m_ValidMask [n]) ;
}

void CPlainFile::ByteAlign (void)
{
  if (m_ValidBits <= 16)
  {
    m_Data = (m_Data << 16) | GetShort () ;
    m_ValidBits += 16 ;
  }
  
  while (m_ValidBits & 0x07)
  {
    m_ValidBits-- ;
  }
}

void CPlainFile::PushBack (int n)
{
  m_ValidBits += n ;
}

int CPlainFile::Read (void *pData, int cBytes)
{
  if(m_sendadifbits)
  {
    cBytes-=4; //CT> I assume that it'll always be >4 at this point in the exec.
    memcpy(pData,m_tmpadifbits,4);
    intptr_t a=(intptr_t)pData+4;
    pData=(void*)a;
    m_sendadifbits=0;
  }
  return m_InputData->IO(pData,1,cBytes) ;
}

// protected

long CPlainFile::GetShort (void)
{
  int c1;
  int c2;
  m_InputData->IO(&c1,1,1);
  m_InputData->IO(&c2,1,1);	
  if (m_InputData->EndOf())
    throw EEndOfFile () ;
  c1&=0xff;
  c2&=0xff;
  return (c1 << 8) | c2 ;
}

#define SYNCWORD	 0x0fff
#define SYNCMASK	 0x0fff
#define SYNCLENGTH	 12
#define REMLENGTH	 16
#define MAX_SEARCH   100000

int CPlainFile::LockAndLoad()
{
	int iCount;
	unsigned int dwSync;
	dwSync=0;
	iCount=0;
	while(iCount<MAX_SEARCH && dwSync!=SYNCWORD){
		int iTemp;
		dwSync<<=8;
		m_InputData->IO(&iTemp,1,1);
		if (m_InputData->EndOf())
			throw EEndOfFile () ;
		dwSync+=iTemp;
		dwSync&=SYNCMASK;
		iCount++;
	}
	m_Data=dwSync;
	m_ValidBits=REMLENGTH;
	if(iCount<MAX_SEARCH) return 1;
	else return 0;
}
