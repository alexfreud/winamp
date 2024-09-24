/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: sequencedetector.h
 *   project : ---
 *   author  : Martin Sieler
 *   date    : 1998-02-14
 *   contents/description: HEADER - sequence detector
 *
 *
\***************************************************************************/

/*
 * $Date: 2010/11/17 20:46:05 $
 * $Id: sequencedetector.h,v 1.1 2010/11/17 20:46:05 audiodsp Exp $
 */

#ifndef __SEQUENCEDETECTOR_H__
#define __SEQUENCEDETECTOR_H__

/* ------------------------ includes --------------------------------------*/

/*-------------------------- defines --------------------------------------*/

/*-------------------------------------------------------------------------*/

class CSequenceDetector
{
public:
  CSequenceDetector(int nLimit);
  ~CSequenceDetector();

  void Reset();
  CSequenceDetector& operator+= (int nValue);

  int  GetLength()          const;
  int  GetValue(int nIndex) const;
  int  GetSum()             const;

protected:

private:
  int   m_Limit;
  int   m_Count;
  bool *m_pDisabled;
  int  *m_pArray;
};

/*-------------------------------------------------------------------------*/
#endif
