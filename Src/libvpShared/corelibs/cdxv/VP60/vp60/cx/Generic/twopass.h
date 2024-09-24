/****************************************************************************
*        
*   Module Title :     twopass.h
*
*   Description  :     Functions for handling twopass dataratecontrol
*
****************************************************************************/
#ifndef __INC_TWOPASS_H
#define __INC_TWOPASS_H

#ifndef STRICT
#define STRICT              /* Strict type checking */
#endif

/****************************************************************************
*  Module statics
****************************************************************************/        


extern void ZeroStats( FIRSTPASS_STATS *section);
extern void AccumulateStats(FIRSTPASS_STATS *section, FIRSTPASS_STATS *frame);
extern void AvgStats ( FIRSTPASS_STATS *section);
extern void OutputStats( FILE *f, FIRSTPASS_STATS *stats);
extern void InputStats( FILE *f, FIRSTPASS_STATS *stats);
extern void CCONV Pass2Initialize ( CP_INSTANCE *cpi, COMP_CONFIG_VP6 *CompConfig );
extern void CCONV Pass2Control( CP_INSTANCE *cpi);
extern void CCONV Pass1Output( CP_INSTANCE *cpi);

#endif