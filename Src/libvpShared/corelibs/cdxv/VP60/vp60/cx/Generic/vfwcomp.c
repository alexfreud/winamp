/****************************************************************************
*
*   Module Title :     vfwcomp.c
*
*   Description  :     Video for Windows Compressor interface definition.
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include <stdio.h>
#include <math.h>
#include "compdll.h" 
#include "misc_common.h"
#include "decodemode.h"

/****************************************************************************
*  Macros
****************************************************************************/
#define MAX_PSNR        60.0

/****************************************************************************
*  Module Statics
****************************************************************************/
static const UINT8 EndpointLookup[SCAN_ORDER_BANDS] =
    { 1, 4, 10, 12, 15, 19, 21, 26, 28, 34, 36, 42, 48, 53, 57, 63 };

static const UINT32 PriorKeyFrameWeight[KEY_FRAME_CONTEXT] = { 1, 2, 3, 4, 5 };

static UINT32 TotDropFrameCount = 0;

// % boost to data rate for GF update frames. 
// This extra spend is recovered from the next few frames
const UINT32 GfuDataRateBoost[64] = 
{
	1150, 1150, 1150, 1150, 1200, 1200, 1200, 1200,
	1250, 1250, 1250, 1250, 1350, 1350, 1350, 1350,
	1250, 1250, 1250, 1250, 1100, 1100, 1050, 1050,
	1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
	 950,  950,  950,  950,  950,  950,  950,  950,
	 900,  900,  900,  900,  900,  900,  850,  850,
	 800,  800,  750,  600,  500,  400,  350,  300,
	 250,  200,  150,  125,  100,   75,   50,    0
};

// Reduce GFU boost as motion lvl increases
const UINT32 GfuMotionCorrection[32] = 
{
   100, 95, 90, 85, 80, 75, 70, 65, 
	60, 55, 50, 45, 40, 35, 30, 25,
	20, 15, 10,  5,  5,  4,  4,  3, 
	 3,  2,  2,  1,  1,  0,  0,  0,
};

// Correction to boost value that depends on recent observed GF usage
// These are 1% steps. > 15% gets max boost.
// Boost is multipled by table value then divided by 128.
const UINT32 GfUsageCorrection2[16] = 
{    
	  8,  16,  32,  64,  80,  96, 112, 120, 
	128, 128, 128, 128, 128, 128, 128, 128
};

const UINT32 GfUsageCorrection[64] = 
{    
    12,12,12,12,12,12,12,12,
    12,12,12,12,12,13,14,15,
    16,17,18,19,20,21,22,23,
    24,25,26,27,28,29,30,31,
    32,33,34,35,36,37,38,39, 
    40,41,42,43,44,45,46,47,
    48,49,50,51,52,53,54,55,
    56,57,58,59,60,61,62,80
};

// Threshold and alpha limits for bicubi filtering
const UINT8 BicubicMaxAlpha[64] =
{
	 3, 3, 3, 3, 3, 3, 3, 3,
	 3, 3, 3, 3, 3, 3, 3, 3,
	 4, 4, 4, 4, 4, 4, 4, 4,
	 5, 5, 5, 5, 5, 5, 5, 5,
	 6, 6, 6, 6, 7, 7, 7, 7,
	 8, 8, 8, 8, 9, 9, 9, 9,
	10,10,10,10,10,10,10,10, 
	11,11,11,11,11,11,11,11, 
};
const UINT8 BicubicMinThresh[64] =
{
	 31,31,31,31,31,31,31,31,
	 16,16,16,16,16,16,16,16,
	 8, 8, 8, 8, 8, 8, 8, 8,
	 4, 4, 4, 4, 4, 4, 4, 4,
	 4, 4, 4, 4, 4, 4, 4, 4,
	 2, 2, 2, 2, 2, 2, 2, 2,
	 1, 1, 1, 1, 1, 1, 1, 1, 
	 1, 1, 1, 1, 1, 1, 1, 1 
};


/****************************************************************************
*  Imports
****************************************************************************/
extern UINT8 FixedQKfBoostTable[64];

#if defined PSNR_ON

/****************************************************************************
 *
 *  ROUTINE       : CalcPSNR
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : PSNR value for frame (in dB).
 *
 *  FUNCTION      : Calculate frame PSNR for diagnostic and tuning purposes.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
double CalcPSNR ( CP_INSTANCE *cpi )
{
    UINT32 i, j;
    INT32  Diff;
    UINT32 LineLength;
    UINT32 PlaneHeight;
    UINT32 FrameSize;
    double FramePsnr;
    double FrameYPsnr;
    double FrameUPsnr;
    double FrameVPsnr;
    UINT8 *RawDataPtr;
    UINT8 *ReconPtr;
    UINT8 *RawDataBuffer;
    UINT8 *ReconBuffer;
    INT32  Total = 0;
    INT32  GrandTotal = 0;

#if defined(_MSC_VER)
	ClearSysState();
#endif

    // choose the Raw data buffer to include or exclude the effect of pre-processing
    // cpi->yuv1ptr (or yuv0ptr to exclude the effect of pre-processing)
	if ( cpi->PreProcFilterLevel == 0 )
		RawDataBuffer = cpi->yuv1ptr;
	else
		RawDataBuffer = cpi->yuv0ptr;

    // Choose the reconstruction buffer according to whether or not post processing is on.
    if ( cpi->pb.PostProcessingLevel )
        ReconBuffer = cpi->pb.PostProcessBuffer;
    else
        ReconBuffer = cpi->pb.LastFrameRecon;

    // Set up for Y plane measurement
    LineLength  = cpi->pb.Configuration.VideoFrameWidth;
    PlaneHeight = cpi->pb.Configuration.VideoFrameHeight;
    RawDataPtr  = &RawDataBuffer[cpi->pb.YDataOffset];
    ReconPtr    = &ReconBuffer[cpi->pb.ReconYDataOffset+(UMV_BORDER*cpi->pb.Configuration.YStride)+UMV_BORDER];

    // Loop throught the Y plane raw and reconstruction data summing (square differences)
    for ( i=0; i<PlaneHeight; i++ )
    {
        for ( j=0; j<LineLength; j++ )
        {
            Diff        = (INT32)(RawDataPtr[j]) - (INT32)(ReconPtr[j]);
            Total      += Diff*Diff;
            GrandTotal += Diff*Diff;
        }
        RawDataPtr += LineLength;
        ReconPtr   += cpi->pb.Configuration.YStride;
    }

    // Work out Y PSNR
    FrameSize = cpi->pb.YPlaneSize;
    
    if ( (double)Total > 0.0 )
        FramePsnr = 10.0 * log10((255.0 * 255.0 * FrameSize) / (double)Total);
    else
        FramePsnr = MAX_PSNR;      // Limit to prevent / 0

    // Limit max reported frame PSNR to limit the effect of any one frame on the average.
    if ( FramePsnr > MAX_PSNR )
        FramePsnr = MAX_PSNR;

    cpi->TotYPsnr += FramePsnr;
    if ( FramePsnr < cpi->MinYPsnr )
        cpi->MinYPsnr = FramePsnr;
    if ( FramePsnr > cpi->MaxYPsnr )
        cpi->MaxYPsnr = FramePsnr;

    FrameYPsnr = FramePsnr;

    // Set up for U plane measurement
    LineLength  = cpi->pb.Configuration.VideoFrameWidth/2;
    PlaneHeight = cpi->pb.Configuration.VideoFrameHeight/2;
    RawDataPtr  = &RawDataBuffer[cpi->pb.UDataOffset];
    ReconPtr    = &ReconBuffer[cpi->pb.ReconUDataOffset+(UMV_BORDER>>1)*cpi->pb.Configuration.UVStride+(UMV_BORDER>>1)];

    // Loop throught the U plane raw and reconstruction data summing (square differences)
    Total = 0;
    for ( i=0; i<PlaneHeight; i++ )
    {
        for ( j=0; j<LineLength; j++ )
        {
            Diff        = (INT32)(RawDataPtr[j]) - (INT32)(ReconPtr[j]);
            Total      += Diff*Diff;
            GrandTotal += Diff*Diff;
        }
        RawDataPtr += LineLength;
        ReconPtr   += cpi->pb.Configuration.UVStride;
    }

    // Work out U PSNR
    FrameSize = cpi->pb.UVPlaneSize;
    
    if ( (double)Total > 0.0 )
        FramePsnr =  10.0 * log10((255.0 * 255.0 * FrameSize) / (double)Total);
    else
        FramePsnr =  MAX_PSNR;      // Limit to prevent / 0

    // Limit max reported frame PSNR to limit the effect of any one frame on the average.
    if ( FramePsnr > MAX_PSNR )
        FramePsnr = MAX_PSNR;

    cpi->TotUPsnr += FramePsnr;
    if ( FramePsnr < cpi->MinUPsnr )
        cpi->MinUPsnr = FramePsnr;
    if ( FramePsnr > cpi->MaxUPsnr )
        cpi->MaxUPsnr = FramePsnr;

    FrameUPsnr = FramePsnr;

    // Set up for V plane measurement
    LineLength  = cpi->pb.Configuration.VideoFrameWidth/2;
    PlaneHeight = cpi->pb.Configuration.VideoFrameHeight/2;
    RawDataPtr  = &RawDataBuffer[cpi->pb.VDataOffset];
    ReconPtr    = &ReconBuffer[cpi->pb.ReconVDataOffset+(UMV_BORDER>>1)*cpi->pb.Configuration.UVStride+(UMV_BORDER>>1)];

    // Loop throught the UV plane raw and reconstruction data summing (square differences)
    Total = 0;
    for ( i=0; i<PlaneHeight; i++ )
    {
        for ( j=0; j<LineLength; j++ )
        {
            Diff        = (INT32)(RawDataPtr[j]) - (INT32)(ReconPtr[j]);
            Total      += Diff*Diff;
            GrandTotal += Diff*Diff;
        }
        RawDataPtr += LineLength;
        ReconPtr   += cpi->pb.Configuration.UVStride;
    }

    // Work out V PSNR
    FrameSize = cpi->pb.UVPlaneSize;
    
    if ( (double)Total > 0.0 )
        FramePsnr = 10.0 * log10((255.0 * 255.0 * FrameSize) / (double)Total);
    else
        FramePsnr = MAX_PSNR;      // Limit to prevent / 0

    // Limit max reported frame PSNR to limit the effect of any one frame on the average.
    if ( FramePsnr > MAX_PSNR )
        FramePsnr = MAX_PSNR;

    cpi->TotVPsnr += FramePsnr;
    
    if ( FramePsnr < cpi->MinVPsnr )
        cpi->MinVPsnr = FramePsnr;
    if ( FramePsnr > cpi->MaxVPsnr )
        cpi->MaxVPsnr = FramePsnr;

    FrameVPsnr = FramePsnr;

    // Now work out the average accross YU and V
    FrameSize = cpi->pb.YPlaneSize + cpi->pb.UVPlaneSize + cpi->pb.UVPlaneSize;

    if ( (double)GrandTotal > 0.0 )
        FramePsnr = 10.0 * log10((255.0 * 255.0 * FrameSize) / (double)GrandTotal);
    else
        FramePsnr = MAX_PSNR;      // Limit to prevent / 0

    cpi->TotalSqError += GrandTotal;

    // Limit max reported frame PSNR to limit the effect of any one frame on the average.
    if ( FramePsnr > MAX_PSNR )
        FramePsnr = MAX_PSNR;

    cpi->TotPsnr += FramePsnr;
    
    if ( FramePsnr < cpi->MinPsnr )
        cpi->MinPsnr = FramePsnr;
    if ( FramePsnr > cpi->MaxPsnr )
        cpi->MaxPsnr = FramePsnr;

    return FramePsnr;
}
#endif
/****************************************************************************
 *
 *  ROUTINE       : SetupKeyFrame
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Sets frame type as Keyframe.
 *
 *  SPECIAL NOTES : Replace this function with cpi->pb.FrameType = BASE_FRAME;
 *
 ****************************************************************************/
