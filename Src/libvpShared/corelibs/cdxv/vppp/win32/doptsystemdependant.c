/****************************************************************************
*
*   Module Title :     SystemDependant.c
*
*   Description  :     Miscellaneous system dependant functions
*
****************************************************************************/

/****************************************************************************
*  Header Files
****************************************************************************/ 
#include "postp.h"
 
/****************************************************************************
*  Imports
*****************************************************************************/
extern void GetProcessorFlags(INT32 *MmxEnabled, INT32 *XmmEnabled, INT32 *WmtEnabled);

// c imports
extern void FilteringVert_12_C(UINT32 QValue,UINT8 * Src, INT32 Pitch);
extern void FilteringHoriz_12_C(UINT32 QValue, UINT8 * Src, INT32 Pitch );
extern void FilteringVert_8_C(UINT32 QValue, UINT8 * Src, INT32 Pitch );
extern void FilteringHoriz_8_C(UINT32 QValue, UINT8 * Src, INT32 Pitch );
extern void HorizontalLine_1_2_Scale_C( const unsigned char * source, unsigned int sourceWidth, unsigned char * dest, unsigned int destWidth );
extern void HorizontalLine_3_5_Scale_C( const unsigned char * source, unsigned int sourceWidth, unsigned char * dest, unsigned int destWidth );
extern void HorizontalLine_4_5_Scale_C( const unsigned char * source, unsigned int sourceWidth, unsigned char * dest, unsigned int destWidth );
extern void VerticalBand_4_5_Scale_C( unsigned char * dest, unsigned int destPitch, unsigned int destWidth );
extern void LastVerticalBand_4_5_Scale_C( unsigned char * dest, unsigned int destPitch, unsigned int destWidth );
extern void VerticalBand_3_5_Scale_C( unsigned char * dest, unsigned int destPitch, unsigned int destWidth );
extern void LastVerticalBand_3_5_Scale_C( unsigned char * dest, unsigned int destPitch, unsigned int destWidth );
extern void VerticalBand_1_2_Scale_C( unsigned char * dest, unsigned int destPitch, unsigned int destWidth );
extern void LastVerticalBand_1_2_Scale_C( unsigned char * dest, unsigned int destPitch, unsigned int destWidth );
extern void FilterHoriz_Simple_C( POSTPROC_INSTANCE *pbi, UINT8 * PixelPtr, INT32 LineLength, INT32 *BoundingValuePtr );
extern void FilterVert_Simple_C( POSTPROC_INSTANCE *pbi, UINT8 * PixelPtr, INT32 LineLength, INT32 *BoundingValuePtr );
extern void FilterHoriz_Generic( POSTPROC_INSTANCE *pbi, UINT8 * PixelPtr, INT32 LineLength, INT32 *BoundingValuePtr );
extern void FilterVert_Generic( POSTPROC_INSTANCE *pbi, UINT8 * PixelPtr, INT32 LineLength, INT32 *BoundingValuePtr );
extern INT32 *SetupBoundingValueArray_Generic( POSTPROC_INSTANCE *pbi, INT32 FLimit );
extern INT32 *SetupDeblockValueArray_Generic( POSTPROC_INSTANCE *pbi, INT32 FLimit );
extern void DeringBlockWeak_C( const POSTPROC_INSTANCE *pbi, const UINT8 *SrcPtr, UINT8 *DstPtr, const INT32 Pitch, UINT32 FragQIndex, UINT32 *QuantScale );
extern void DeringBlockStrong_C( const POSTPROC_INSTANCE *pbi, const UINT8 *SrcPtr, UINT8 *DstPtr, const INT32 Pitch, UINT32 FragQIndex, UINT32 *QuantScale);
extern void DeblockLoopFilteredBand_C( POSTPROC_INSTANCE *pbi, UINT8 *SrcPtr, UINT8 *DesPtr, UINT32 PlaneLineStep, UINT32 FragsAcross, UINT32 StartFrag, UINT32 *QuantScale );
extern void DeblockNonFilteredBand_C( POSTPROC_INSTANCE *pbi, UINT8 *SrcPtr, UINT8 *DesPtr, UINT32 PlaneLineStep, UINT32 FragsAcross, UINT32 StartFrag, UINT32 *QuantScale );
extern void DeblockNonFilteredBandNewFilter_C( POSTPROC_INSTANCE *pbi, UINT8 *SrcPtr, UINT8 *DesPtr, UINT32 PlaneLineStep, UINT32 FragsAcross, UINT32 StartFrag, UINT32 *QuantScale );
extern void PlaneAddNoise_C( UINT8 *Start, UINT32 Width, UINT32 Height, INT32 Pitch, int q);

