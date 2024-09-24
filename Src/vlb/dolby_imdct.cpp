/* $Header: /cvs/root/winamp/vlb/dolby_imdct.cpp,v 1.1 2009/04/28 20:21:09 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *   filename: dolby_imdct.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: IMDCT transform routines
 *
\***************************************************************************/

#include"dolby_imdct.h"
#ifdef INTERPOLATE_WINDOW
#include "window_tab.h"
#endif

/* #defines related to Izero and CalculateKBDWindowExact */
#define IzeroEPSILON 1E-21			/* Max error acceptable in Izero */
#define M_PI	3.14159265358979323846264338327950288f

/*------------------------ Window Functions ---------------------------*/

void Sine_Window(float*pfWindow,int iSize)
{
#ifdef INTERPOLATE_WINDOW
	int count, j;

	switch (iSize)
	{
		case 256:
			for(j=0; j < iSize/2 ;j++)
			{
				pfWindow[j]=(float)(sin_short[j]);
			}
			break;			

		case 512:
			count = 0;
			for (j = 0; j < iSize/4-1; j++)
			{
				pfWindow[count++] = (float) sin_short[j];
				pfWindow[count++] = (float) ((sin_short[j] + sin_short[j + 1]) * 0.5);
			}
			pfWindow[count++] = (float) sin_short[j];
			pfWindow[count] = (float) 1.0;
			break;

		case 2048:
			for(j=0; j < iSize/2 ;j++)
			{
				pfWindow[j]=(float)(sin_long[j]);
			}
			break;			

		case 4096:
			count = 0;
			for (j = 0; j < iSize/4-1; j++)
			{
				pfWindow[count++] = (float) sin_long[j];
				pfWindow[count++] = (float) ((sin_long[j] + sin_long[j + 1]) * 0.5);
			}
			pfWindow[count++] = (float) sin_long[j];
			pfWindow[count] = (float) 1.0;
			break;
	}
#else
	int n;
	for(n=0;n<iSize/2;n++){
		pfWindow[n]=(float)(sin(pi*((float)n+0.5)/(float)iSize));
	}
#endif
}


static float Izero(float x)
{
	float sum,
	       u,
	       halfx,
	       temp;
	int n;

	sum = u = 1.0f;
	n = 1;

	halfx = x / 2.0f;
	do
	{
		temp = halfx / (float) n;
		n += 1;
		temp *= temp;
		u *= temp;
		sum += u;
	} while (u >= IzeroEPSILON * sum);
	return (sum);
}

/*****************************************************************************

    functionname: CalculateKBDWindowExact
    description:  calculates the window coefficients for the Kaiser-Bessel
                  derived window
    returns:
    input:        window length, alpha
    output:       window coefficients

*****************************************************************************/
void CalculateKBDWindowExact(float * win, float alpha, int length)
{
	int i;
	float IBeta;
	float tmp;
	float sum = 0.0;

	alpha *= M_PI;
	IBeta = 1.0f / Izero(alpha);

	/* calculate lower half of Kaiser Bessel window */
	for (i = 0; i < (length >> 1); i++)
	{
		tmp = 4.0f * (float) i / (float) length - 1.0f;
		win[i] = Izero(alpha * ((float) sqrt(1.0f - tmp * tmp))) * IBeta;
		sum += win[i];
	}

	sum = 1.0f / sum;
	tmp = 0.0;

	/* calculate lower half of window */
	for (i = 0; i < (length >> 1); i++)
	{
		tmp += win[i];
		win[i] = (float) sqrt(tmp * sum);
	}
}