void SetupKeyFrame ( CP_INSTANCE *cpi )
{
    VP6_SetFrameType ( &cpi->pb, BASE_FRAME );
}

/****************************************************************************
 *
 *  ROUTINE       : AdjustKeyFrameContext
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Adjusts the context for a keyframe.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void AdjustKeyFrameContext ( CP_INSTANCE *cpi )
{
    UINT32 i;
    INT32 AvKeyFramesPerSecond;
    INT32 MinFrameTargetRate;

    // Average key frame frequency and size
    UINT32  AvKeyFrameFrequency = (UINT32) (cpi->CurrentFrame / cpi->KeyFrameCount);
    UINT32  AvKeyFrameBytes     = (UINT32) (cpi->TotKeyFrameBytes / cpi->KeyFrameCount);
    UINT32 TotalWeight = 0;

    // Update the frame carry over
    cpi->TotKeyFrameBytes += (cpi->ThisFrameSize/8);

    // reset keyframe context and calculate weighted average of last KEY_FRAME_CONTEXT keyframes
    for ( i=0; i<KEY_FRAME_CONTEXT; i++ )
    {
        if ( i < KEY_FRAME_CONTEXT-1 )
        {
            cpi->PriorKeyFrameSize[i]     = cpi->PriorKeyFrameSize[i+1];
            cpi->PriorKeyFrameDistance[i] = cpi->PriorKeyFrameDistance[i+1];
        }
        else
        {
            cpi->PriorKeyFrameSize[KEY_FRAME_CONTEXT - 1]     = cpi->ThisFrameSize;
            cpi->PriorKeyFrameDistance[KEY_FRAME_CONTEXT - 1] = cpi->LastKeyFrame;
        }

        AvKeyFrameBytes += PriorKeyFrameWeight[i] * cpi->PriorKeyFrameSize[i] / 8;
        AvKeyFrameFrequency += PriorKeyFrameWeight[i] * cpi->PriorKeyFrameDistance[i];
        TotalWeight += PriorKeyFrameWeight[i];
    }
    AvKeyFrameBytes /= TotalWeight;
    AvKeyFrameFrequency /= TotalWeight;
    AvKeyFramesPerSecond =  100 * cpi->Configuration.OutputFrameRate / AvKeyFrameFrequency ;

    /* Calculate a new target rate per frame allowing for average key frame frequency over newest frames . */
    if ( (100 * cpi->Configuration.TargetBandwidth > AvKeyFrameBytes * AvKeyFramesPerSecond) &&
         (100 * cpi->Configuration.OutputFrameRate - AvKeyFramesPerSecond ))
    {
        cpi->InterFrameTarget =
            (INT32)(100* cpi->Configuration.TargetBandwidth - AvKeyFrameBytes * AvKeyFramesPerSecond )
            / ( (100 * cpi->Configuration.OutputFrameRate - AvKeyFramesPerSecond ) );

    }
    else // don't let this number get too small!!!
    {
        cpi->InterFrameTarget = 1;
    }

    // minimum allowable frame_target_rate
    MinFrameTargetRate = cpi->PerFrameBandwidth / 3;

    if ( cpi->InterFrameTarget < MinFrameTargetRate )
        cpi->InterFrameTarget = MinFrameTargetRate;


    cpi->LastKeyFrame = 1;
    cpi->LastKeyFrameSize = cpi->ThisFrameSize;
}