// mmx imports
extern void FilteringVert_12_MMX(UINT32 QValue,UINT8 * Src, INT32 Pitch);
extern void FilteringHoriz_12_MMX(UINT32 QValue, UINT8 * Src, INT32 Pitch );
extern void FilteringVert_8_MMX(UINT32 QValue, UINT8 * Src, INT32 Pitch );
extern void FilteringHoriz_8_MMX(UINT32 QValue, UINT8 * Src, INT32 Pitch );
extern void HorizontalLine_1_2_Scale_MMX( const unsigned char * source, unsigned int sourceWidth, unsigned char * dest, unsigned int destWidth );
extern void HorizontalLine_3_5_Scale_MMX( const unsigned char * source, unsigned int sourceWidth, unsigned char * dest, unsigned int destWidth );
extern void HorizontalLine_4_5_Scale_MMX( const unsigned char * source, unsigned int sourceWidth, unsigned char * dest, unsigned int destWidth );
extern void VerticalBand_4_5_Scale_MMX( unsigned char * dest, unsigned int destPitch, unsigned int destWidth );
extern void LastVerticalBand_4_5_Scale_MMX( unsigned char * dest, unsigned int destPitch, unsigned int destWidth );
extern void VerticalBand_3_5_Scale_MMX( unsigned char * dest, unsigned int destPitch, unsigned int destWidth );
extern void LastVerticalBand_3_5_Scale_MMX( unsigned char * dest, unsigned int destPitch, unsigned int destWidth );
extern void VerticalBand_1_2_Scale_MMX( unsigned char * dest, unsigned int destPitch, unsigned int destWidth );
extern void LastVerticalBand_1_2_Scale_MMX( unsigned char * dest, unsigned int destPitch, unsigned int destWidth );
extern void FilterHoriz_Simple_MMX( POSTPROC_INSTANCE *pbi, UINT8 * PixelPtr, INT32 LineLength, INT32 *BoundingValuePtr );
extern void FilterVert_Simple_MMX( POSTPROC_INSTANCE *pbi, UINT8 * PixelPtr, INT32 LineLength, INT32 *BoundingValuePtr );
extern void FilterHoriz_MMX( POSTPROC_INSTANCE *pbi, UINT8 * PixelPtr, INT32 LineLength, INT32 *BoundingValuePtr );
extern void FilterVert_MMX( POSTPROC_INSTANCE *pbi, UINT8 * PixelPtr, INT32 LineLength, INT32 *BoundingValuePtr );
extern INT32 *SetupBoundingValueArray_ForMMX( POSTPROC_INSTANCE *pbi, INT32 FLimit );
extern INT32 *SetupDeblockValueArray_ForMMX( POSTPROC_INSTANCE *pbi, INT32 FLimit );
extern void DeringBlockWeak_MMX( const POSTPROC_INSTANCE *pbi, const UINT8 *SrcPtr, UINT8 *DstPtr, const INT32 Pitch, UINT32 FragQIndex, UINT32 *QuantScale );
extern void DeringBlockStrong_MMX( const POSTPROC_INSTANCE *pbi, const UINT8 *SrcPtr, UINT8 *DstPtr, const INT32 Pitch, UINT32 FragQIndex, UINT32 *QuantScale);
extern void DeblockLoopFilteredBand_MMX( POSTPROC_INSTANCE *pbi, UINT8 *SrcPtr, UINT8 *DesPtr, UINT32 PlaneLineStep, UINT32 FragsAcross, UINT32 StartFrag, UINT32 *QuantScale );
extern void DeblockNonFilteredBand_MMX( POSTPROC_INSTANCE *pbi, UINT8 *SrcPtr, UINT8 *DesPtr, UINT32 PlaneLineStep, UINT32 FragsAcross, UINT32 StartFrag, UINT32 *QuantScale );
extern void DeblockNonFilteredBandNewFilter_MMX( POSTPROC_INSTANCE *pbi, UINT8 *SrcPtr, UINT8 *DesPtr, UINT32 PlaneLineStep, UINT32 FragsAcross, UINT32 StartFrag, UINT32 *QuantScale );
extern void FillLoopFilterLimitValues_MMX(void);
extern INT16 *LoopFilterLimitValuesV2_MMX;             
extern void PlaneAddNoise_mmx( UINT8 *Start, UINT32 Width, UINT32 Height, INT32 Pitch, int q);