void KBD_Window(float*pfWindow,int iSize)
{
#ifdef INTERPOLATE_WINDOW
	int count, j;

	switch (iSize)
	{
		case 256:
			for(j=0; j < iSize/2 ;j++)
			{
				pfWindow[j]=(float)(KBD_short[j]);
			}
			break;			

		case 512:
			count = 0;
			for (j = 0; j < iSize/4-1; j++)
			{
				pfWindow[count++] = (float) KBD_short[j];
				pfWindow[count++] = (float) ((KBD_short[j] + KBD_short[j + 1]) * 0.5);
			}
			pfWindow[count++] = (float) KBD_short[j];
			pfWindow[count] = (float) 1.0;
			break;

		case 2048:
			for(j=0; j < iSize/2 ;j++)
			{
				pfWindow[j]=(float)(KBD_long[j]);
			}
			break;			

		case 4096:
			count = 0;
			for (j = 0; j < iSize/4-1; j++)
			{
				pfWindow[count++] = (float) KBD_long[j];
				pfWindow[count++] = (float) ((KBD_long[j] + KBD_long[j + 1]) * 0.5);
			}
			pfWindow[count++] = (float) KBD_long[j];
			pfWindow[count] = (float) 1.0;

			break;
	}
#else

	/* The stock AAC MP4 encoder calculates KBD windows with different alphas
	   depending on the blocklength. We must match those windows here. */

	switch(iSize)
	{
		case 256:
		case 512: 
			CalculateKBDWindowExact(pfWindow, 6.0, iSize);
			break;
		
		case 2048:
		case 4096:
			CalculateKBDWindowExact(pfWindow, 4.0, iSize);
			break;
		default:
 			CalculateKBDWindowExact(pfWindow, 4.0, iSize);

	}

#endif
}

/*----------------------- DOLBY IMDCT ---------------------------------*/