/****************************************************************************
 *
 *  ROUTINE       : ResizeFrameTo
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *                  int hscale       : Horizontal scale factor numerator.
 *                  int hratio       : Horizontal scale factor denominator.
 *                  int vscale       : Vertical scale factor numerator.
 *                  int vratioNone   : Vertical scale factor denominator.
 *   
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Changes the encoder frame size by the specified ratio.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void ResizeFrameTo ( CP_INSTANCE *cpi, int hscale, int hratio, int vscale, int vratio )
{
    int w  = cpi->YuvInputData.YWidth;
    int h  = cpi->YuvInputData.YHeight;
    int nw = w;
    int nh = h;

    cpi->pb.Configuration.HScale = hscale;
    cpi->pb.Configuration.HRatio = hratio;
    cpi->pb.Configuration.VScale = vscale;
    cpi->pb.Configuration.VRatio = vratio;

	nw = (cpi->pb.Configuration.HScale - 1 + w * cpi->pb.Configuration.HRatio) / cpi->pb.Configuration.HScale;
	nh = (cpi->pb.Configuration.VScale - 1 + h * cpi->pb.Configuration.VRatio) / cpi->pb.Configuration.VScale;
	nw = (nw + 15) / 16 * 16;
	nh = (nh + 15) / 16 * 16;

    cpi->InputConfig.YWidth   = nw;
    cpi->InputConfig.YHeight  = nh;
    cpi->InputConfig.UVWidth  = nw/2;
    cpi->InputConfig.UVHeight = nh/2;
    cpi->InputConfig.YStride  = nw;
    cpi->InputConfig.UVStride = nw/2;

    ChangeEncoderSize ( cpi, nw, nh );

    CopyOrResize ( cpi, TRUE );

    cpi->KeyFrameDataTarget = (int)cpi->KeyFrameDataTargetOrig * (nw + nh) / (w + h);

	if ( cpi->KeyFrameDataTarget > (int)cpi->Configuration.TargetBandwidth/2 )
		cpi->KeyFrameDataTarget = (int)cpi->Configuration.TargetBandwidth/2;
}

/****************************************************************************
 *
 *  ROUTINE       : ResizeFrame
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Resizes a frame as necessary.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void ResizeFrame ( CP_INSTANCE *cpi )
{
	int HScale = 1;
    int HRatio = 1;
    int VScale = 1;
    int VRatio = 1;

	if ( cpi->ForceInternalSize )
	{
		ResizeFrameTo ( cpi, 
                        cpi->ForceHScale, 
                        cpi->ForceHRatio, 
                        cpi->ForceVScale, 
                        cpi->ForceVRatio );
		return;
	}

	if ( cpi->pb.Configuration.Interlaced )
	{
		switch ( cpi->SizeStep )
		{
		case 1:
			HScale = 5;
			HRatio = 4;
			break;
		case 2:
			HScale = 5;
			HRatio = 3;
			break;
		case 3:
			HScale = 2;
			HRatio = 1;
			break;
		case 4:
			HScale = 5;
			HRatio = 3;
			VScale = 2;
			VRatio = 1;
			break;
		case 5:
			HScale = 2;
			HRatio = 1;
			VScale = 2;
			VRatio = 1;
			break;
		}
	}
	else
	{
		switch ( cpi->SizeStep )
		{
		case 1:
			HScale = 5;
			HRatio = 4;
			break;
		case 2:
			HScale = 5;
			HRatio = 4;
			VScale = 5;
			VRatio = 4;
			break;
		case 3:
			HScale = 5;
			HRatio = 3;
			VScale = 5;
			VRatio = 4;
			break;
		case 4:
			HScale = 5;
			HRatio = 3;
			VScale = 5;
			VRatio = 3;
			break;
		case 5:
			HScale = 2;
			HRatio = 1;
			VScale = 2;
			VRatio = 1;
			break;
		}
	}

	ResizeFrameTo ( cpi, HScale, HRatio, VScale, VRatio );


    return;
}

/****************************************************************************
 *
 *  ROUTINE       : CompressFirstFrame
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Compresses the first frame.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void CompressFirstFrame ( CP_INSTANCE *cpi )
{
    UINT32  i;

	cpi->ErrorPerBit = 80;

	// MV and mode counters used in assessing new MV frequency.
	cpi->FrameNewMvCounter = 0;
	cpi->FrameModeCounter  = 0;

    // if not AutoKeyframing cpi->ForceKeyFrameEvery = is frequency
    if ( !cpi->AutoKeyFrameEnabled )
        cpi->ForceKeyFrameEvery = cpi->KeyFrameFrequency;

    /* set up context of key frame sizes and distances for more local datarate control */
    for ( i=0; i<KEY_FRAME_CONTEXT; i++ )
    {
        cpi->PriorKeyFrameSize[i]     = cpi->KeyFrameDataTarget;
        cpi->PriorKeyFrameDistance[i] = cpi->ForceKeyFrameEvery;
    }

    // Keep track of the total number of Key Frames Coded.
    cpi->KeyFrameCount    = 1;
    cpi->LastKeyFrame     = 1;
    cpi->TotKeyFrameBytes = 0;

    if ( cpi->AllowSpatialResampling && cpi->SizeStep != 0 || cpi->ForceInternalSize )
        ResizeFrame ( cpi );
	else
	    CopyOrResize ( cpi, TRUE );

    // Use scan order updates for larger images.
	if ( cpi->pb.Configuration.VideoFrameWidth >= 480 )
		cpi->AllowScanOrderUpdates = TRUE;
	else
 		cpi->AllowScanOrderUpdates = FALSE;

    SetupKeyFrame ( cpi );

    // Calculate a new target rate per frame allowing for average key frame frequency and size thus far.
    if ( cpi->Configuration.TargetBandwidth > ((cpi->KeyFrameDataTarget * cpi->Configuration.OutputFrameRate)/cpi->KeyFrameFrequency) )
    {
        cpi->InterFrameTarget =  (INT32)((cpi->Configuration.TargetBandwidth -
            ((cpi->KeyFrameDataTarget * cpi->Configuration.OutputFrameRate)/cpi->KeyFrameFrequency)) / cpi->Configuration.OutputFrameRate);

    }
    else
        cpi->InterFrameTarget = 1;

    // Reset the drop frame flags
    cpi->DropCount = 0;
	cpi->MaxDropCount = 0;


    // Select Intra mode for all MBs and calculate the total error score
    cpi->IntraError = PickIntra ( cpi );
    cpi->InterError = cpi->IntraError;

#if defined(_MSC_VER)
	ClearSysState();
#endif

    if( 0) //cpi->pass == 2) 
    {
        {
            int Q,R;

            cpi->ThisFrameTarget = cpi->InterFrameTarget;
            RegulateQ ( cpi, cpi->ThisFrameTarget);
            Q = cpi->pb.quantizer->FrameQIndex;
            cpi->ThisFrameTarget = cpi->InterFrameTarget + ((cpi->InterFrameTarget * cpi->KFBoost) >> 4) ;
            RegulateQ ( cpi, cpi->ThisFrameTarget);

            R= cpi->pb.quantizer->FrameQIndex  - Q;
            if(R>FixedQKfBoostTable[Q])
                R=FixedQKfBoostTable[Q];
            //S= (FixedQKfBoostTable[Q] + R) / 2;
            ClampAndUpdateQ ( cpi, Q+R);

        }
        //ClampAndUpdateQ ( cpi, cpi->pb.AvgFrameQIndex + FixedQKfBoostTable[cpi->pb.AvgFrameQIndex]);
    }
	else
	{

	     // Set a target size for this key frame based upon the baseline target and frequency
		cpi->ThisFrameTarget = cpi->KeyFrameDataTarget;
        RegulateQ ( cpi, cpi->ThisFrameTarget);
	}

    /* Compress and output the frist frame */
    UpdateFrame ( cpi );
}