// wmt imports
extern void DeblockLoopFilteredBand_WMT( POSTPROC_INSTANCE *pbi, UINT8 *SrcPtr, UINT8 *DesPtr, UINT32 PlaneLineStep, UINT32 FragsAcross, UINT32 StartFrag, UINT32 *QuantScale );
extern void DeblockNonFilteredBand_WMT( POSTPROC_INSTANCE *pbi, UINT8 *SrcPtr, UINT8 *DesPtr, UINT32 PlaneLineStep, UINT32 FragsAcross, UINT32 StartFrag, UINT32 *QuantScale );
extern void DeringBlockWeak_WMT( const POSTPROC_INSTANCE *pbi, const UINT8 *SrcPtr, UINT8 *DstPtr, const INT32 Pitch, UINT32 FragQIndex, UINT32 *QuantScale );
extern void DeringBlockStrong_WMT( const POSTPROC_INSTANCE *pbi, const UINT8 *SrcPtr, UINT8 *DstPtr, const INT32 Pitch, UINT32 FragQIndex, UINT32 *QuantScale );
extern void CFastDeInterlace(UINT8 * SrcPtr,UINT8 * DstPtr,INT32 Width,INT32 Height,INT32 Stride);
extern void MmxFastDeInterlace(UINT8 * SrcPtr,UINT8 * DstPtr,INT32 Width,INT32 Height,INT32 Stride);
extern void WmtFastDeInterlace(UINT8 * SrcPtr,UINT8 * DstPtr,INT32 Width,INT32 Height,INT32 Stride);
extern void ClampLevels_C( POSTPROC_INSTANCE *pbi,INT32 BlackClamp,INT32 WhiteClamp,UINT8 *Src,UINT8 *Dst);
extern void ClampLevels_wmt( POSTPROC_INSTANCE *pbi,INT32 BlackClamp,INT32 WhiteClamp,UINT8 *Src,UINT8 *Dst);
extern void ClampLevels_mmx( POSTPROC_INSTANCE *pbi,INT32 BlackClamp,INT32 WhiteClamp,UINT8 *Src,UINT8 *Dst);
extern void PlaneAddNoise_wmt( UINT8 *Start, UINT32 Width, UINT32 Height, INT32 Pitch, int q);