IMDCTObject::IMDCTObject(int _iLongSize,
						 int _iOverlap)
	:iLongSize(_iLongSize),
	iHLongSize(_iLongSize/2),
	iOverlap(_iOverlap),
	iOldShape(0)
{
	if(iLongSize==0){
		return;
	}
	if(iOverlap==0){
		return;
	}
	int n,k;
	
	//Obtain memory for Short Windows:
	//These arrays are larger than they need to be,
	//but are made this large for convenience
	iShortSize = iLongSize / 8;
	iHShortSize = iHLongSize / 8;
	pfShortWindowSin = new float[iHLongSize];
	pfShortWindowKBD = new float[iHLongSize];
	
	//Obtain memory for Longwindows:
	pfLongWindowSin=new float[iHLongSize];
	pfS2LWindowSin=new float[iHLongSize];

	pfLongWindowKBD=new float[iHLongSize];
	pfS2LWindowKBD=new float[iHLongSize];
	
	//Obtain memory for temporary buffer:
	pfTempBuffer=new float[iLongSize];
	pfEightWindowsBuffer=new float[iLongSize];
	pfSavedBuffer=new float[iHLongSize];
	for(n=0;n<iHLongSize;n++) pfSavedBuffer[n]=0;
	
	//Create Windows:
	KBD_Window(pfLongWindowKBD,iLongSize);
	Sine_Window(pfLongWindowSin,iLongSize);
	
	//Create S2L Window:
	KBD_Window(pfShortWindowKBD,iOverlap*2); // temporary
	for(k=0,n=iHLongSize/2-iOverlap/2;k<iOverlap;k++,n++){
		pfS2LWindowKBD[n]=pfShortWindowKBD[k];
	}
	for(n=0;n<iHLongSize/2-iOverlap/2;n++){
		pfS2LWindowKBD[n]=0.0f;
	}
	for(n=iHLongSize/2+iOverlap/2;n<iHLongSize;n++){
		pfS2LWindowKBD[n]=1.0f;
	}

	//Create correct length short window
	KBD_Window(pfShortWindowKBD,iShortSize);

	//Create S2L Window:
	Sine_Window(pfShortWindowSin,iOverlap*2); // temporary
	for(k=0,n=iHLongSize/2-iOverlap/2;k<iOverlap;k++,n++){
		pfS2LWindowSin[n]=pfShortWindowSin[k];
	}
	for(n=0;n<iHLongSize/2-iOverlap/2;n++){
		pfS2LWindowSin[n]=0.0f;
	}
	for(n=iHLongSize/2+iOverlap/2;n<iHLongSize;n++){
		pfS2LWindowSin[n]=1.0f;
	}

	//Create correct length short window
	Sine_Window(pfShortWindowSin,iShortSize);

	// Compute Long Twiddles
	iLongFFTSize=iLongSize/4;
	pfReal=new float[iLongFFTSize];
	pfImag=new float[iLongFFTSize];
	pfRealLongTwiddle=new float[iLongFFTSize];
	pfImagLongTwiddle=new float[iLongFFTSize];
	pfRealLongTwiddle[0]=(float)(cos(0.125*2.0*pi/(float)iLongSize));
	pfImagLongTwiddle[0]=(float)(sin(0.125*2.0*pi/(float)iLongSize));
	for(n=1;n<iLongFFTSize;n++){
		pfRealLongTwiddle[n]=(float)(cos(2.0*pi/(float)iLongSize))*pfRealLongTwiddle[n-1]-
									(float)(sin(2.0*pi/(float)iLongSize))*pfImagLongTwiddle[n-1];
		pfImagLongTwiddle[n]=(float)(cos(2.0*pi/(float)iLongSize))*pfImagLongTwiddle[n-1]+
									(float)(sin(2.0*pi/(float)iLongSize))*pfRealLongTwiddle[n-1];
	}

	// Compute Short Twiddles
	// these twiddles may be redundant - they may be able to be computed as a function
	// of the long twiddles
	iShortFFTSize=iShortSize/4;
	pfRealShort=new float[iShortFFTSize];
	pfImagShort=new float[iShortFFTSize];
	pfRealShortTwiddle=new float[iShortFFTSize];
	pfImagShortTwiddle=new float[iShortFFTSize];
	pfRealShortTwiddle[0]=(float)(cos(0.125*2.0*pi/(float)iShortSize));
	pfImagShortTwiddle[0]=(float)(sin(0.125*2.0*pi/(float)iShortSize));
	for(n=1;n<iShortFFTSize;n++){
		pfRealShortTwiddle[n]=(float)(cos(2.0*pi/(float)iShortSize))*pfRealShortTwiddle[n-1]-
									(float)(sin(2.0*pi/(float)iShortSize))*pfImagShortTwiddle[n-1];
		pfImagShortTwiddle[n]=(float)(cos(2.0*pi/(float)iShortSize))*pfImagShortTwiddle[n-1]+
									(float)(sin(2.0*pi/(float)iShortSize))*pfRealShortTwiddle[n-1];
	}


}

IMDCTObject::IMDCTObject()
	:iLongSize(0),
	iOverlap(0)
{
}

