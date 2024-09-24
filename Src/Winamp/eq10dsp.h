/*****************************************

   EQ10 library version 1.0
   Copyright (C)2002 4Front Technologies
   Written by George Yohng

   http://www.opensound.com
   
   Proprietary software.
 
 *****************************************/


#ifndef EQ10DSP_H_INCLUDED
#define EQ10DSP_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

/* used for volume detectors. for instance, if you want to plot
   frequency response, you can use "detect" variable of needed
   subband to query level of that frequency band.
   
   release time - is the time in seconds in which detector falls back
   to zero, if no peaks detected */


// #define EQ10_DETECTOR_CODE           /* uncomment this to  */
// #define EQ10_DETECTOR_RELEASE  1.0f  /* enable band detector */


/* Dynamic limiter, which prevents EQ from distortion. In no case you 
   can overflow EQ and cause it to clip */

#define EQ10_TRIM_CODE    0.930  /* trim at -0.6dB */
#define EQ10_TRIM_RELEASE 0.700  /* trim release, in seconds */


#define EQ10_NOFBANDS 10     /* want more bands? not a problem */

#define EQ10_Q        1.41  /* global `Q' factor */
                                                                                    
/* if you want separate Q per each band, comment global Q and uncomment
   the following array */

//#define EQ10_DQ     {1.4,1.4,1.4,1.4,1.4,1.4,1.4,1.4,1.4,1.4}

/* frequency table compatible to Q10 standard */




typedef
struct eq10band_s
{
    double gain;    /* gain of current band. Do not use this value, 
                      use eq10_setgain instead */

#ifdef EQ10_DETECTOR_CODE
    double detect;  /* band detector value, do not use. 
                      use eq10_detect to read detector value in dB */

    double detectdecay;  /* internal - do not use */
#endif

    double ua0,ub1,ub2;  /* internal - do not use */
    double da0,db1,db2;  /* internal - do not use */
    double x1,x2,y1,y2;  /* internal - do not use */

} eq10band_t;


typedef
struct eq10_s
{
    double rate;         /* sample rate; do not modify */
                        /* use eq10_setup to change */

    eq10band_t band[EQ10_NOFBANDS]; /* bands of equalizer */

    double detect;       /* global detector value. do not use */
    double detectdecay;  /* internal - do not use */

} eq10_t;



double eq10_db2gain(double gain_dB); /* converts decibels to internal gain value*/
double eq10_gain2db(double gain);    /* converts internal gain value to decibels*/


/* prepare eq array for processing, 

   eq   - pointer to array,
   eqs  - number of elements in array (number of audio channels)
   rate - sample rate

   WARNING! this function resets all data in eq and sets all gains to 0dB
*/
void eq10_setup(eq10_t *eq, int eqs, double rate);



/* set band gain */
/*
   eq     - pointer to array,
   eqs    - number of elements in array (number of audio channels)
   bandnr - # of band (0...EQ_NOFBANDS-1)
*/
void eq10_setgain(eq10_t *eq,int eqs,int bandnr,double gain_dB);


/* get current band gain */
/* eq - pointer to element, possible to read gain on each channel
        separately */
double eq10_getgain(eq10_t *eq,int bandnr);


/* get detector value */
/* eq - pointer to element, possible to read detector value on 
        each channel separately */
double eq10_detect(eq10_t *eq,int bandnr);


/* process function

   eq     - pointer to eq structure, corresponding to wanted channel
   buf    - input buffer (interleaved multichannel)
   outbuf - output buffer
   sz     - number of samples in input buffer 
   idx    - index of processed channel (0...N-1)
   step   - total number of channels in interleaved stream (N)

*/

void eq10_processf(eq10_t *eq,float *buf,float *outbuf,int sz,int idx,int step);


/*

Example:

    #define NCHAN 6
    
    ...

    eq10_t eq[NCHAN]; // we process 5.1 data, thus 6 channels
    int t;
    eq10_t *peq;

    ...

    eq10_setup(eq,NCHAN,44100); // initialize

    ...

    eq10_setgain(eq,NCHAN, 5, -10.0f ); // set -10dB for gain6 (nr's from zero)
    ...

    while (bla bla bla) // inner loop
    {
        for(t=0, peq=eq; t<NCHAN; t++, peq++)
        {
            eq10_processf(peq, input_buf, output_buf, cSamples, t, NCHAN);
        }
    }

    ...

*/

#ifdef __cplusplus
}
#endif
#endif //EQ10DSP_H_INCLUDED