/****************************************************************************
 *
 *  ROUTINE       : CompressKeyFrame
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Compresses a Keyframe.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void CompressKeyFrame ( CP_INSTANCE *cpi )
{

	// Reset the active worst quality to the baseline value for key frames.
	cpi->Configuration.ActiveWorstQuality = cpi->Configuration.WorstQuality;

	// Auto-spatial re-sampling only allowed for buffered mode.
	if ( cpi->BufferedMode && cpi->pass != 2)
	{


		// Decide whether we need to down sample or if we are able to return to a larger image size
		// We downsample if the buffer fullness is below a given level and falling since the last key frame.
		if( (cpi->BufferLevel < cpi->ResampleDownWaterMark) &&
			(cpi->BufferLevel <= cpi->LastKeyFrameBufferLevel) )
		{
			if ( cpi->SizeStep < 5 )
				cpi->SizeStep++;
		}
		// We upsample (or undo down sample) if the buffer fullness is above a given level
		// and is not falling or it is significantly above the optimal level.
		else if ( (cpi->BufferLevel > ((cpi->OptimalBufferLevel * 110)/100) ) ||
				  ((cpi->BufferLevel > cpi->ResampleUpWaterMark) &&
				   (cpi->BufferLevel >= cpi->LastKeyFrameBufferLevel) ) )
		{
			if ( cpi->SizeStep > 0 )
				cpi->SizeStep--;
		}
	}

	// Implement any resize that has been chosen
    if ( cpi->AllowSpatialResampling && cpi->SizeStep != cpi->LastSizeStep || cpi->ForceInternalSize )
        ResizeFrame ( cpi );
	else
	    CopyOrResize ( cpi, TRUE );

    // Use scan order updates for larger images.
	if ( cpi->pb.Configuration.VideoFrameWidth >= 480 )
		cpi->AllowScanOrderUpdates = TRUE;
	else
 		cpi->AllowScanOrderUpdates = FALSE;

    // Keep track of the total number of Key Frames Coded
    cpi->KeyFrameCount += 1;

    // Reset the drop frame flags
    cpi->DropCount = 0;
	cpi->MaxDropCount = 0;

    SetupKeyFrame ( cpi );

	// Set the key frame size constraints
	cpi->ThisFrameTarget = cpi->KeyFrameDataTarget;

    // Select Intra mode for all MBs and calculate the total error score
    cpi->IntraError = PickIntra ( cpi );
    cpi->InterError = cpi->IntraError;

#if defined(_MSC_VER)
	ClearSysState();
#endif

	// Reset the KeyFrameBpbCorrectionFactor to 1.0
    cpi->KeyFrameBpbCorrectionFactor = 1;

	// Set an appropriate Key frame Q to match the recent ambient quality
    if( cpi->pass == 2) 
    {
	    if ( cpi->KFForced == TRUE)
		    ClampAndUpdateQ ( cpi, cpi->pb.AvgFrameQIndex + (FixedQKfBoostTable[cpi->pb.AvgFrameQIndex]/2) );
        else
        {
            int Q,R;

            cpi->ThisFrameTarget = cpi->InterFrameTarget;
            RegulateQ ( cpi, cpi->ThisFrameTarget);
            Q = cpi->pb.quantizer->FrameQIndex;
            cpi->ThisFrameTarget = cpi->InterFrameTarget + ((cpi->InterFrameTarget * cpi->KFBoost) >> 4) ;
            RegulateQ ( cpi, cpi->ThisFrameTarget);

            R= cpi->pb.quantizer->FrameQIndex  - Q;
            if(R>FixedQKfBoostTable[Q])
                R=FixedQKfBoostTable[Q];
            //S= (FixedQKfBoostTable[Q] + R) / 2;
            ClampAndUpdateQ ( cpi, Q+R);

        }
        //ClampAndUpdateQ ( cpi, cpi->pb.AvgFrameQIndex + FixedQKfBoostTable[cpi->pb.AvgFrameQIndex]);
    }
    else
    {
	    if ( cpi->KFForced == TRUE)
		    ClampAndUpdateQ ( cpi, cpi->pb.AvgFrameQIndex + (FixedQKfBoostTable[cpi->pb.AvgFrameQIndex]/2) );
	    else
		    ClampAndUpdateQ ( cpi, cpi->pb.AvgFrameQIndex + FixedQKfBoostTable[cpi->pb.AvgFrameQIndex] );
    }

    /* Compress and output the first frame */
    UpdateFrame ( cpi );
    cpi->LastSizeStep = cpi->SizeStep;

}

/****************************************************************************
 *
 *  ROUTINE       : CompressFrame
 *
 *  INPUTS        : CP_INSTANCE *cpi   : Pointer to encoder instance.
 *                  UINT32 FrameNumber : Frame number (NOT USED).
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Compresses a frame.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void CompressFrame ( CP_INSTANCE *cpi, UINT32 FrameNumber )
{
    UINT32  i;
    UINT32  KFIndicator;
	BOOL 	DropedFrame = FALSE;
    int     actualMBS;

    cpi->KFForced=0;
    CopyOrResize ( cpi, FALSE );

	/* Clear down the macro block level mode and MV arrays. */
    for ( i=0; i<cpi->pb.UnitFragments; i++ )
    {
        cpi->pb.FragInfo[i].FragCodingMode = CODE_INTER_NO_MV;     // Default coding mode
        cpi->pb.FragInfo[i].MVectorX       = 0;
        cpi->pb.FragInfo[i].MVectorY       = 0;
    }

    /* Default to normal frames. */ 
    VP6_SetFrameType ( &cpi->pb, NORMAL_FRAME );

    // Calculate the target bytes for this frame. */
    cpi->ThisFrameTarget = cpi->InterFrameTarget;
 
    /* */