IMDCTObject::IMDCTObject(IMDCTObject&oCopy)
	:iLongSize(oCopy.iLongSize),
	iHLongSize(iLongSize/2),
	iOverlap(oCopy.iOverlap),
	iOldShape(oCopy.iOldShape)
{

	if(iLongSize==0){
		return;
	}
	if(iOverlap==0){
		return;
	}
	int n,k;
	
	//Obtain memory for Short Windows:
	//These arrays are larger than they need to be,
	//but are made this large for convenience
	iShortSize = iLongSize / 8;
	iHShortSize = iHLongSize / 8;
	pfShortWindowSin = new float[iHLongSize];
	pfShortWindowKBD = new float[iHLongSize];
	
	//Obtain memory for windows:
	pfLongWindowSin=new float[iHLongSize];
	pfS2LWindowSin=new float[iHLongSize];

	pfLongWindowKBD=new float[iHLongSize];
	pfS2LWindowKBD=new float[iHLongSize];
	
	//Obtain memory for temporary buffer:
	pfTempBuffer=new float[iLongSize];
	pfEightWindowsBuffer=new float[iLongSize];
	pfSavedBuffer=new float[iHLongSize];
	for(n=0;n<iHLongSize;n++) pfSavedBuffer[n]=0;
	
	//Create Windows:
	KBD_Window(pfLongWindowKBD,iLongSize);
	Sine_Window(pfLongWindowSin,iLongSize);
	
	//Create S2L Window:
	KBD_Window(pfShortWindowKBD,iOverlap*2); // temporary
	for(k=0,n=iHLongSize/2-iOverlap/2;k<iOverlap;k++,n++){
		pfS2LWindowKBD[n]=pfShortWindowKBD[k];
	}
	for(n=0;n<iHLongSize/2-iOverlap/2;n++){
		pfS2LWindowKBD[n]=0.0f;
	}
	for(n=iHLongSize/2+iOverlap/2;n<iHLongSize;n++){
		pfS2LWindowKBD[n]=1.0f;
	}

	//Create correct length short window
	KBD_Window(pfShortWindowKBD,iShortSize);

	//Create S2L Window:
	Sine_Window(pfShortWindowSin,iOverlap*2); // temporary
	for(k=0,n=iHLongSize/2-iOverlap/2;k<iOverlap;k++,n++){
		pfS2LWindowSin[n]=pfShortWindowSin[k];
	}
	for(n=0;n<iHLongSize/2-iOverlap/2;n++){
		pfS2LWindowSin[n]=0.0f;
	}
	for(n=iHLongSize/2+iOverlap/2;n<iHLongSize;n++){
		pfS2LWindowSin[n]=1.0f;
	}

	//Create correct length short window
	Sine_Window(pfShortWindowSin,iShortSize);
	
	// Compute Long Twiddles
	iLongFFTSize=iLongSize/4;
	pfReal=new float[iLongFFTSize];
	pfImag=new float[iLongFFTSize];
	pfRealLongTwiddle=new float[iLongFFTSize];
	pfImagLongTwiddle=new float[iLongFFTSize];
	pfRealLongTwiddle[0]=(float)(cos(0.125*2.0*pi/(float)iLongSize));
	pfImagLongTwiddle[0]=(float)(sin(0.125*2.0*pi/(float)iLongSize));
	for(n=1;n<iLongFFTSize;n++){
		pfRealLongTwiddle[n]=(float)(cos(2.0*pi/(float)iLongSize))*pfRealLongTwiddle[n-1]-
									(float)(sin(2.0*pi/(float)iLongSize))*pfImagLongTwiddle[n-1];
		pfImagLongTwiddle[n]=(float)(cos(2.0*pi/(float)iLongSize))*pfImagLongTwiddle[n-1]+
									(float)(sin(2.0*pi/(float)iLongSize))*pfRealLongTwiddle[n-1];
	}

	// Compute Short Twiddles
	// these twiddles may be redundant - they may be able to be computed as a function
	// of the long twiddles
	iShortFFTSize=iShortSize/4;
	pfRealShort=new float[iShortFFTSize];
	pfImagShort=new float[iShortFFTSize];
	pfRealShortTwiddle=new float[iShortFFTSize];
	pfImagShortTwiddle=new float[iShortFFTSize];
	pfRealShortTwiddle[0]=(float)(cos(0.125*2.0*pi/(float)iShortSize));
	pfImagShortTwiddle[0]=(float)(sin(0.125*2.0*pi/(float)iShortSize));
	for(n=1;n<iShortFFTSize;n++){
		pfRealShortTwiddle[n]=(float)(cos(2.0*pi/(float)iShortSize))*pfRealShortTwiddle[n-1]-
									(float)(sin(2.0*pi/(float)iShortSize))*pfImagShortTwiddle[n-1];
		pfImagShortTwiddle[n]=(float)(cos(2.0*pi/(float)iShortSize))*pfImagShortTwiddle[n-1]+
									(float)(sin(2.0*pi/(float)iShortSize))*pfRealShortTwiddle[n-1];
	}

}

