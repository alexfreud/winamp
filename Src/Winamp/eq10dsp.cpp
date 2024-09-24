/*****************************************

   EQ10 library version 1.0
   Copyright (C)2002 4Front Technologies
   Written by George Yohng

   http://www.opensound.com
   
   Proprietary software.
 
 *****************************************/
#include "main.h"
#include "eq10dsp.h"

//#include <stdio.h>
//#include <string.h>
#include <math.h>
#include "WinampAttributes.h"

#define DENORMAL_FIX // comment this for no denormal fixes

char _eq10_copyright[]=
"EQ10 Library version 1.0\n"
"Copyright (C)2002 4Front Technologies  http://www.opensound.com\n"
"Copyright (C)2001-2002 by George Yohng http://www.yohng.com\n\0"
"EQ10 ENGINE";

static double eq10_freq[EQ10_NOFBANDS]={ 70, 180, 320, 600, 1000, 3000, 6000, 12000, 14000, 16000 }; // winamp style frequency table;
static double eq10_freq_iso[EQ10_NOFBANDS]={31,62,125,250,500,1000,2000,4000,8000,16000}; // ISO frequency table

#ifdef EQ10_DQ
static double eq10_q[EQ10_NOFBANDS]=EQ10_DQ;
#endif


static void eq10_bsetup2(int u,double rate,eq10band_t *band,double freq,double Q)
{
    double angle;
    double a0,/*a1,a2,*/b0,b1,b2,alpha;

    if (rate<4000.0)     rate=4000.0;
    if (rate>384000.0)   rate=384000.0;
    if (freq<20.0)       freq=20.0;
    if (freq>=(rate*0.499)) {band->ua0=band->da0=0;return;}

    angle = 2.0*3.1415926535897932384626433832795*freq/rate;
    alpha = sin(angle)/(2.0*Q);

    b0 = 1.0/(1.0+alpha); 
    a0 = b0*alpha;  
    b1 = b0*2*cos(angle); 
    b2 = b0*(alpha-1);

    if (u>0) 
    {
        band->ua0=a0;
        band->ub1=b1;
        band->ub2=b2;
    }
		else
    {
        band->da0=a0;
        band->db1=b1;
        band->db2=b2;
    }
}

static void eq10_bsetup(double rate,eq10band_t *band,double freq,double Q)
{
    memset(band,0,sizeof(*band));
    eq10_bsetup2(-1,rate,band,freq,Q*0.5);
    eq10_bsetup2(1,rate,band,freq,Q*2.0);
#ifdef EQ10_DETECTOR_CODE
    /* release of detector */
    band->detectdecay=pow(0.001,1.0/(rate*EQ10_DETECTOR_RELEASE)); 
#endif
}

void eq10_setup(eq10_t *eq, int eqs, double rate)
{
    int t,k;

    for(k=0;k<eqs;k++,eq++)
    {
        for(t=0;t<EQ10_NOFBANDS;t++)
#ifndef EQ10_DQ
					eq10_bsetup(rate,&eq->band[t],(config_eq_frequencies==EQ_FREQUENCIES_WINAMP)?eq10_freq[t]:eq10_freq_iso[t],EQ10_Q);
#else
            eq10_bsetup(rate,&eq->band[t],(config_eq_frequencies==EQ_FREQUENCIES_WINAMP)?eq10_freq[t]:eq10_freq_iso[t],eq10_q[t]);
#endif

        eq->detect=0;
        /* release of trimmer */
        eq->detectdecay=pow(0.001,1.0/(rate*EQ10_TRIM_RELEASE)); 
    }
}

