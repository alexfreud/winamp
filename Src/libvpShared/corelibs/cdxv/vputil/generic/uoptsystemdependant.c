/****************************************************************************
*
*   Module Title :     SystemDependant.c
*
*   Description  :     Miscellaneous system dependant functions.
*
****************************************************************************/

/****************************************************************************
*  Header Files
****************************************************************************/
#include "codec_common.h"
#include "vputil_if.h"

/****************************************************************************
*  Exports
****************************************************************************/
// Scalar (no mmx) reconstruction functions
extern void ClearSysState_C ( void );
extern void IDctSlow ( INT16 *InputData, INT16 *QuantMatrix, INT16 *OutputData );
extern void IDct10 ( INT16 *InputData, INT16 *QuantMatrix, INT16 *OutputData );
extern void IDct1 ( INT16 *InputData, INT16 *QuantMatrix, INT16 *OutputData );
extern void ScalarReconIntra ( INT16 *TmpDataBuffer, UINT8 *ReconPtr, UINT16 *ChangePtr, UINT32 LineStep );
extern void ScalarReconInter ( INT16 *TmpDataBuffer, UINT8 *ReconPtr, UINT8 *RefPtr, INT16 *ChangePtr, UINT32 LineStep );
extern void ScalarReconInterHalfPixel2 ( INT16 *TmpDataBuffer, UINT8 *ReconPtr,UINT8 *RefPtr1, UINT8 *RefPtr2, INT16 *ChangePtr, UINT32 LineStep );
extern void ReconBlock_C(INT16 *SrcBlock,INT16 *ReconRefPtr, UINT8 *DestBlock, UINT32 LineStep );
extern void SubtractBlock_C ( UINT8 *SrcBlock, INT16 *DestPtr, UINT32 LineStep );
extern void UnpackBlock_C ( UINT8 *ReconPtr, INT16 *ReconRefPtr, UINT32 ReconPixelsPerLine );
extern void AverageBlock_C ( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT16 *ReconRefPtr, UINT32 ReconPixelsPerLine );
extern void CopyBlock_C ( unsigned char *src, unsigned char *dest, unsigned int srcstride );
extern void Copy12x12_C ( const unsigned char *src, unsigned char *dest, unsigned int srcstride, unsigned int deststride );
extern void fdct_short_C ( INT16 *InputData, INT16 *OutputData );
extern void FilterBlockBil_8_C( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT8 *ReconRefPtr, UINT32 ReconPixelsPerLine, INT32 ModX, INT32 ModY );
extern void FilterBlock_C( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT16 *ReconRefPtr, UINT32 PixelsPerLine, INT32 ModX, INT32 ModY, BOOL UseBicubic, UINT8 BicubicAlpha );
extern void GetProcessorFlags ( INT32 *MmxEnabled, INT32 *XmmEnabled, INT32 *WmtEnabled );

/****************************************************************************
 * 
 *  ROUTINE       :     fillidctconstants
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     STUB FUNCTION.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void fillidctconstants ( void )
{
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
 ****************************************************************************/
void UtilMachineSpecificConfig ( void )
{
		int i;
		for(i=0;i<=64;i++)
		{
			if(i<=1)idctc[i]=IDct1;
			else if(i<=10)idctc[i]=IDct10;
			else idctc[i]=IDctSlow;
		}
		fdct_short=fdct_short_C ;
		for(i=0;i<=64;i++)
		{
			if(i<=1)idct[i]=IDct1;
			else if(i<=10)idct[i]=IDct10;
			else idct[i]=IDctSlow;
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
        Copy12x12 = Copy12x12_C;
        FilterBlockBil_8 = FilterBlockBil_8_C;
        FilterBlock=FilterBlock_C;
}