IMDCTObject& IMDCTObject::operator=(IMDCTObject&oCopy)
{
	if(iLongSize!=0&&iOverlap!=0){
		delete[] pfLongWindowKBD;
		delete[] pfS2LWindowKBD;
		
		delete[] pfLongWindowSin;
		delete[] pfS2LWindowSin;

		delete[] pfShortWindowSin;
		delete[] pfShortWindowKBD;

		delete[] pfSavedBuffer;
		delete[] pfReal;
		delete[] pfImag;
		delete[] pfRealLongTwiddle;
		delete[] pfImagLongTwiddle;

		delete[] pfRealShort;
		delete[] pfImagShort;
		delete[] pfRealShortTwiddle;
		delete[] pfImagShortTwiddle;

	}

	iLongSize=oCopy.iLongSize;
	iHLongSize=iLongSize/2;

	iShortSize=oCopy.iShortSize;
	iHShortSize=iShortSize/2;
	
	iOverlap=oCopy.iOverlap;
	iOldShape=oCopy.iOldShape;
	if(iLongSize==0){
		return *this;
	}
	if(iOverlap==0){
		return *this;
	}
	int n,k;
	
	//Obtain memory for Short Windows:
	//These arrays are larger than they need to be,
	//but are made this large for convenience
	iShortSize = iLongSize / 8;
	iHShortSize = iHLongSize / 8;
	pfShortWindowSin = new float[iHLongSize];
	pfShortWindowKBD = new float[iHLongSize];
	
	//Obtain memory for windows:
	pfLongWindowSin=new float[iHLongSize];
	pfS2LWindowSin=new float[iHLongSize];

	pfLongWindowKBD=new float[iHLongSize];
	pfS2LWindowKBD=new float[iHLongSize];
	
	//Obtain memory for temporary buffer:
	pfTempBuffer=new float[iLongSize];
	pfEightWindowsBuffer=new float[iLongSize];
	pfSavedBuffer=new float[iHLongSize];
	for(n=0;n<iHLongSize;n++) pfSavedBuffer[n]=0;
	
	//Create Windows:
	KBD_Window(pfLongWindowKBD,iLongSize);
	Sine_Window(pfLongWindowSin,iLongSize);
	
	//Create S2L Window:
	KBD_Window(pfShortWindowKBD,iOverlap*2); // temporary
	for(k=0,n=iHLongSize/2-iOverlap/2;k<iOverlap;k++,n++){
		pfS2LWindowKBD[n]=pfShortWindowKBD[k];
	}
	for(n=0;n<iHLongSize/2-iOverlap/2;n++){
		pfS2LWindowKBD[n]=0.0f;
	}
	for(n=iHLongSize/2+iOverlap/2;n<iHLongSize;n++){
		pfS2LWindowKBD[n]=1.0f;
	}

	//Create correct length short window
	KBD_Window(pfShortWindowKBD,iShortSize);

	//Create S2L Window:
	Sine_Window(pfShortWindowSin,iOverlap*2); // temporary
	for(k=0,n=iHLongSize/2-iOverlap/2;k<iOverlap;k++,n++){
		pfS2LWindowSin[n]=pfShortWindowSin[k];
	}
	for(n=0;n<iHLongSize/2-iOverlap/2;n++){
		pfS2LWindowSin[n]=0.0f;
	}
	for(n=iHLongSize/2+iOverlap/2;n<iHLongSize;n++){
		pfS2LWindowSin[n]=1.0f;
	}

	//Create correct length short window
	Sine_Window(pfShortWindowSin,iShortSize);
	
	// Compute Long Twiddles
	iLongFFTSize=iLongSize/4;
	pfReal=new float[iLongFFTSize];
	pfImag=new float[iLongFFTSize];
	pfRealLongTwiddle=new float[iLongFFTSize];
	pfImagLongTwiddle=new float[iLongFFTSize];
	pfRealLongTwiddle[0]=(float)(cos(0.125*2.0*pi/(float)iLongSize));
	pfImagLongTwiddle[0]=(float)(sin(0.125*2.0*pi/(float)iLongSize));
	for(n=1;n<iLongFFTSize;n++){
		pfRealLongTwiddle[n]=(float)(cos(2.0*pi/(float)iLongSize))*pfRealLongTwiddle[n-1]-
									(float)(sin(2.0*pi/(float)iLongSize))*pfImagLongTwiddle[n-1];
		pfImagLongTwiddle[n]=(float)(cos(2.0*pi/(float)iLongSize))*pfImagLongTwiddle[n-1]+
									(float)(sin(2.0*pi/(float)iLongSize))*pfRealLongTwiddle[n-1];
	}

	// Compute Short Twiddles
	// these twiddles may be redundant - they may be able to be computed as a function
	// of the long twiddles
	iShortFFTSize=iShortSize/4;
	pfRealShort=new float[iShortFFTSize];
	pfImagShort=new float[iShortFFTSize];
	pfRealShortTwiddle=new float[iShortFFTSize];
	pfImagShortTwiddle=new float[iShortFFTSize];
	pfRealShortTwiddle[0]=(float)(cos(0.125*2.0*pi/(float)iShortSize));
	pfImagShortTwiddle[0]=(float)(sin(0.125*2.0*pi/(float)iShortSize));
	for(n=1;n<iShortFFTSize;n++){
		pfRealShortTwiddle[n]=(float)(cos(2.0*pi/(float)iShortSize))*pfRealShortTwiddle[n-1]-
									(float)(sin(2.0*pi/(float)iShortSize))*pfImagShortTwiddle[n-1];
		pfImagShortTwiddle[n]=(float)(cos(2.0*pi/(float)iShortSize))*pfImagShortTwiddle[n-1]+
									(float)(sin(2.0*pi/(float)iShortSize))*pfRealShortTwiddle[n-1];
	}

	return *this;
}