/*
    cpi->pb.mbi.blockDxInfo[0].dequantPtr = cpi->pb.quantizer->dequant_coeffs[VP6_QTableSelect[0]];
    cpi->pb.mbi.blockDxInfo[1].dequantPtr = cpi->pb.quantizer->dequant_coeffs[VP6_QTableSelect[1]];
    cpi->pb.mbi.blockDxInfo[2].dequantPtr = cpi->pb.quantizer->dequant_coeffs[VP6_QTableSelect[2]];
    cpi->pb.mbi.blockDxInfo[3].dequantPtr = cpi->pb.quantizer->dequant_coeffs[VP6_QTableSelect[3]];
    cpi->pb.mbi.blockDxInfo[4].dequantPtr = cpi->pb.quantizer->dequant_coeffs[VP6_QTableSelect[4]];
    cpi->pb.mbi.blockDxInfo[5].dequantPtr = cpi->pb.quantizer->dequant_coeffs[VP6_QTableSelect[5]];

	cpi->pb.mbi.blockDxInfo[0].MvShift =
    cpi->pb.mbi.blockDxInfo[1].MvShift =
    cpi->pb.mbi.blockDxInfo[2].MvShift =
    cpi->pb.mbi.blockDxInfo[3].MvShift = Y_MVSHIFT;
	cpi->pb.mbi.blockDxInfo[4].MvShift =
	cpi->pb.mbi.blockDxInfo[5].MvShift = UV_MVSHIFT;

	cpi->pb.mbi.blockDxInfo[0].MvModMask =
    cpi->pb.mbi.blockDxInfo[1].MvModMask =
    cpi->pb.mbi.blockDxInfo[2].MvModMask =
    cpi->pb.mbi.blockDxInfo[3].MvModMask = Y_MVMODMASK;
	cpi->pb.mbi.blockDxInfo[4].MvModMask =
	cpi->pb.mbi.blockDxInfo[5].MvModMask = UV_MVMODMASK;

	cpi->pb.mbi.blockDxInfo[0].Plane =
    cpi->pb.mbi.blockDxInfo[1].Plane =
    cpi->pb.mbi.blockDxInfo[2].Plane =
    cpi->pb.mbi.blockDxInfo[3].Plane = 0;
	cpi->pb.mbi.blockDxInfo[4].Plane =
	cpi->pb.mbi.blockDxInfo[5].Plane = 1;

    cpi->pb.mbi.blockDxInfo[0].LastDc = 
    cpi->pb.mbi.blockDxInfo[1].LastDc = 
    cpi->pb.mbi.blockDxInfo[2].LastDc = 
    cpi->pb.mbi.blockDxInfo[3].LastDc = cpi->pb.fc.LastDcY;
    cpi->pb.mbi.blockDxInfo[4].LastDc = cpi->pb.fc.LastDcU;
    cpi->pb.mbi.blockDxInfo[5].LastDc = cpi->pb.fc.LastDcV;

    cpi->pb.mbi.blockDxInfo[0].Left = &cpi->pb.fc.LeftY[0];
    cpi->pb.mbi.blockDxInfo[1].Left = &cpi->pb.fc.LeftY[0];
    cpi->pb.mbi.blockDxInfo[2].Left = &cpi->pb.fc.LeftY[1];
    cpi->pb.mbi.blockDxInfo[3].Left = &cpi->pb.fc.LeftY[1];
    cpi->pb.mbi.blockDxInfo[4].Left = &cpi->pb.fc.LeftU;
    cpi->pb.mbi.blockDxInfo[5].Left = &cpi->pb.fc.LeftV;
*/
 
	// For Buffered mode make data rate and Q range adjustments based on buffer fullness.
	if ( cpi->BufferedMode )
	{
		INT32 OnePercentBits = 1 + cpi->OptimalBufferLevel/100;

		//if ( cpi->BufferLevel < cpi->OptimalBufferLevel || cpi->BytesOffTarget < 0 )
		if ( ( cpi->BufferLevel < cpi->OptimalBufferLevel ) || 
			 ( cpi->BytesOffTarget < cpi->OptimalBufferLevel ) )
		{
			INT32 PercentLow = 0;

			// Decide whether or not we need to adjust the frame data rate target.
			//
			// If we are are below the optimal buffer fullness level and adherence  
			// to buffering contraints is important to the end useage then adjust
			// the per frame target.
			if ( (cpi->EndUsage == USAGE_STREAM_FROM_SERVER) && ( cpi->BufferLevel < cpi->OptimalBufferLevel ) )
            {
				PercentLow = (cpi->OptimalBufferLevel - cpi->BufferLevel) / OnePercentBits;
				if ( PercentLow > 100 )
					 PercentLow = 100;		
				else if ( PercentLow < 0 )
					 PercentLow = 0;
            }
			// Are we overshooting the long term clip data rate...
			else if ( cpi->BytesOffTarget < 0 )
			{
				// Adjust per frame data target downwards to compensate.
			    PercentLow = (INT32) (100 * -cpi->BytesOffTarget / (cpi->TotalByteCount * 8));
			    if ( PercentLow > 100 )
				     PercentLow = 100;		
			    else if ( PercentLow < 0 )
				     PercentLow = 0;
			}

			// lower the target bandwidth for this frame.
			cpi->ThisFrameTarget = (cpi->ThisFrameTarget * (100 - (PercentLow/2)) )/100;

			// Set a reduced data rate target for our initial Q calculation. 
			// This should provide a slight upward pressure on  buffer fullness 
			// during easier sections. 
			if ( (cpi->UnderShootPct > 0) && (cpi->UnderShootPct <= 100) ) 
			{
				cpi->ThisFrameTarget = (cpi->ThisFrameTarget * cpi->UnderShootPct)/100;
			}

			// Are we using allowing control of ActiveWorstQuality according to buffer level.
			if ( cpi->AutoWorstQ )
			{
				INT32 CriticalBufferLevel;

				// For streaming applications the most important factor is cpi->BufferLevel as this takes
				// into account the specified short term buffering constraints. However, hitting the long 
				// term clip data rate target is also important.
				if ( cpi->EndUsage == USAGE_STREAM_FROM_SERVER )
				{
					// Take the smaller of cpi->BufferLevel and cpi->BytesOffTarget
					CriticalBufferLevel = (cpi->BufferLevel < cpi->BytesOffTarget) ? cpi->BufferLevel : cpi->BytesOffTarget;
				}
				// For local file playback short term buffering contraints are less of an issue
				else
				{
					// Consider only how we are doing for the clip as a whole
					CriticalBufferLevel = cpi->BytesOffTarget;
				}

				// Set the active worst quality based upon the selected buffer fullness number.
				if ( CriticalBufferLevel < cpi->OptimalBufferLevel ) 
				{
					if ( CriticalBufferLevel > (cpi->OptimalBufferLevel/4) )
					{
						UINT32 QAdjustmentRange = cpi->NiAvQi - cpi->Configuration.WorstQuality;
						UINT32 AboveBase = (CriticalBufferLevel - (cpi->OptimalBufferLevel/4));

						// Step active worst quality down from cpi->NiAvQi when (CriticalBufferLevel == cpi->OptimalBufferLevel)
						// to cpi->Configuration.WorstQuality when (CriticalBufferLevel == cpi->OptimalBufferLevel/4) 
						cpi->Configuration.ActiveWorstQuality = cpi->Configuration.WorstQuality + 
																( (QAdjustmentRange * AboveBase) / (cpi->OptimalBufferLevel*3/4) );

						//cpi->Configuration.ActiveWorstQuality = (cpi->NiAvQi * CriticalBufferLevel) / cpi->OptimalBufferLevel;
						if ( cpi->Configuration.ActiveWorstQuality < cpi->Configuration.WorstQuality )
							cpi->Configuration.ActiveWorstQuality = cpi->Configuration.WorstQuality;
					}
					else 
					{
						cpi->Configuration.ActiveWorstQuality = cpi->Configuration.WorstQuality;
					}
				}
				else
				{
					cpi->Configuration.ActiveWorstQuality = cpi->NiAvQi;
				}

/*				// Problems with this for local file mode because
				// cpi->NiAvQi set to lower of average and last frame so as soon as cpi->BytesOffTarget
				// goes negative we tend to race down to worst quality so this does not behave as one might expect.
				else
				{
					if ( cpi->BytesOffTarget < 0 )
					{
						INT32 PercentOvershoot;
						
						// Work out the overshoot as a percentage of the total file size
						// Base cpi->Configuration.ActiveWorstQuality on this amount.
						PercentOvershoot = (100 * -cpi->BytesOffTarget / (cpi->TotalByteCount * 8));

						if ( PercentOvershoot > (cpi->NiAvQi - cpi->Configuration.WorstQuality) )
							cpi->Configuration.ActiveWorstQuality = cpi->Configuration.WorstQuality;
						else
							cpi->Configuration.ActiveWorstQuality = cpi->NiAvQi - PercentOvershoot;
					}
					else
						cpi->Configuration.ActiveWorstQuality = cpi->NiAvQi;
				}
*/
			}
			else
			{						
				cpi->Configuration.ActiveWorstQuality = cpi->Configuration.WorstQuality;
			}
		}
		else 
		{
			INT32 PercentHigh;

            if(cpi->BytesOffTarget > cpi->OptimalBufferLevel)
            {
			    PercentHigh = (INT32) (100 * (cpi->BytesOffTarget - cpi->OptimalBufferLevel) / (cpi->TotalByteCount * 8));
			    if ( PercentHigh > 100 )
				     PercentHigh = 100;		
			    else if ( PercentHigh < 0 )
				     PercentHigh = 0;
                cpi->ThisFrameTarget = (cpi->ThisFrameTarget * (100 + (PercentHigh/2)) )/100;
            }

			// Are we using allowing control of ActiveWorstQuality according to bufferl level.
			if ( cpi->AutoWorstQ )
			{
				// When using the relaxed buffer model stick to the user specified value
				cpi->Configuration.ActiveWorstQuality = cpi->NiAvQi;
			}
			else
			{
				cpi->Configuration.ActiveWorstQuality = cpi->Configuration.WorstQuality;
			}

		}

		// Set ActiveBestQuality to prevent quality rising too high
   		cpi->Configuration.ActiveBestQuality = Q_TABLE_SIZE - cpi->BestAllowedQ;

        // Worst quality obviously must not be better than best quality
		if ( cpi->Configuration.ActiveWorstQuality > cpi->Configuration.ActiveBestQuality )
			cpi->Configuration.ActiveWorstQuality = cpi->Configuration.ActiveBestQuality - 1;
		
	}
	// Unbuffered mode (eg. video conferencing)
	else
	{
		// Set the active worst quality
		cpi->Configuration.ActiveWorstQuality = cpi->Configuration.WorstQuality;
	}

	// The auto-drop frame code is only used in buffered mode.
	// In unbufferd mode (eg vide conferencing) the descision to
	// code or drop a frame is made outside the codec in response to real
	// world comms or buffer considerations.
	if ( cpi->DropFramesAllowed && cpi->BufferedMode ) 
	{
		// Check for a buffer underrun-crisis in which case we have to drop a frame
		if ( cpi->BufferLevel < cpi->PerFrameBandwidth )
			cpi->DropFrame = TRUE;
		// Check for drop frame crtieria
		else if ( cpi->BufferLevel < cpi->DropFramesWaterMark )
		{
			if ( cpi->DropCount < cpi->MaxDropCount )
				cpi->DropFrame = TRUE;
		}
	}

    if ( !cpi->DropFrame )
    {
        // pick all the macroblock modes and motion vectors
        UINT32 InterError;
        UINT32 IntraError;

        /*********************** Q PREDICTION STAGE 1  *****************************/

        /* Select modes and motion vectors for each of the blocks : return an error score for inter and intra */

        // Test for auto key frame.
        if ( cpi->AutoKeyFrameEnabled && (cpi->LastKeyFrame >= cpi->ForceKeyFrameEvery) ) 
		{
            cpi->KFForced=1;
			CompressKeyFrame(cpi);  // Code a key frame
			return;
		}
        

#if defined(_MSC_VER)
	    ClearSysState();
#endif

		// Update data rate to allow for GF updates.
		// Note that we come in here even for fixed. In order to set the next update interval.
		// Also not that we do not make a correction for the frames between a kf and the first GF update after a KF.
		if ( (!cpi->DisableGolden) && cpi->BufferedMode && (cpi->pb.quantizer->FrameQIndex < 60) && (cpi->LastKeyFrame >= cpi->GfUpdateInterval))
		{

			UINT32 MaxVariance = 0;
			UINT32 Sum2 = 0;
			UINT32 Sum3 = 0;
            int    Sum =                       // number of macroblocks
                  (cpi->pb.MBRows - (BORDER_MBS*2)) 
                * (cpi->pb.MBCols - (BORDER_MBS*2));

			if ( Sum )
			{
				Sum2 = Sum - (cpi->ModeDist[CODE_INTRA] + cpi->ModeDist[CODE_INTER_PLUS_MV] + cpi->ModeDist[CODE_INTER_FOURMV]);
				Sum3 = Sum2 - cpi->ModeDist[CODE_INTER_NO_MV] - cpi->ModeDist[CODE_USING_GOLDEN];			

				// Convert Sum2 and Sum3 to %
				Sum2 = (Sum2 * 100 / Sum);			
				Sum3 = (Sum3 * 100 / Sum);		

                cpi->fps.PercentMotion = Sum2;
                cpi->fps.PercentNewMotion = Sum3;
			}
				    
			// Calculate various motion metrics
			if ( cpi->FrameMvStats.NumMvs )
			{
				cpi->GfuMotionSpeed = (cpi->FrameMvStats.SumAbsX > cpi->FrameMvStats.SumAbsY) ? (cpi->FrameMvStats.SumAbsX/cpi->FrameMvStats.NumMvs) : (cpi->FrameMvStats.SumAbsY/cpi->FrameMvStats.NumMvs);
                cpi->fps.MotionSpeed = cpi->GfuMotionSpeed;
				cpi->fps.VarianceX = ((cpi->FrameMvStats.NumMvs * cpi->FrameMvStats.SumXSq) - (cpi->FrameMvStats.SumX*cpi->FrameMvStats.SumX)) / (cpi->FrameMvStats.NumMvs * cpi->FrameMvStats.NumMvs);
				cpi->fps.VarianceY = ((cpi->FrameMvStats.NumMvs * cpi->FrameMvStats.SumYSq) - (cpi->FrameMvStats.SumY*cpi->FrameMvStats.SumY)) / (cpi->FrameMvStats.NumMvs * cpi->FrameMvStats.NumMvs);
				MaxVariance = (UINT32) ((cpi->fps.VarianceX > cpi->fps.VarianceY) ? cpi->fps.VarianceX : cpi->fps.VarianceY);
				cpi->GfuMotionComplexity = (UINT32) (cpi->GfuMotionSpeed + ((cpi->fps.VarianceX)/4) + ((cpi->fps.VarianceY)/4));
				if ( cpi->GfuMotionComplexity > 31 )
					cpi->GfuMotionComplexity = 31;
			}	
			else
			{
				cpi->GfuMotionSpeed = 0; 
				cpi->GfuMotionComplexity = 0;
			}


            if( cpi->pass == 2 )
            {
                if(cpi->pb.RefreshGoldenFrame == TRUE) 
                {
    				cpi->ThisFrameTarget = (cpi->ThisFrameTarget * (100 * cpi->GfUpdateInterval)) /
	    								   ((100 * cpi->GfUpdateInterval) + cpi->GfuBoost);
    
	    			cpi->ThisFrameTarget = cpi->ThisFrameTarget + ((cpi->ThisFrameTarget * cpi->GfuBoost) / 100);	
                }
                else if ( cpi->FramesTillGfUpdateDue > 0 )
                {
				    // Non GFU frames reduced in bandwidth to account for 
				    // + GfuBoost % on GFU frames
				    cpi->ThisFrameTarget = (cpi->ThisFrameTarget * (100 * cpi->GfUpdateInterval)) /
						                   ((100 * cpi->GfUpdateInterval) + cpi->GfuBoost);
                }
            }
            else
            {
			    if ( cpi->FramesTillGfUpdateDue > 0 )
			    {
				    // Non GFU frames reduced in bandwidth to account for 
				    // + GfuBoost % on GFU frames
				    cpi->ThisFrameTarget = (cpi->ThisFrameTarget * (100 * cpi->GfUpdateInterval)) /
						                   ((100 * cpi->GfUpdateInterval) + cpi->GfuBoost);
			    }
                else if (cpi->DisableGolden == 0) 
                {
			        int IntraToInterRatio;
#define NEWWAY        
#ifdef NEWWAY
					IntraToInterRatio = 100 * cpi->IntraError / (cpi->InterError );
					IntraToInterRatio = IntraToInterRatio * Sum3 / 100;

					cpi->GfuBoost = IntraToInterRatio;

					// Correct boost to take account of recent observed level of GF usage
					if ( (cpi->GfUsage >> 3) < 64)
						cpi->GfuBoost = (cpi->GfuBoost * GfUsageCorrection[(cpi->GfUsage  >> 3)]) / 16;
					else
						cpi->GfuBoost = (cpi->GfuBoost * GfUsageCorrection[63]) / 16;

					cpi->GfuBoost = cpi->GfuBoost* GfuDataRateBoost[cpi->pb.AvgFrameQIndex] / 1000;


					// Should we even consider a GF update or is there no point
					if ( ( Sum3 > GF_MODE_DIST_THRESH2) &&
						 ( cpi->GfuMotionSpeed <= MAX_GF_UPDATE_MOTION)  
					   ) 
				    {


#else

				    // Calculate the %extra for GFU frames
				    cpi->GfuBoost = (GfuDataRateBoost[cpi->pb.quantizer->FrameQIndex] * GfuMotionCorrection[cpi->GfuMotionComplexity]) / 100;

				    // Correct boost to take account of recent observed level of GF usage
				    if ( (cpi->GfUsage >> 3) <= 15 )
					    cpi->GfuBoost = (cpi->GfuBoost * GfUsageCorrection2[(cpi->GfUsage >> 3)]) / 128;
				    else
					    cpi->GfuBoost = (cpi->GfuBoost * GfUsageCorrection2[15]) / 128;

                    // Should we even consider a GF update or is there no point
				    if ( (Sum2 > GF_MODE_DIST_THRESH1) && (Sum3 > GF_MODE_DIST_THRESH2) &&
					     (cpi->GfuMotionSpeed <= MAX_GF_UPDATE_MOTION) && 
						 (cpi->GfuBoost >= 80)  &&
					     (MaxVariance <= GF_MAX_VAR_THRESH) ) 
					{


#endif 

						cpi->ThisFrameTarget = (cpi->ThisFrameTarget * (100 * cpi->GfUpdateInterval)) /
										       ((100 * cpi->GfUpdateInterval) + cpi->GfuBoost);

					    cpi->ThisFrameTarget = cpi->ThisFrameTarget + ((cpi->ThisFrameTarget * cpi->GfuBoost) / 100);	

					    cpi->pb.RefreshGoldenFrame = TRUE;

						if(0)
						{
							FILE *gfstats= fopen("gf.stt","a");
							fprintf(gfstats,"Frame : %8d boost:%d, sp:%d,base:%d,ratio:%d,motion:%d,Gf:%d \n",
								- 1 + (INT32) cpi->CurrentFrame , 
								cpi->GfuBoost,
								cpi->GfuMotionSpeed,
								GfuDataRateBoost[cpi->pb.AvgFrameQIndex],
								100 * cpi->IntraError / (cpi->InterError),
								Sum3,
								cpi->GfUsage
								);
							fclose(gfstats);
						}



					    // Select the interval before the next GF update
					    // To find the interval we find the max of AvX and AvY and work out how many frames
					    // it will take to move X pels (GF_UPDATE_MOTION_INTERVAL in 1/4 pel) assuming the motion 
					    // level does not change. The value is then capped to the range MIN_GF_UPDATE_INTERVAL to MAX_GF_UPDATE_INTERVAL
					    if ( cpi->GfuMotionSpeed > 0 )
					    {
						    cpi->GfUpdateInterval = GF_UPDATE_MOTION_INTERVAL / cpi->GfuMotionSpeed;

						    if ( cpi->GfUpdateInterval < MIN_GF_UPDATE_INTERVAL )
							    cpi->GfUpdateInterval = MIN_GF_UPDATE_INTERVAL;

						    else if ( cpi->GfUpdateInterval > MAX_GF_UPDATE_INTERVAL )
							    cpi->GfUpdateInterval = MAX_GF_UPDATE_INTERVAL;

					    }
					    else
						    cpi->GfUpdateInterval = MAX_GF_UPDATE_INTERVAL;



				    }
			    }
            }
		}

		// If we have a mode where RD opt is to be used re-do pickmodes with rdopt enabled
		if (cpi->QuickCompress == 0)
			cpi->RdOpt = 2;
		else if  (cpi->QuickCompress == 3)
			cpi->RdOpt = 2;
			//cpi->RdOpt = 1;

		// Get a cost estimate for the sake of RD opt.
		// As we have not yet done pick modes for this frame this is by necessity 
		// based upon stats from the last frame.
		//if ( cpi->RdOpt )
		{
			RegulateQ ( cpi, (cpi->ThisFrameTarget - (cpi->ModeMvCostEstimate/64)) );
		}

		// Select the optimal modes
		PickModes ( cpi, &InterError, &IntraError ); 

        // Normalize the key frame indicator to the range 0-100
		actualMBS   = (cpi->pb.MBRows - (BORDER_MBS*2)) * (cpi->pb.MBCols - (BORDER_MBS*2));
	    KFIndicator = (cpi->MotionScore * 100)/((actualMBS * 2)/3);

        cpi->InterErrorb = InterError;
        cpi->InterError = InterError;
        cpi->IntraError = IntraError;

        // Test for auto key frame.
        if( cpi->AutoKeyFrameEnabled )
        {

            if(    cpi->pass < 2  
                && KFIndicator > (UINT32) cpi->AutoKeyFrameThreshold
                && cpi->LastKeyFrame > cpi->MinimumDistanceToKeyFrame
                && (   cpi->IntraError < 2 * cpi->InterError 
                    && cpi->IntraError < cpi->InterError + 2000  * actualMBS  
                   )
                && (   100 * abs(cpi->InterError - cpi->LastInterError ) / cpi->LastInterError > 40
                    || 100 * abs(cpi->LastIntraError - cpi->IntraError) / cpi->LastIntraError > 40
					|| cpi->IntraError * 5 < cpi->InterError * 6
                   )
              )
            {

                CompressKeyFrame(cpi);  // Code a key frame
                return;
            }

        }

        // Increment the frames since last key frame count
        cpi->LastKeyFrame++;

#if defined(_MSC_VER)
	    ClearSysState();
#endif

		// Maintain a record of GF usage over the last few frames
		// Each frame reduce value by 1/8 then add in usage (0-100) for the current frame
		{

			UINT32 ThisFrameGolden;

			ThisFrameGolden = cpi->ModeDist[CODE_USING_GOLDEN] + cpi->ModeDist[CODE_GOLDEN_MV] + 
				              cpi->ModeDist[CODE_GOLD_NEAREST_MV] + cpi->ModeDist[CODE_GOLD_NEAR_MV]; 

            ThisFrameGolden = (ThisFrameGolden * 100) / ((cpi->pb.MBRows-2*BORDER_MBS )*(cpi->pb.MBCols-2*BORDER_MBS));
            cpi->fps.PercentGolden = ThisFrameGolden;

			cpi->GfUsage = ((cpi->GfUsage * 7) + 4) / 8;
			cpi->GfUsage += ThisFrameGolden;
		}

        // Get an estimate of the Q that we should code at.
        RegulateQ ( cpi, (cpi->ThisFrameTarget - (cpi->ModeMvCostEstimate/64)) );

        cpi->DropCount = 0;


		// This code is experimental and needs further refinement.
		if ( cpi->pb.Vp3VersionNo > 7 )
		{
			INT32 IIRatio;
			UINT8 MaxAplha;
			UINT8 MinThresh;

			// Calucalte an intra inter ratio for blocks that use motion prediction.
			if ( cpi->MotionInterErr > 0 )
				IIRatio = (cpi->MotionIntraErr * 10)/cpi->MotionInterErr;
			else
				IIRatio = 10;

			// Set Bicubic alpha and apply Q related limits
			cpi->pb.PredictionFilterAlpha			= cpi->BaselineAlpha;

			// If a golden frame was thrown recently use its q for deciding alpha and thresholdd limits else the current frame Q.
			if ( cpi->FramesTillGfUpdateDue > 0 )
			{
				MaxAplha = BicubicMaxAlpha[cpi->LastGfOrKFrameQ];
				MinThresh = BicubicMinThresh[cpi->LastGfOrKFrameQ];
			}
			else
			{
				MaxAplha = BicubicMaxAlpha[cpi->pb.quantizer->FrameQIndex];
				MinThresh = BicubicMinThresh[cpi->pb.quantizer->FrameQIndex];
			}

			cpi->pb.PredictionFilterMode             = AUTO_SELECT_PM;	

			// Select the filtering parameters based upon the inter intra ratio
			if ( IIRatio < 15 )
			{
				cpi->pb.PredictionFilterVarThresh	 = 31;
			}
			else if ( IIRatio < 20 )
			{
				cpi->pb.PredictionFilterVarThresh    = cpi->BaselineBicThresh + 16;
			}
			else if ( IIRatio < 40 )
			{
				cpi->pb.PredictionFilterVarThresh    = cpi->BaselineBicThresh + 8;
			}
			else if ( IIRatio < 60 )
			{
				cpi->pb.PredictionFilterVarThresh    = cpi->BaselineBicThresh + 4;
			}
			else if ( IIRatio < 80 )
			{
				cpi->pb.PredictionFilterVarThresh    = cpi->BaselineBicThresh + 2;
			}
			else if ( IIRatio < 100 )
			{
				cpi->pb.PredictionFilterVarThresh    = cpi->BaselineBicThresh + 1;
				cpi->pb.PredictionFilterAlpha        += 1;
			}
			else 
			{
				cpi->pb.PredictionFilterVarThresh    = cpi->BaselineBicThresh;
				cpi->pb.PredictionFilterAlpha        += 1;
			}

			// Limit check alpha
			if ( cpi->pb.PredictionFilterAlpha > MaxAplha )
				cpi->pb.PredictionFilterAlpha = MaxAplha;

			// Limit check variance threshold
			if ( cpi->pb.PredictionFilterVarThresh > 31 )
				cpi->pb.PredictionFilterVarThresh = 31;
			else if ( cpi->pb.PredictionFilterVarThresh < MinThresh )
				cpi->pb.PredictionFilterVarThresh = MinThresh;		
		}

        /* Proceed with the frame update. */
        UpdateFrame ( cpi );
    }
	else
	{
		// Update the buffer level variable.
		cpi->BytesOffTarget += cpi->PerFrameBandwidth;

		// Are we are using the secondary buffer limit constraints
		if ( cpi->MaxAllowedDatarate )
		{
			cpi->BufferLevel += ((cpi->MaxAllowedDatarate * cpi->PerFrameBandwidth) / 100);           
			if ( cpi->BufferLevel > cpi->MaxBufferLevel )
				cpi->BufferLevel = cpi->MaxBufferLevel;
		}
		// else update the secondary buffer level in line with the current buffer level
		else
		{
			cpi->BufferLevel = cpi->BytesOffTarget;
		}

		// Update the drop frame flag etc.
		cpi->DropFrame = FALSE;
		cpi->DropCount++;
		TotDropFrameCount++;
		DropedFrame = TRUE;
	}
}


