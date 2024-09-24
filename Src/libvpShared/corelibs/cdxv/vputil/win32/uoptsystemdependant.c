/****************************************************************************
*
*   Module Title :     SystemDependant.c
*
*   Description  :     Miscellaneous system dependant functions
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
* 
*   1.20 YWX 06-Nov-02 Added forward DCT function optimized for Pentium 4
*   1.19 YWX 15-Jun-01 added function pointer setups for new deblocking filter
*   1.18 YWX 26-Apr-01 Fixed the cpu frequency detection bug caused by Sleep()
*   1.17 JBX 22-Mar-01 Merged with new vp4-mapca bitstream
*   1.16 JBB 26-Jan-01 Cleaned out unused function
*   1.15 YWX 08-dec-00 Added WMT PostProcessor and 
*                        moved function declarations into _head files
*   1.14 JBB 30 NOV 00 Version number changes 
*   1.13 YWX 03-Nov-00 Optimized postprocessor filters
*   1.12 YWX 02-Nov-00 Added new loopfilter function pointers
*   1.11 YWX 19-Oct-00 Added 1-2 Scaling functions pointers
*   1.10 jbb 16 oct 00 added ifdefs to insure version code
*   1.09 YWX 04-Oct-00 Added function pointers for scaling 
*   1.08 YWX 06 Sep 00 Added function pointers for new deringing filter 
*                      using frag baseed Q Value.
*   1.07 JBB 21 Aug 00 New More Blurry in high variance area deringer
*	1.06 YWX 2  Aug 00 Added function pointers for postprocess  
*	1.05 YWX 15/05/00  Added functions to check processor frequency
*					   and more function pointers for postprocessor
*	1.04 YWX 08/05/00  Added function pointers setup for postprocess
*   1.03 SJL 20/04/00  Added ability to enable the new dequant code.
*   1.02 SJL 22/03/00  Function pointers for the loop filter.
*   1.01 JBB 21/03/00  More Function Pointers for optimized playback
*   1.00 PGW 12/10/99  Configuration baseline
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/
#include "codec_common.h"
#include "vputil_if.h"
#include "cpuidlib.h"

//global debugging aid's!
int fastIDCTDisabled = 0;
int forceCPUID = 0;
int CPUID = 0;


extern void GetProcessorFlags(INT32 *MmxEnabled, INT32 *XmmEnabled, INT32 *WmtEnabled);

// Scalar (no mmx) reconstruction functions
extern void ClearSysState_C(void);
extern void IDctSlow(  INT16 * InputData, INT16 *QuantMatrix, INT16 * OutputData );
extern void IDct10(  INT16 * InputData, INT16 *QuantMatrix, INT16 * OutputData );
extern void IDct1(  INT16 * InputData, INT16 *QuantMatrix, INT16 * OutputData );
extern void ScalarReconIntra( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT16 * ChangePtr, UINT32 LineStep );
extern void ScalarReconInter( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT8 * RefPtr, INT16 * ChangePtr, UINT32 LineStep );
extern void ScalarReconInterHalfPixel2( INT16 *TmpDataBuffer, UINT8 * ReconPtr,UINT8 * RefPtr1, UINT8 * RefPtr2, INT16 * ChangePtr, UINT32 LineStep );
extern void ReconBlock_C(INT16 *SrcBlock,INT16 *ReconRefPtr, UINT8 *DestBlock, UINT32 LineStep);
extern void SubtractBlock_C( UINT8 *SrcBlock, INT16 *DestPtr, UINT32 LineStep );
extern void UnpackBlock_C( UINT8 *ReconPtr, INT16 *ReconRefPtr, UINT32 ReconPixelsPerLine);
extern void AverageBlock_C( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT16 *ReconRefPtr, UINT32 ReconPixelsPerLine);
extern void CopyBlock_C(unsigned char *src, unsigned char *dest, unsigned int srcstride);
extern void Copy12x12_C(const unsigned char *src, unsigned char *dest, unsigned int srcstride, unsigned int deststride);
extern void fdct_short_C ( INT16 * InputData, INT16 * OutputData );
extern void FilterBlockBil_8_C( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT8 *ReconRefPtr, UINT32 ReconPixelsPerLine, INT32 ModX, INT32 ModY );
extern void FilterBlock_C( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT16 *ReconRefPtr, UINT32 PixelsPerLine, INT32 ModX, INT32 ModY, BOOL UseBicubic, UINT8 BicubicAlpha );

// MMx versions
extern void fdct_MMX ( INT16 * InputData, INT16 * OutputData );
extern void ClearMmx(void);
extern void MMXReconIntra( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT16 * ChangePtr, UINT32 LineStep );
extern void MmxReconInter( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT8 * RefPtr, INT16 * ChangePtr, UINT32 LineStep );
extern void MmxReconInterHalfPixel2( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT8 * RefPtr1, UINT8 * RefPtr2, INT16 * ChangePtr, UINT32 LineStep );
extern void MMX_idct(  Q_LIST_ENTRY * InputData, INT16 *QuantMatrix, INT16 * OutputData );
extern void MMX_idct10(  Q_LIST_ENTRY * InputData, INT16 *QuantMatrix, INT16 * OutputData );
extern void MMX_idct1(  Q_LIST_ENTRY * InputData, INT16 *QuantMatrix, INT16 * OutputData );
extern void MMX_idct_DX(  Q_LIST_ENTRY * InputData, INT16 *QuantMatrix, INT16 * OutputData );
extern void MMX_idct10_DX(  Q_LIST_ENTRY * InputData, INT16 *QuantMatrix, INT16 * OutputData );
extern void ReconBlock_MMX(INT16 *SrcBlock,INT16 *ReconRefPtr, UINT8 *DestBlock, UINT32 LineStep);
extern void SubtractBlock_MMX( UINT8 *SrcBlock, INT16 *DestPtr, UINT32 LineStep );
extern void UnpackBlock_MMX( UINT8 *ReconPtr, INT16 *ReconRefPtr, UINT32 ReconPixelsPerLine);
extern void AverageBlock_MMX( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT16 *ReconRefPtr, UINT32 ReconPixelsPerLine);
extern void CopyBlockMMX(unsigned char *src, unsigned char *dest, unsigned int srcstride);
extern void Copy12x12_MMX(const unsigned char *src, unsigned char *dest, unsigned int srcstride, unsigned int deststride);
extern void FilterBlockBil_8_mmx( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT8 *ReconRefPtr, UINT32 ReconPixelsPerLine, INT32 ModX, INT32 ModY );
extern void FilterBlock_mmx( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT16 *ReconRefPtr, UINT32 PixelsPerLine, INT32 ModX, INT32 ModY, BOOL UseBicubic, UINT8 BicubicAlpha );

// WMT versions
extern void WmtReconIntra( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT16 * ChangePtr, UINT32 LineStep );
extern void WmtReconInter( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT8 * RefPtr, INT16 * ChangePtr, UINT32 LineStep );
extern void WmtReconInterHalfPixel2( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT8 * RefPtr1, UINT8 * RefPtr2, INT16 * ChangePtr, UINT32 LineStep );
extern void Wmt_idct1(  Q_LIST_ENTRY * InputData, INT16 *QuantMatrix, INT16 * OutputData );
extern void Wmt_IDct_Dx( Q_LIST_ENTRY * InputData, INT16 *QuantMatrix, INT16 * OutputData );
extern void Wmt_IDct10_Dx(  Q_LIST_ENTRY * InputData, INT16 *QuantMatrix, INT16 * OutputData );
extern void fdct_WMT(short *InputData, short *OutputData);
extern void FilterBlockBil_8_wmt( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT8 *ReconRefPtr, UINT32 ReconPixelsPerLine, INT32 ModX, INT32 ModY );
extern void FilterBlock_wmt( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT16 *ReconRefPtr, UINT32 PixelsPerLine, INT32 ModX, INT32 ModY, BOOL UseBicubic, UINT8 BicubicAlpha );


#define IdctAdjustBeforeShift 8
extern UINT16 idctconstants[(4+7+1) * 4];
extern UINT16 idctcosTbl[ 7];

void fillidctconstants(void)
{
	int j = 16;  
	UINT16 * p; 
	do 
	{ 
		idctconstants[ --j] = 0;
	}  
	while( j);
	
	idctconstants[0] = idctconstants[5] = idctconstants[10] = idctconstants[15] = 65535;
	
	j = 1; 
	do 
	{
		p = idctconstants + ( (j+3) << 2);
		p[0] = p[1] = p[2] = p[3] = idctcosTbl[ j - 1];
	} 
	while( ++j <= 7);
	
	idctconstants[44] = idctconstants[45] = idctconstants[46] = idctconstants[47] = IdctAdjustBeforeShift;
}

/****************************************************************************
 * 
 *  ROUTINE       :     Get Processor Flags
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Checks for machine specifc features such as MMX support 
 *                      sets approipriate flags and function pointers.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void GetProcessorFlags
( 
 INT32 *MmxEnabled,
 INT32 *XmmEnabled,
 INT32 *WmtEnabled
)
{
	
	PROCTYPE CPUType = findCPUId();
    if(forceCPUID)
        CPUType = CPUID;

	switch(CPUType)
	{
	case X86    :
	case PPRO   :
	case C6X86  :
	case C6X86MX:
	case AMDK5  :
	case MACG3	:
	case MAC68K	:
		*MmxEnabled = FALSE;
		*XmmEnabled = FALSE;
		*WmtEnabled = FALSE;
		break;
	case PII	:   
	case AMDK63D:
	case AMDK6  :
	case PMMX	:   
		*MmxEnabled = TRUE;
		*XmmEnabled = FALSE;
		*WmtEnabled = FALSE;
		break;
	case XMM    :
		*MmxEnabled = TRUE;
		*XmmEnabled = TRUE;
		*WmtEnabled = FALSE;
		break;
	case WMT	:
		*MmxEnabled = TRUE;
		*XmmEnabled = TRUE;
		*WmtEnabled = TRUE;
		break;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     MachineSpecificConfig
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Checks for machine specifc features such as MMX support 
 *                      sets approipriate flags and function pointers.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void UtilMachineSpecificConfig
(
  void
)
{
	UINT32 i;
	INT32 MmxEnabled;
	INT32 XmmEnabled; 
	INT32 WmtEnabled;

	GetProcessorFlags( &MmxEnabled,&XmmEnabled,&WmtEnabled);
    
	if(WmtEnabled)		//Willamette
	{
		for(i=0;i<=64;i++)
		{

            if(fastIDCTDisabled)
                idct[i]=Wmt_IDct_Dx;
            else
            {
    			if(i<=1)idct[i]=Wmt_idct1;
	    		else if(i<=10)idct[i]=Wmt_IDct10_Dx;
		    	else idct[i]=Wmt_IDct_Dx;
            }
		}
		for(i=0;i<=64;i++)
		{
            if(fastIDCTDisabled)
                idctc[i]=MMX_idct;
            else
            {
			    if(i<=1)idctc[i]=Wmt_idct1;
			    else if(i<=10)idctc[i]=MMX_idct10;
			    else idctc[i]=MMX_idct;
            }
		}
        fdct_short=fdct_WMT;

        ReconIntra = WmtReconIntra;
        ReconInter = WmtReconInter;
        ReconInterHalfPixel2 = WmtReconInterHalfPixel2;
		ClearSysState = ClearMmx;
        AverageBlock = AverageBlock_MMX;
        UnpackBlock = UnpackBlock_MMX;
        ReconBlock = ReconBlock_MMX;
        SubtractBlock = SubtractBlock_MMX;
		CopyBlock = CopyBlockMMX;
        Copy12x12 = Copy12x12_MMX;    
        FilterBlockBil_8 = FilterBlockBil_8_wmt;
        FilterBlock=FilterBlock_wmt;
        //FilterBlock=FilterBlock_C;
	}
	else if ( MmxEnabled )
    {
		for(i=0;i<=64;i++)
		{
            if(fastIDCTDisabled)
                idctc[i]=MMX_idct_DX;
            else
            {
    			if(i<=1)idctc[i]=MMX_idct1;
	    		else if(i<=10)idctc[i]=MMX_idct10;
		    	else idctc[i]=MMX_idct;
		    }
        }
        fdct_short=fdct_MMX;
		for(i=0;i<=64;i++)
		{
            if(fastIDCTDisabled)
                idct[i]=MMX_idct_DX;
            else
            {
			    if(i<=1)idct[i]=MMX_idct1;
			    else if(i<=10)idct[i]=MMX_idct10_DX;
			    else idct[i]=MMX_idct_DX;
            }
		}

        ReconIntra = MMXReconIntra;
        ReconInter = MmxReconInter;
        ReconInterHalfPixel2 = MmxReconInterHalfPixel2;
		ClearSysState = ClearMmx;
        AverageBlock = AverageBlock_MMX;
        UnpackBlock = UnpackBlock_MMX;
        ReconBlock = ReconBlock_MMX;
        SubtractBlock = SubtractBlock_MMX;
		CopyBlock = CopyBlockMMX;
        Copy12x12 = Copy12x12_MMX;
        FilterBlockBil_8 = FilterBlockBil_8_mmx;
        FilterBlock=FilterBlock_mmx;
        //FilterBlock=FilterBlock_C;
   }
    else
    {
		int i;
		for(i=0;i<=64;i++)
		{
            if(fastIDCTDisabled)
                idctc[i]=IDctSlow;
            else
            {
			    if(i<=1)idctc[i]=IDct1;
			    else if(i<=10)idctc[i]=IDct10;
			    else idctc[i]=IDctSlow;
            }
		}
		fdct_short=fdct_short_C ;
		for(i=0;i<=64;i++)
		{
            if(fastIDCTDisabled)
                idct[i]=IDctSlow;
            else
            {
			    if(i<=1)idct[i]=IDct1;
			    else if(i<=10)idct[i]=IDct10;
			    else idct[i]=IDctSlow;
            }
		}
		ClearSysState = ClearSysState_C;
		ReconIntra = ScalarReconIntra;
		ReconInter = ScalarReconInter;
		ReconInterHalfPixel2 = ScalarReconInterHalfPixel2;
		AverageBlock = AverageBlock_C;
		UnpackBlock = UnpackBlock_C;
		ReconBlock = ReconBlock_C;
		SubtractBlock = SubtractBlock_C;
		CopyBlock = CopyBlock_C;
        Copy12x12 = Copy12x12_MMX;
        FilterBlockBil_8 = FilterBlockBil_8_C;
        FilterBlock=FilterBlock_C;
    } 
    //FilterBlock=FilterBlock_C;

}
