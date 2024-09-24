/* $Header: /cvs/root/winamp/vlb/fft.cpp,v 1.1 2009/04/28 20:21:09 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *   filename: fft.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: Standard FFT routine
 *
\***************************************************************************/

/* Description:		This function calculates a 1024,256 or 128 point fft via
					decimation in frequency algorithm described in "Digital Signal
					Processing" by Oppenheim and Schafer, refer to pages 304 (flow
					chart) and 330-332 (Fotran program in problem 5). Modified to
					get only desired block sizes. */

#include"fft.h"

#ifndef PI
#define PI	3.14159265358979
#endif

#define HOTSWAP(a,b) fTemp=a; a=b; b=fTemp;
void fftl(float *pfReal, float *pfImag,int iSize)
{
	static int		iInit=0;
	static int		iPower2=0;
	static int		iTransformSize=0;
	static float	*pfTwiddleReal=0;
	static float	*pfTwiddleImag=0;
	
	if(iSize!=iTransformSize){
		iInit=0;
	}
	if(!iInit){
		int iCounter;

		if((iSize-1)&iSize){
			//printf("FFT Length Must be Radix 2!\n");
			return;
		}
		
		free(pfTwiddleReal);
		free(pfTwiddleImag);
		
		iTransformSize=iSize;
		
		pfTwiddleReal=(float*)malloc(iSize/2*sizeof(float));
		pfTwiddleImag=(float*)malloc(iSize/2*sizeof(float));
		
		for(iCounter=0;iCounter<iSize/2;iCounter++){
			pfTwiddleReal[iCounter]=(float)(cos(2.0*PI/(double)iSize*(double)iCounter));
			pfTwiddleImag[iCounter]=(float)(-1.0*sin(2.0*PI/(double)iSize*(double)iCounter));	
		}
		iPower2=0;
		for(iCounter=1;iCounter<iSize;iCounter<<=1) iPower2++;

		iInit++;
	}

	/* Perform Fast Fourier Transform:							*/
	
	/* With Help From Figure 6.11 pp 464						*/
	/* John G. Proakis,"Digital Signal Processing," 3rd Edt.    */
	/* 1996 Prentice Hall										*/

	int iSkip;
	int iFFTCounter;
	int iCounter1;
	int iCounter2;
	int iOffset;
	int iHSize;

	iHSize=iSize>>1;
	iFFTCounter=1;
	for(iSkip=iHSize;iSkip>0;iSkip>>=1){
		iOffset=0;
		for(iCounter1=0;iCounter1<iFFTCounter;iCounter1++){
			int iOffset1;
			int iOffset2;
			int iTwiddleOffset;
			iOffset1=iOffset;
			iOffset2=iOffset1+iSkip;
			iTwiddleOffset=0;
			for(iCounter2=0;iCounter2<iSkip;iCounter2++){	
				float fTemp;
		
				fTemp=pfReal[iOffset1];
				pfReal[iOffset1]+=pfReal[iOffset2];
				pfReal[iOffset2]=fTemp-pfReal[iOffset2];

				fTemp=pfImag[iOffset1];
				pfImag[iOffset1]+=pfImag[iOffset2];
				pfImag[iOffset2]=fTemp-pfImag[iOffset2];
				
				
				if(iCounter2){
					fTemp=pfReal[iOffset2];
					pfReal[iOffset2]=pfReal[iOffset2]*pfTwiddleReal[iTwiddleOffset]-
									 pfImag[iOffset2]*pfTwiddleImag[iTwiddleOffset];
					
					pfImag[iOffset2]=fTemp*pfTwiddleImag[iTwiddleOffset]+
									 pfImag[iOffset2]*pfTwiddleReal[iTwiddleOffset];
				}
				iOffset1++;
				iOffset2++;
				iTwiddleOffset+=iFFTCounter;
			}
			iOffset+=(iSkip<<1);
		}
		iFFTCounter<<=1;
	}

	/*Reorder Data:														*/
	iCounter1=0;
	for(iCounter2=0;iCounter2<iSize-1;iCounter2++){
		int iCounter3;
		if(iCounter2<iCounter1){
			float fTemp;
			HOTSWAP(pfReal[iCounter2],pfReal[iCounter1]);
			HOTSWAP(pfImag[iCounter2],pfImag[iCounter1]);
		}
		iCounter3=iHSize;
		while(iCounter3<=iCounter1){
			iCounter1-=iCounter3;
			iCounter3=iCounter3>>1;
		}
		iCounter1=iCounter1+iCounter3;
	}
}
/*
void fftl(float* xreal,float* ximag,int N)
{
  int m,mm1;
  static int init=0;
  int nv2,nm1,mp;
  static float wreal[13][13],wimag[13][13];
  int i,j,k,l;
  int ip,le,le1;
  float treal,timag,ureal,uimag;
  if (init==0){
	memset((char*) wreal,0,sizeof(wreal));
	memset((char*)wimag,0,sizeof(wimag));
	m=13;
	for(l=0;l<m;l++){
		 le=1<<(m-l);
		 le1=le>>1;
		 wreal[0][l]=(float)(cosl(pi/le1));
		 wimag[0][l]=-1.0f*(float)(sinl(pi/le1));
	}
	m=12;
	for(l=0;l<m;l++){
		 le=1<<(m-l);
		 le1=le>>1;
		 wreal[1][l]=(float)(cosl(pi/le1));
		 wimag[1][l]=-1.0f*(float)(sinl(pi/le1));
	}
	m=11;
	for(l=0;l<m;l++){
		 le=1<<(m-l);
		 le1=le>>1;
		 wreal[2][l]=(float)(cosl(pi/le1));
		 wimag[2][l]=-1.0f*(float)(sinl(pi/le1));
	}
	m=10;
	for(l=0;l<m;l++){
		 le=1<<(m-l);
		 le1=le>>1;
		 wreal[3][l]=(float)(cosl(pi/le1));
		 wimag[3][l]=-1.0f*(float)(sinl(pi/le1));
	}
	m=9;
	for(l=0;l<m;l++){
		 le=1<<(m-l);
		 le1=le>>1;
		 wreal[4][l]=(float)(cosl(pi/le1));
		 wimag[4][l]=-1.0f*(float)(sinl(pi/le1));
	}
	m=8;
	for(l=0;l<m;l++){
		le=1<<(m-l);
		le1=le>>1;
		wreal[5][l]=(float)(cosl(pi/le1));
		wimag[5][l]=-1.0f*(float)(sinl(pi/le1));
	}
	m=7;
	for(l=0;l<m;l++){
		le=1<<(m-l);
		le1=le>>1;
		wreal[6][l]=(float)(cosl(pi/le1));
		wimag[6][l]=-1.0f*(float)(sinl(pi/le1));
	}
	m=6;
	for(l=0;l<m;l++){
		le=1<<(m-l);
		le1=le>>1;
		wreal[7][l]=(float)(cosl(pi/le1));
		wimag[7][l]=-1.0f*(float)(sinl(pi/le1));
	}
	m=5;
	for(l=0;l<m;l++){
		le=1<<(m-l);
		le1=le>>1;
		wreal[8][l]=(float)(cosl(pi/le1));
		wimag[8][l]=-1.0f*(float)(sinl(pi/le1));
	}
	m=4;
	for(l=0;l<m;l++){
		le=1<<(m-l);
		le1=le>>1;
		wreal[9][l]=(float)(cosl(pi/le1));
		wimag[9][l]=-1.0f*(float)(sinl(pi/le1));
	}
	m=3;
	for(l=0;l<m;l++){
		le=1<<(m-l);
		le1=le>>1;
		wreal[10][l]=(float)(cosl(pi/le1));
		wimag[10][l]=-1.0f*(float)(sinl(pi/le1));
	}
	m=2;
	for(l=0;l<m;l++){
		le=1<<(m-l);
		le1=le>>1;
		wreal[11][l]=(float)(cosl(pi/le1));
		wimag[11][l]=-1.0f*(float)(sinl(pi/le1));
	}
	m=1;
	for(l=0;l<m;l++){
		le=1<<(m-l);
		le1=le>>1;
		wreal[12][l]=(float)(cosl(pi/le1));
		wimag[12][l]=-1.0f*(float)(sinl(pi/le1));
	}
	init++;
  }
  switch(N){
	case 8192: m=13;mp=0;break;
	case 4096: m=12;mp=1;break;
	case 2048: m=11;mp=2;break;
	case 1024: m=10;mp=3;break;
	case 512: m=9;mp=4;break;
	case 256:m=8;mp=5;break;
	case 128:m=7;mp=6;break;
	case 64:m=6;mp=7;break;
	case 32:m=5;mp=8;break;
	case 16:m=4;mp=9;break;
	case 8:m=3;mp=10;break;
	case 4:m=2;mp=11;break;
	case 2:m=1;mp=12;break;
	default: printf("FFT ERROR! %d\n",N);return;
  }
  mm1=m-1;
  nv2=N>>1;
  nm1=N-1;
  for(l=0;l<mm1;l++){
	le=1<<(m-l);
	le1=le>>1;
	ureal=1;
	uimag=0;
	for(j=0;j<le1;j++){
		for(i=j;i<N;i+=le){
			ip=i+le1;
			treal=xreal[i]+xreal[ip];
			timag=ximag[i]+ximag[ip];
			xreal[ip]=xreal[i]-xreal[ip];
			ximag[ip]=ximag[i]-ximag[ip];
			xreal[i]=treal;
			ximag[i]=timag;
			treal=xreal[ip];
			xreal[ip]=xreal[ip]*ureal-ximag[ip]*uimag;
			ximag[ip]=ximag[ip]*ureal+treal*uimag;
		}
		treal=ureal;
		ureal=ureal*wreal[mp][l]-uimag*wimag[mp][l];
		uimag=uimag*wreal[mp][l]+treal*wimag[mp][l];
	}
  }
  for(i=0;i<N;i+=2){
	ip=i+1;
	treal=xreal[i]+xreal[ip];
	timag=ximag[i]+ximag[ip];
	xreal[ip]=xreal[i]-xreal[ip];
	ximag[ip]=ximag[i]-ximag[ip];
	xreal[i]=treal;
	ximag[i]=timag;
  }
  j=0;
  for(i=0;i<nm1;i++){
	if(i<j){
		treal=xreal[j];
		timag=ximag[j];
		xreal[j]=xreal[i];
		ximag[j]=ximag[i];
		xreal[i]=treal;
		ximag[i]=timag;
	}
	k=nv2;
	while(k<=j){
		j=j-k;
		k=k>>1;
	}
	j=j+k;
  }
}
*/