/****************************************************************************
 * 
 *  ROUTINE       : PredictScanOrder
 *
 *  INPUTS        : CP_INSTANCE *cpi   : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Work out an optimal DCT coefficient scan order based
 *                  upon stats gathered from previous frame.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void PredictScanOrder ( CP_INSTANCE *cpi )
{
	UINT32 i, j, k;
	UINT32 Sum;
	UINT32 tmp2[2];
	UINT32 tmp[BLOCK_SIZE][2];
	UINT32 GroupStartPoint, GroupEndPoint;

	// Convert frame nz counts to ratio values vs frame zero counts
	for ( i=1; i<BLOCK_SIZE; i++ )
	{
		Sum = cpi->FrameNzCount[i][0] + cpi->FrameNzCount[i][1];
		if ( Sum )
			tmp[i][0] = (cpi->FrameNzCount[i][1]*255)/Sum;
		else
			tmp[i][0] = 0;			
		tmp[i][1] = i;
	}

	// Sort into decending order.
	for ( i=1; i<BLOCK_SIZE-1; i++ )
	{
		for ( j=i+1; j>1; j-- )
		{
			if ( tmp[j][0] > tmp[j-1][0] )
			{
				// Swap them over
				tmp2[0] = tmp[j-1][0];
				tmp2[1] = tmp[j-1][1];

				tmp[j-1][0] = tmp[j][0];
				tmp[j-1][1] = tmp[j][1];

				tmp[j][0] = tmp2[0];
				tmp[j][1] = tmp2[1];
			}
		}
	}

	// Split the coeffs into value range groups then re-sort within each group 
	// into ascending order based upon zig zag scan position
	GroupEndPoint = 0;
	for ( k=0; k<SCAN_ORDER_BANDS; k++ )
	{
		GroupStartPoint = GroupEndPoint+1;
		GroupEndPoint = EndpointLookup[k];

		for ( i=GroupStartPoint; i<GroupEndPoint; i++ )
		{
			for ( j=i+1; j>GroupStartPoint; j-- )
			{
				if ( tmp[j][1] < tmp[j-1][1] )
				{
					// Swap them over
					tmp2[0] = tmp[j-1][0];
					tmp2[1] = tmp[j-1][1];

					tmp[j-1][0] = tmp[j][0];
					tmp[j-1][1] = tmp[j][1];

					tmp[j][0] = tmp2[0];
					tmp[j][1] = tmp2[1];
				}
			}
		}

		// For each coef index mark its band number
		for ( i=GroupStartPoint; i<=GroupEndPoint; i++ )
		{
			// Note the scan band number for each coef.
			// tmp[i][1] is the position of the coef in the traditional zig-zag scan order, 
			// i is the position in the new scan order and K is the band number.
			cpi->NewScanOrderBands[tmp[i][1]] = k;	
		}
	} 
}

/****************************************************************************
 *
 *  ROUTINE       :     UpdateFrame
 *
 *  INPUTS        :     None.
 *
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Writes the fragment data to the output file and updates
 *                      the displayed frame.
 *
 *  SPECIAL NOTES :     None.
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void UpdateFrame ( CP_INSTANCE *cpi )
{
    double FramePSNR = 0.0;
    PB_INSTANCE *pbi = &cpi->pb;

	// Key frames can not have backwards dependancy so set up defaults for pbi->ScanBands.
	if ( VP6_GetFrameType( pbi ) == BASE_FRAME )
	{
		// Set starting point for key frames... These cannot rely on what went before
		if ( pbi->Configuration.Interlaced )
			memcpy ( pbi->ScanBands, DefaultInterlacedScanBands, sizeof(pbi->ScanBands) );
		else
			memcpy ( pbi->ScanBands, DefaultNonInterlacedScanBands, sizeof(pbi->ScanBands) );
	}

	// Based upon the previous coded frame work out a predicted best 
	// scan order banding for coding this frame
	if ( (cpi->CurrentFrame > 1) && (!cpi->ErrorResilliantMode) &&
		 ((pbi->Configuration.Interlaced) || (cpi->AllowScanOrderUpdates)) )
	{
		PredictScanOrder( cpi );
	}
	else
	{
		// Chose between default interlaced and non-interlaced sets.
		if ( pbi->Configuration.Interlaced )
			memcpy ( cpi->NewScanOrderBands, DefaultInterlacedScanBands, sizeof(cpi->NewScanOrderBands) );
		else
			memcpy ( cpi->NewScanOrderBands, DefaultNonInterlacedScanBands, sizeof(cpi->NewScanOrderBands) );
	}

	// Build the scan order
	BuildScanOrder ( pbi, cpi->NewScanOrderBands );

    // Encode the frame.
	EncodeData ( cpi );

    /* Update the BpbCorrectionFactor variable according to whether or not we were
    *  close enough with our selection of DCT quantiser.
    */
    if ( VP6_GetFrameType( pbi ) != BASE_FRAME )

        UpdateBpbCorrectionFactor ( cpi, cpi->ThisFrameSize );

    // Adjust carry over and or key frame context.
    if ( VP6_GetFrameType( pbi ) == BASE_FRAME )
        AdjustKeyFrameContext ( cpi );

    cpi->TotalByteCount += (cpi->ThisFrameSize/8);

	// The auto-drop frame code is only used in buffered mode.
	// In unbufferd mode (eg video conferencing) the descision to
	// code or drop a frame is made outside the codec in response to real
	// world comms or buffer considerations.
	if ( cpi->BufferedMode )
	{
		// If the frame was massively oversize and we are below optimal buffer level drop next frame
		if ( (cpi->DropFramesAllowed) &&
			 (cpi->BufferLevel < cpi->OptimalBufferLevel) && 
			 ((int)cpi->ThisFrameSize > (4 * cpi->ThisFrameTarget))  )
		{
			cpi->DropFrame = TRUE;
		}

		// Set the count for maximum consequative dropped frames based upon ratio of 
		// this frame size to target size for this frame.

		if(cpi->ThisFrameTarget > 0) 
		{
			cpi->MaxDropCount = (cpi->ThisFrameSize / cpi->ThisFrameTarget);
			if ( cpi->MaxDropCount > cpi->MaxConsecDroppedFrames )
				cpi->MaxDropCount = cpi->MaxConsecDroppedFrames;
		}
	}

    // If appropriate call the frame PSNR function