IMDCTObject::~IMDCTObject()
{
	if(iLongSize!=0&&iOverlap!=0){
		delete[] pfLongWindowKBD;
		delete[] pfS2LWindowKBD;
		
		delete[] pfLongWindowSin;
		delete[] pfS2LWindowSin;

		delete[] pfShortWindowSin;
		delete[] pfShortWindowKBD;

		delete[] pfSavedBuffer;
		delete[] pfReal;
		delete[] pfImag;
		delete[] pfRealLongTwiddle;
		delete[] pfImagLongTwiddle;

		delete[] pfRealShort;
		delete[] pfImagShort;
		delete[] pfRealShortTwiddle;
		delete[] pfImagShortTwiddle;

        delete[] pfTempBuffer;
        delete[] pfEightWindowsBuffer;

	}
}

int IMDCTObject::Transform(	float*pfData,
							int iWindowType,
							int iWindowShape)
{
	int n,k;
	float *pfWindowFirst;
	float *pfWindowSecond;
	int eightShortSequenceFlag;
	float* pfDataShortBlock;

	switch(iWindowType){
		case 0:
			if(iOldShape == SINE_WINDOW){
				pfWindowFirst=pfLongWindowSin;
			}
			else{
				pfWindowFirst=pfLongWindowKBD;
			}
			if(iWindowShape == SINE_WINDOW){
				pfWindowSecond=pfLongWindowSin;
			}
			else{
				pfWindowSecond=pfLongWindowKBD;
			}
			eightShortSequenceFlag = 0;
		break;
		case 1:
			if(iOldShape == SINE_WINDOW){
				pfWindowFirst=pfLongWindowSin;
			}
			else{
				pfWindowFirst=pfLongWindowKBD;
			}
			if(iWindowShape == SINE_WINDOW){
				pfWindowSecond=pfS2LWindowSin;
			}
			else{
				pfWindowSecond=pfS2LWindowKBD;
			}
			eightShortSequenceFlag = 0;
		break;
		case 2:
			eightShortSequenceFlag = 1;
		break;
		case 3:
			if(iOldShape == SINE_WINDOW){
				pfWindowFirst=pfS2LWindowSin;
			}
			else{
				pfWindowFirst=pfS2LWindowKBD;
			}
			if(iWindowShape == SINE_WINDOW){
				pfWindowSecond=pfLongWindowSin;
			}
			else{
				pfWindowSecond=pfLongWindowKBD;
			}
			eightShortSequenceFlag = 0;
		break;
		default:
			//printf("Invalid Window Type\n");
			return -1;
	}

	if (eightShortSequenceFlag == 0)
	{
		//Copy Data
		memcpy(pfTempBuffer,pfData,iHLongSize*sizeof(float));

		//IMDCT:
		imdct(pfTempBuffer, LONG_BLOCK);
		//DGC Goes Here
		//Window
		for(n=0,k=iHLongSize-1;n<iHLongSize;n++,k--){
			pfTempBuffer[n]*=pfWindowFirst[n];
			pfTempBuffer[n+iHLongSize]*=pfWindowSecond[k];
		}
	} else {
		//Copy coefficient Data to a (possibly larger) array in preparation
		//for imdct. 
		//CNC: the use of the constact 256 is really ugly here!!!
		//256 = (2 * CShortBlock::MaximumBins), but this expression
		//is inconvenient to insert in the code below due to access restrictions.
		//The hardcoding of the constant 256 refers to the fixed spacing
		//of the mdct coefficients in the data pfData, independent of whether
		//or not we're dealing with a single or a double length transform.
		//Note: the imdct only requires N/2 freq. coefficients to
		//produce N (time aliased) time samples. In the case of a double
		//length transform, we'll need to copy the spectrally extended
		//short blocks, one by one, to their proper place in the larger
		//array, pfEightWindowsBuffer. Since the source and destination
		//arrays might be of different sizes due to using a single *or* a
		//double length transform, the block-to-block spacing of the mdct
		//coefficients could be different in each of the arrays. 
		for (n=0;n<8;n++)
		{
			memcpy(&pfEightWindowsBuffer[n*iShortSize],
				   &pfData[n*256],
				   256*sizeof(float));
		}

		// clear the TempBuffer, which will hold the time-aliased
		// output for the entire frame (all overlap-added short blocks)
		for (k = 0 ; k < iLongSize ; k++)
		{
		  pfTempBuffer[k] = 0 ;     
		}

		for(n=0;n<8;n++)
		{
			// IMDCT
			pfDataShortBlock = &pfEightWindowsBuffer[n*iShortSize];
			imdct(pfDataShortBlock,SHORT_BLOCK);

			// DGC goes here

			// Window the time-aliased, time domain data.. Take into account possible window shape switching on
			// the first of the eight short windows.
			// Note that the AAC standard says that 
			// "The window shape of the previous block influences the first of the eight short blocks (w_0(m)) only."
			// ISO/IEC 13818-7:1997(E), page 78.
			// This means that the rising edge of the zeroth short block must match the previous window frame's window 
			// shape. However, all subsequent rising and falling edges of remaining short blocks should reflect the
			// currently decoded window shape type.

			if (n==0)
			{
				// rising edge depends on old shape
				if (iOldShape == SINE_WINDOW) 
				{
					for (k=0;k<iHShortSize;k++)
					{
						pfDataShortBlock[k]*=pfShortWindowSin[k];
					}
				}
				else if (iOldShape == KBD_WINDOW) 
				{
					for (k=0;k<iHShortSize;k++)
					{
						pfDataShortBlock[k]*=pfShortWindowKBD[k];
					}
				}

				// falling edge reflects new shape
				if (iWindowShape == SINE_WINDOW) 
				{
					for (k=0;k<iHShortSize;k++)
					{
						pfDataShortBlock[k+iHShortSize]*=pfShortWindowSin[iHShortSize-k-1];
					}
				}
				else if (iWindowShape == KBD_WINDOW) 
				{
					for (k=0;k<iHShortSize;k++)
					{
						pfDataShortBlock[k+iHShortSize]*=pfShortWindowKBD[iHShortSize-k-1];
					}
				}
			} 
			else // n != 0
			{
				if (iWindowShape == SINE_WINDOW) 
				{
					for (k=0;k<iHShortSize;k++)
					{
						pfDataShortBlock[k]*=pfShortWindowSin[k];
						pfDataShortBlock[k+iHShortSize]*=pfShortWindowSin[iHShortSize-k-1];
					}
				}
				else if (iWindowShape == KBD_WINDOW) 
				{
					for (k=0;k<iHShortSize;k++)
					{
						pfDataShortBlock[k]*=pfShortWindowKBD[k];
						pfDataShortBlock[k+iHShortSize]*=pfShortWindowKBD[iHShortSize-k-1];
					}
				}
			}
			// overlap add the 8 windows in this block
			for (k = 0 ; k < iShortSize ; k++)
			{
			  pfTempBuffer[(iLongSize/4 - iShortSize/4) + (iHShortSize * n) + k] += pfDataShortBlock[k] ;     
			}

		} // for n...

	} // if (eightShortSequenceFlag...)

	//OLA:
	for(n=0;n<iHLongSize;n++){
		pfData[n]=pfTempBuffer[n]+pfSavedBuffer[n];
	}
	//Update Saved Buffer:
	memcpy(pfSavedBuffer,&pfTempBuffer[iHLongSize],iHLongSize*sizeof(float));

	iOldShape=iWindowShape;
	return 0;
}