void eq10_processf(eq10_t *eq,float *buf,float *outbuf,int sz,int idx,int step)
{
    int t,k;
    float *in,*out;
	if (!eq) return;
	
    buf+=idx;
    outbuf+=idx;
            
    in=buf;

    for(k=0;k<EQ10_NOFBANDS;k++)
    {
        double a0,b1,b2;
        double x1 = eq->band[k].x1;
        double x2 = eq->band[k].x2;
        double y1 = eq->band[k].y1;
        double y2 = eq->band[k].y2;
        double gain = eq->band[k].gain;

#ifdef EQ10_DETECTOR_CODE
        double detect = eq->band[k].detect;
        double detectdecay = eq->band[k].detectdecay;
#endif

        
        out = outbuf;

        if (gain>0.0)
        {
            a0 = eq->band[k].ua0*gain;
            b1 = eq->band[k].ub1;
            b2 = eq->band[k].ub2;
        }
				else
        {
            a0 = eq->band[k].da0*gain;
            b1 = eq->band[k].db1;
            b2 = eq->band[k].db2;
        }

        if (a0==0.0) continue;

        for(t=0;t<sz;t++,in+=step,out+=step)
        {
            double y0 = (in[0]-x2)*a0 + y1*b1 + y2*b2 

#ifdef DENORMAL_FIX            
            + 1e-30;
#else
            ;
#endif

#ifdef EQ10_DETECTOR_CODE
            if (fabs(y0)>detect) detect=fabs(y0);
            detect*=detectdecay;

#ifdef DENORMAL_FIX
            detect+=1e-30;
#endif

#endif
            x2=x1; x1=in[0]; y2=y1; y1=y0;

            out[0] = (float)(y0 + in[0]);
        }

        in=outbuf;

        eq->band[k].x1=x1;
        eq->band[k].x2=x2;
        eq->band[k].y1=y1;
        eq->band[k].y2=y2;

#ifdef EQ10_DETECTOR_CODE
        eq->band[k].detect=detect;
#endif
    }

		if (config_eq_limiter)
    {
        double detect=eq->detect;
        double  detectdecay=eq->detectdecay;
        out=outbuf;
        for(t=0;t<sz;t++,in+=step,out+=step)
        {
            /* *0.99 - reserve */
            if (fabs(in[0])>detect) detect=fabs(in[0]);


            if (detect>EQ10_TRIM_CODE) 
                out[0]=in[0]*(float)(EQ10_TRIM_CODE/detect);
            else
                out[0]=in[0];

            detect*=detectdecay;
#ifdef DENORMAL_FIX
            detect+=1e-30;
#endif
        }
        eq->detect=detect;
    }
		else if ((in==buf)&&(buf!=outbuf))
    { 
        out=outbuf;
        for(t=0;t<sz;t++,in+=step,out+=step) out[0]=in[0];
    }

}

double eq10_db2gain(double gain_dB)
{
    return pow(10.0,gain_dB/20.0)-1.0;
}

double eq10_gain2db(double gain)
{
    return 20.0*log10(gain+1.0);
}


void eq10_setgain(eq10_t *eq,int eqs,int bandnr,double gain_dB)
{
    double realgain;
    int k;

		if (!eq)
		return;

			realgain=eq10_db2gain(gain_dB);

    for(k=0;k<eqs;k++,eq++)
        eq->band[bandnr].gain=realgain;
}

double eq10_getgain(eq10_t *eq,int bandnr)
{
    return eq10_gain2db(eq->band[bandnr].gain);
}

double eq10_detect(eq10_t *eq,int bandnr)
{
#ifdef EQ10_DETECTOR_CODE
    return eq10_gain2db(eq->band[bandnr].detect);
#else
    return 0;
#endif    
}


#ifdef TESTCASE


eq10_t eq;

float buf1[4096] = {0};
float buf[4096] = {0};

int main()
{
    int t,k;
    eq10_setup(&eq,1,44100);

    for(t=0;t<4096;t++)
    {
        buf1[t]=warand()*(1.0/16384);
    }

    for(t=0;t<EQ10_NOFBANDS;t++) eq.band[t].gain=-0.874107;

    for(t=0;t<10000;t++)
    {
        eq10_processf(&eq,buf1,buf,4096,0,1); 
    }

    return 0;
}

#endif