/****************************************************************************
 * 
 *  ROUTINE       : PostProcMachineSpecificConfig
 *
 *  INPUTS        : UINT32 Version : Codec version number.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Checks for machine specifc features such as MMX support 
 *                  sets appropriate flags and function pointers.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void PostProcMachineSpecificConfig ( UINT32 Version )
{
	// If MMX supported then set to use MMX versions of functions else 
    // use original 'C' versions.
	INT32 MmxEnabled;
	INT32 XmmEnabled; 
	INT32 WmtEnabled;

	GetProcessorFlags( &MmxEnabled, &XmmEnabled, &WmtEnabled );
	
	if ( WmtEnabled )
	{
        // Willamette
		FillLoopFilterLimitValues_MMX();

        FilterHoriz                     = FilterHoriz_MMX;
        FilterVert                      = FilterVert_MMX;
        SetupBoundingValueArray         = SetupBoundingValueArray_ForMMX;
        SetupDeblockValueArray          = SetupDeblockValueArray_ForMMX;
        DeringBlockWeak                 = DeringBlockWeak_WMT;
        DeringBlockStrong               = DeringBlockStrong_WMT;
		DeblockLoopFilteredBand         = DeblockLoopFilteredBand_WMT;
		DeblockNonFilteredBand          = DeblockNonFilteredBand_WMT;
        DeblockNonFilteredBandNewFilter = DeblockNonFilteredBandNewFilter_MMX;
		FilterHoriz_Simple              = FilterHoriz_Simple_MMX;
		FilterVert_Simple               = FilterVert_Simple_MMX;
        HorizontalLine_1_2_Scale        = HorizontalLine_1_2_Scale_MMX;
        HorizontalLine_3_5_Scale        = HorizontalLine_3_5_Scale_MMX;
        HorizontalLine_4_5_Scale        = HorizontalLine_4_5_Scale_MMX;
        VerticalBand_1_2_Scale          = VerticalBand_1_2_Scale_MMX;
        LastVerticalBand_1_2_Scale      = LastVerticalBand_1_2_Scale_MMX;
        VerticalBand_3_5_Scale          = VerticalBand_3_5_Scale_MMX;
        LastVerticalBand_3_5_Scale      = LastVerticalBand_3_5_Scale_MMX;
        VerticalBand_4_5_Scale          = VerticalBand_4_5_Scale_MMX;
        LastVerticalBand_4_5_Scale      = LastVerticalBand_4_5_Scale_MMX;
        FilteringHoriz_8                = FilteringHoriz_8_MMX;
        FilteringVert_8                 = FilteringVert_8_MMX;
        FilteringHoriz_12               = FilteringHoriz_12_MMX;
        FilteringVert_12                = FilteringVert_12_MMX;
        FastDeInterlace                 = WmtFastDeInterlace;
        ClampLevels                     = ClampLevels_wmt; 
        PlaneAddNoise                   = PlaneAddNoise_wmt;
	}
	else if ( MmxEnabled )
    {
		FillLoopFilterLimitValues_MMX();

        FilterHoriz                     = FilterHoriz_MMX;
        FilterVert                      = FilterVert_MMX;
        SetupBoundingValueArray         = SetupBoundingValueArray_ForMMX;
        SetupDeblockValueArray          = SetupDeblockValueArray_ForMMX;
        DeringBlockWeak                 = DeringBlockWeak_MMX;
        DeringBlockStrong               = DeringBlockStrong_MMX;
		DeblockLoopFilteredBand         = DeblockLoopFilteredBand_MMX;
		DeblockNonFilteredBand          = DeblockNonFilteredBand_MMX;
		DeblockNonFilteredBandNewFilter = DeblockNonFilteredBandNewFilter_MMX;
		FilterHoriz_Simple              = FilterHoriz_Simple_MMX;
		FilterVert_Simple               = FilterVert_Simple_MMX;
        HorizontalLine_1_2_Scale        = HorizontalLine_1_2_Scale_MMX;
        HorizontalLine_3_5_Scale        = HorizontalLine_3_5_Scale_MMX;
        HorizontalLine_4_5_Scale        = HorizontalLine_4_5_Scale_MMX;
        VerticalBand_1_2_Scale          = VerticalBand_1_2_Scale_MMX;
        LastVerticalBand_1_2_Scale      = LastVerticalBand_1_2_Scale_MMX;
        VerticalBand_3_5_Scale          = VerticalBand_3_5_Scale_MMX;
        LastVerticalBand_3_5_Scale      = LastVerticalBand_3_5_Scale_MMX;
        VerticalBand_4_5_Scale          = VerticalBand_4_5_Scale_MMX;
        LastVerticalBand_4_5_Scale      = LastVerticalBand_4_5_Scale_MMX;
        FilteringHoriz_8                = FilteringHoriz_8_MMX;
        FilteringVert_8                 = FilteringVert_8_MMX;
        FilteringHoriz_12               = FilteringHoriz_12_MMX;
        FilteringVert_12                = FilteringVert_12_MMX;
        FastDeInterlace                 = MmxFastDeInterlace;
        ClampLevels                     = ClampLevels_mmx; 
        PlaneAddNoise                   = PlaneAddNoise_mmx;
    }
    else
    {

        FilterHoriz                     = FilterHoriz_Generic;
        FilterVert                      = FilterVert_Generic;
        SetupBoundingValueArray         = SetupBoundingValueArray_Generic;
        SetupDeblockValueArray          = SetupDeblockValueArray_Generic;
        DeringBlockWeak                 = DeringBlockWeak_C;
        DeringBlockStrong               = DeringBlockStrong_C;
		DeblockLoopFilteredBand         = DeblockLoopFilteredBand_C;
		DeblockNonFilteredBand          = DeblockNonFilteredBand_C;
		DeblockNonFilteredBandNewFilter = DeblockNonFilteredBandNewFilter_C;
		FilterHoriz_Simple              = FilterHoriz_Simple_C;
		FilterVert_Simple               = FilterVert_Simple_C;
        HorizontalLine_1_2_Scale        = HorizontalLine_1_2_Scale_C;        
        VerticalBand_1_2_Scale          = VerticalBand_1_2_Scale_C;
        LastVerticalBand_1_2_Scale      = LastVerticalBand_1_2_Scale_C;
        HorizontalLine_3_5_Scale        = HorizontalLine_3_5_Scale_C;
        VerticalBand_3_5_Scale          = VerticalBand_3_5_Scale_C;
        LastVerticalBand_3_5_Scale      = LastVerticalBand_3_5_Scale_C;
        HorizontalLine_4_5_Scale        = HorizontalLine_4_5_Scale_C;
        VerticalBand_4_5_Scale          = VerticalBand_4_5_Scale_C;
        LastVerticalBand_4_5_Scale      = LastVerticalBand_4_5_Scale_C;
        FilteringHoriz_8                = FilteringHoriz_8_C;
        FilteringVert_8                 = FilteringVert_8_C;
        FilteringHoriz_12               = FilteringHoriz_12_C;
        FilteringVert_12                = FilteringVert_12_C;
        FastDeInterlace                 = CFastDeInterlace;
        ClampLevels                     = ClampLevels_C; 
        PlaneAddNoise                   = PlaneAddNoise_C;
    }
}