void IMDCTObject::imdct(float*pfData, int blockType)
{
	float fTempReal,fTempImag;
	float fFactor;
	int n,k,iLast,iRotation,iMiddle,iSize;

	float *realData, *imagData, *realTwiddle, *imagTwiddle;

	if (blockType == LONG_BLOCK)
	{
		iLast=iLongFFTSize;
		iRotation=iLongFFTSize/2;
		iMiddle=iHLongSize;
		iSize=iLongSize;
		fFactor=2.0f/(float)iLongSize; 
		realData = pfReal;
		imagData = pfImag;
		realTwiddle = pfRealLongTwiddle;
		imagTwiddle = pfImagLongTwiddle;
	}
	else if (blockType == SHORT_BLOCK)
	{
		iLast=iShortFFTSize;
		iRotation=iShortFFTSize/2;
		iMiddle=iHShortSize;
		iSize=iShortSize;
		fFactor=2.0f/(float)iShortSize; 
		realData = pfRealShort;
		imagData = pfImagShort;
		realTwiddle = pfRealShortTwiddle;
		imagTwiddle = pfImagShortTwiddle;
	}

	for(k=0;k<iLast;k++){
		n=2*k;
		fTempReal=-pfData[n];
		fTempImag=pfData[iMiddle-1-n];

		realData[k]=fTempReal*realTwiddle[k]-fTempImag*imagTwiddle[k];
		imagData[k]=-fTempImag*realTwiddle[k]-fTempReal*imagTwiddle[k];
	}
	fftl(realData,imagData,iLast);
	for(k=0;k<iLast;k++){
		//conj!
		fTempReal=fFactor*(realData[k]*realTwiddle[k]+imagData[k]*imagTwiddle[k]);
		fTempImag=fFactor*(-imagData[k]*realTwiddle[k]+realData[k]*imagTwiddle[k]);

		n=2*k;
		pfData[iMiddle+iLast-1-n]=fTempReal;
		pfData[iLast+n]=fTempImag;
		if(k<iRotation){
			pfData[iMiddle+iLast+n]=fTempReal;
			pfData[iLast-1-n]=-fTempImag;
		}
		else{
			pfData[n-iLast]=-fTempReal;
			pfData[iLast+iSize-1-n]=fTempImag;
		}
	}
}