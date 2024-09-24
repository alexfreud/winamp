/* $Header: /cvs/root/winamp/vlb/dolby_imdct.h,v 1.1 2009/04/28 20:21:09 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *   filename: dolby_mdct.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: Inverse MDCT include file
 *
\***************************************************************************/

#ifndef _DOLBY_IMDCT_H
#define _DOLBY_IMDCT_H
#include<math.h>
#include<string.h>
#include"fft.h"

/*#define INTERPOLATE_WINDOW  */

class IMDCTObject{

	public:
		IMDCTObject(int _iLongSize,
					int _iOverlap);
		IMDCTObject();
		IMDCTObject(IMDCTObject&oCopy);
		IMDCTObject& operator=(IMDCTObject&oCopy);
		~IMDCTObject();

		int Transform(	float*pfData,
						int iWindowType,
						int iWindowShape);

	private:
		enum {
			LONG_BLOCK,
			SHORT_BLOCK
		};

		enum {
			LONG_WINDOW = 0,
			START_WINDOW = 1,
			EIGHT_SHORT_SEQUENCE = 2,
			STOP_WINDOW = 3
		};

		enum {
			SINE_WINDOW = 0,
			KBD_WINDOW = 1
		};

		void imdct(float* pfData, int blocktype);

	private:
		int iLongSize;
		int iShortSize;
		int iHLongSize;
		int iHShortSize;
		int iOverlap;
		int iOldShape;
		
		//Windows:
		float*pfLongWindowSin;
		float*pfS2LWindowSin;
		float*pfShortWindowSin;

		float*pfLongWindowKBD;
		float*pfS2LWindowKBD;
		float*pfShortWindowKBD;
		

		float*pfTempBuffer;
		float*pfEightWindowsBuffer;

		float*pfSavedBuffer;

		//MDCT Data:
		int iLongFFTSize;
		int iShortFFTSize;
		float*pfReal;
		float*pfImag;
		float*pfRealLongTwiddle;
		float*pfImagLongTwiddle;

		float*pfRealShort;
		float*pfImagShort;
		float*pfRealShortTwiddle;
		float*pfImagShortTwiddle;

};
#endif