#if defined PSNR_ON
    if ( !cpi->AllowSpatialResampling )
    {
		if ( cpi->pb.quantizer->FrameQIndex < PPROC_QTHRESH )
		{
			cpi->pb.PostProcessingLevel = 4;

			PostProcess ( cpi->pb.postproc,
				          cpi->pb.Vp3VersionNo,
				          cpi->pb.FrameType,
				          cpi->pb.PostProcessingLevel,
				          cpi->pb.quantizer->FrameQIndex,
				          cpi->pb.LastFrameRecon,
				          cpi->pb.PostProcessBuffer,
				          (unsigned char *) cpi->pb.FragInfo,
				          sizeof(FRAG_INFO),
				          0x0001 );
		}
		else
			cpi->pb.PostProcessingLevel = 0;

        FramePSNR = CalcPSNR ( cpi );
    }
#endif

    // If appropriate call the frame PSNR function
#if defined FILE_PSNR 
    if ( !cpi->AllowSpatialResampling )
    {
		if ( cpi->pb.quantizer->FrameQIndex < PPROC_QTHRESH )
		{
			cpi->pb.PostProcessingLevel=4;

			PostProcess
				(
				cpi->pb.postproc,
				cpi->pb.Vp3VersionNo,
				cpi->pb.FrameType,
				cpi->pb.PostProcessingLevel,
				cpi->pb.quantizer->FrameQIndex,
				cpi->pb.LastFrameRecon,
				cpi->pb.PostProcessBuffer,
				(unsigned char *) cpi->pb.FragInfo,
				sizeof(FRAG_INFO),
				0x0001
				);
		}
		else
			cpi->pb.PostProcessingLevel=0;
    }
