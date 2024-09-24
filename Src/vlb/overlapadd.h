/* $Header: /cvs/root/winamp/vlb/overlapadd.h,v 1.2 2011/06/13 02:06:03 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: overlapadd.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: time domain overlap add buffer object
 *
\***************************************************************************/

#ifndef __OVERLAPADD_H__
#define __OVERLAPADD_H__

/** Buffer For Time Domain Overlap/Add Data.

    This simple helper class provides a self-initializing buffer for one frame
    worth of time domain signal data, associated window shape information and
    appropriate access methods.
*/

class COverlapAddBuffer
{

protected :

  enum
  {
    BufferSize = 1024 
  } ;

public :

  COverlapAddBuffer ()
  {
    m_WindowShape = 0 ;

    for (int i = 0 ; i < COverlapAddBuffer::BufferSize ; i++)
    {
      m_Buffer [i] = 0.0F ;
    }

  }

  ~COverlapAddBuffer () {} ;

  // buffer access

  float &operator [] (int index)
  {
    return m_Buffer [index] ;
  }

  // window shape access

  int GetWindowShape (void) const
  {
    return m_WindowShape ;
  }

  void SetWindowShape (int shape)
  {
    m_WindowShape = shape ;
  }

#if defined (WIN32) && defined (_M_IX86)

  float *AccessBuffer (void)
  {
    return m_Buffer ;
  }

#endif

protected :

  int m_WindowShape ;

  float m_Buffer [COverlapAddBuffer::BufferSize] ;

} ;

#endif