#endif


	// Update the buffer level variable.
	cpi->BytesOffTarget += (cpi->PerFrameBandwidth - cpi->ThisFrameSize);

	// Are we are using the secondary buffer limit constraints
	if ( cpi->MaxAllowedDatarate  )
	{
		cpi->BufferLevel += (((cpi->MaxAllowedDatarate * cpi->PerFrameBandwidth) / 100) - cpi->ThisFrameSize);           
		if ( cpi->BufferLevel > cpi->MaxBufferLevel )
			cpi->BufferLevel = cpi->MaxBufferLevel;
	}
	// else update the secondary buffer level in line with the current buffer level
	else
	{
		cpi->BufferLevel = cpi->BytesOffTarget;
	}

	// If appropriate update the "last key frame buffer level" value.
	if ( VP6_GetFrameType( pbi ) == BASE_FRAME )
		cpi->LastKeyFrameBufferLevel = cpi->BufferLevel;

	// Keep a record of ambient average Q.
	if ( pbi->FrameType == BASE_FRAME )
		pbi->AvgFrameQIndex = pbi->quantizer->FrameQIndex;
	else
		pbi->AvgFrameQIndex = (2 + 3 * pbi->AvgFrameQIndex + pbi->quantizer->FrameQIndex) / 4 ;

}
