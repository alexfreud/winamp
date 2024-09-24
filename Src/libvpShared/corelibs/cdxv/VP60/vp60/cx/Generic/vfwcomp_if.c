/****************************************************************************
*
*   Module Title :     vfwcomp_if.c
*
*   Description  :     Compressor interface definition.
*
****************************************************************************/

/****************************************************************************
*  Header Files
****************************************************************************/
#include <stdio.h>
#include "compdll.h"
#include "mcomp.h"
#include "misc_common.h"
#include "vp60eversion.h"
#include "twopass.h"
#include <math.h>
/****************************************************************************
*  Macros
****************************************************************************/
#define CommentString "\nON2.COM VERSION VP60E " VP60EVERSION "\n"

#ifdef _MSC_VER
#pragma comment(exestr,CommentString)
#endif

/****************************************************************************
*  Typedefs
****************************************************************************/

typedef struct _COMPRESSOR_STATE
{
    UINT32 PriorKeyFrameSize[KEY_FRAME_CONTEXT];
    UINT32 PriorKeyFrameDistance[KEY_FRAME_CONTEXT];
    INT64  CurrentFrame;
    UINT32 LastFrameSize;
    INT32  DropCount;
    INT64  KeyFrameCount;
    INT64  TotKeyFrameBytes;
    UINT32 LastKeyFrameSize;
    UINT32 LastKeyFrame;
    INT64  TotalByteCount;
    UINT32 ActiveMaxQ;
    double BpbCorrectionFactor;
} COMPRESSOR_STATE;

/****************************************************************************
*  Module Statics
****************************************************************************/
static const char vp60eVersion[] = VP60EVERSION;
static INT32 ClipBytes;

//#define TIMING
#ifdef TIMING
#include "mmsystem.h"
static long ITotalTime=0;
static long ITime1, ITime2;
#endif

#if defined MEASURE_SECTION_COSTS
UINT32 ClipSectionBits[10] = {0,0,0,0,0,0,0,0,0,0};
#endif

static const UINT8 BicThreshTable[11] = { 31, 31, 31, 16,  8,  4,  3,  2,  1,  1,   1};
static const UINT8 BicAlphaTable[11]  = {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10};

/****************************************************************************
*  Imports
****************************************************************************/
extern UINT32  scanupdates[64][2];

extern void ScaleFrame (
  YUV_BUFFER_CONFIG *src,
  YUV_BUFFER_CONFIG *dst,
  unsigned char *tempArea,
  unsigned char tempHeight,
  unsigned int hscale,
  unsigned int hratio,
  unsigned int vscale,
  unsigned int vratio,
  unsigned int interlaced
  );

extern void CompressFirstFrame ( CP_INSTANCE *cpi );
extern void CompressKeyFrame ( CP_INSTANCE *cpi );
extern void CompressFrame ( CP_INSTANCE *cpi, UINT32 FrameNumber );

/****************************************************************************
 *
 *  ROUTINE       : VP60E_GetVersionNumber
 *
 *  INPUTS        : None.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : const char *CCONV: Pointer to version string.
 *
 *  FUNCTION      : Returns a pointer to the version string.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
const char *CCONV VP60E_GetVersionNumber ( void )
{
    return vp60eVersion;
}

/****************************************************************************
 *
 *  ROUTINE       : ChangeEncoderSize
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *                  UINT32 Width     : New frame Width.
 *                  UINT32 Height    : New frame Height.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Updates the encoder frame size.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void CCONV ChangeEncoderSize ( CP_INSTANCE *cpi, UINT32 Width, UINT32 Height )
{
    // Frame size __MUST__ be multiple of 16 pels in each dimension
    cpi->pb.Configuration.VideoFrameHeight = ((Height+15)&0xFFFFFFF0); 
    cpi->pb.Configuration.VideoFrameWidth  = ((Width +15)&0xFFFFFFF0);
    cpi->pb.YPlaneSize = 0xFFF;

    // Initialise image format details
    if ( !VP6_InitFrameDetails( &cpi->pb ) )
        return;

    if ( !EAllocateFragmentInfo ( cpi ) )
    {
        VP6_DeleteFragmentInfo ( &cpi->pb );
        VP6_DeleteFrameInfo ( &cpi->pb );
        return;
    }

    if ( !EAllocateFrameInfo ( cpi ) )
    {
        VP6_DeleteFragmentInfo ( &cpi->pb );
        VP6_DeleteFrameInfo ( &cpi->pb );
        EDeleteFragmentInfo ( cpi );
        return;
    }

    // Initialise Motion compensation
    InitMotionCompensation ( cpi );
}

/****************************************************************************
 *
 *  ROUTINE       : PickSizeStep
 *
 *  INPUTS        : CP_INSTANCE *cpi            : Pointer to encoder instance.
 *                  COMP_CONFIG_VP6 *CompConfig : Encoder configuration.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Starts & initializes encoder's size stepping mechanism.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void pickSizeStep ( CP_INSTANCE *cpi, COMP_CONFIG_VP6 *CompConfig )
{
    double bitsPerPixel;

    int Width  = ((CompConfig->FrameSize & 0xFFFF0000) >> 16);
    int Height = CompConfig->FrameSize & 0x0000FFFF;

	if ( CompConfig->FrameRate == 0 )
		CompConfig->FrameRate = 30;

	if ( Width==0 )
		Width = 320;

	if ( Height== 0 )
		Height = 240;

    bitsPerPixel = (CompConfig->TargetBitRate * 1024.0) /
                   (CompConfig->FrameRate * Width * Height);

    // drop size to 4/5 before dropping frame rate to 1/2 or 1/3
    if ( bitsPerPixel < 0.03 )         // VP4 was 0.043
    {
        cpi->SizeStep = 2;
        bitsPerPixel = (CompConfig->TargetBitRate * 1024.0) /
                       (CompConfig->FrameRate * Width * Height * 4/5 * 4/5);
    }

    cpi->FrameRateInput = CompConfig->FrameRate;
    cpi->FrameRateDropFrames = 0;

    if ( cpi->DropFramesAllowed )
    {
        // figure out output frame rate
        if ( bitsPerPixel > 0.025 )         
            cpi->FrameRateDropFrames = 0;
        else if ( bitsPerPixel > 0.015 )
            cpi->FrameRateDropFrames = 1;
        else
            cpi->FrameRateDropFrames = 2;
    }

    cpi->FrameRateDropCount = 0;
    cpi->Configuration.OutputFrameRate = CompConfig->FrameRate / (cpi->FrameRateDropFrames+1);
    
    bitsPerPixel = (CompConfig->TargetBitRate * 1024.0) /
                   (cpi->Configuration.OutputFrameRate * Width * Height);
    
    // categorize the cpi->SizeStep of the clip by the number of
    // bits we are allowing per pixel!
    if( bitsPerPixel > 0.090 )
        cpi->SizeStep = 0;
    else if( bitsPerPixel > 0.060 )     // VP4 was 0.09
        cpi->SizeStep = 1;
    else if ( bitsPerPixel > .040 )     // VP4 was 0.070
        cpi->SizeStep = 2;
    else if ( bitsPerPixel > .030 )     // VP4 was 0.06
        cpi->SizeStep = 3;
    else if ( bitsPerPixel > .015 )     // VP4 was 0.043
        cpi->SizeStep = 4;
    else
        cpi->SizeStep = 5;
}


/****************************************************************************
 *
 *  ROUTINE       : ChangeEncoderConfig
 *
 *  INPUTS        : CP_INSTANCE *cpi            : Pointer to encoder instance.
 *                  COMP_CONFIG_VP6 *CompConfig : Encoder configuration.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Updates encoder with new configuration.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void CCONV ChangeEncoderConfig ( CP_INSTANCE *cpi, COMP_CONFIG_VP6 *CompConfig )
{
    INT32 Quality = CompConfig->Quality;
    cpi->BufferedMode                  = (CompConfig->OptimalBufferLevel > 0) ? TRUE : FALSE;
    cpi->AutoKeyFrameEnabled           = CompConfig->AutoKeyFrameEnabled;
    cpi->MinimumDistanceToKeyFrame     = CompConfig->MinimumDistanceToKeyFrame;
    cpi->ForceKeyFrameEvery            = CompConfig->ForceKeyFrameEvery;
    cpi->PreProcFilterLevel            = CompConfig->NoiseSensitivity;
    cpi->AllowSpatialResampling        = CompConfig->AllowSpatialResampling && cpi->BufferedMode;  // NOTE: disallow if mode is unbuffered.
    cpi->AutoKeyFrameThreshold         = CompConfig->AutoKeyFrameThreshold;
    cpi->CPUUsed                       = CompConfig->Speed;
    cpi->Configuration.TargetBandwidth = CompConfig->TargetBitRate * 1024;
    cpi->ActualTargetBitRate           = cpi->Configuration.TargetBandwidth;

    cpi->OptimalBufferLevel            = CompConfig->OptimalBufferLevel * cpi->Configuration.TargetBandwidth;
    cpi->StartingBufferLevel           = CompConfig->StartingBufferLevel * cpi->Configuration.TargetBandwidth;
    cpi->MaxBufferLevel				   = CompConfig->MaximumBufferSize * cpi->Configuration.TargetBandwidth;
	
	cpi->DropFramesWaterMark           = (cpi->OptimalBufferLevel * CompConfig->DropFramesWaterMark) / 100;
    cpi->ResampleDownWaterMark         = (cpi->OptimalBufferLevel * CompConfig->ResampleDownWaterMark) / 100;
    cpi->ResampleUpWaterMark           = (cpi->OptimalBufferLevel * CompConfig->ResampleUpWaterMark) / 100;

    cpi->DisableGolden                 = CompConfig->DisableGolden       ;         
    cpi->VBMode                        = CompConfig->VBMode              ; 
    cpi->BestAllowedQ                  = CompConfig->BestAllowedQ        ;          
    cpi->UnderShootPct                 = CompConfig->UnderShootPct       ;         

    cpi->MaxAllowedDatarate            = CompConfig->MaxAllowedDatarate  ;    
    cpi->MaximumBufferSize             = CompConfig->MaximumBufferSize   ;     

    cpi->TwoPassVBREnabled             = CompConfig->TwoPassVBREnabled   ;     
    cpi->TwoPassVBRBias                = CompConfig->TwoPassVBRBias      ;        
    cpi->TwoPassVBRMaxSection          = CompConfig->TwoPassVBRMaxSection;  
    cpi->TwoPassVBRMinSection          = CompConfig->TwoPassVBRMinSection;  
    cpi->Pass                          = CompConfig->Pass                ;                  
    cpi->ErrorResilliantMode           = CompConfig->ErrorResilientMode; 

	if(cpi->ErrorResilliantMode) 
		cpi->DisableGolden             =1;

    cpi->DropFramesAllowed             = CompConfig->AllowDF && cpi->BufferedMode; // NOTE: disallow if mode is unbuffered.
	cpi->MaxConsecDroppedFrames		   = 4;	// TBD
    cpi->QuickCompress                 = CompConfig->QuickCompress;

	cpi->BaselineAlpha				   = BicAlphaTable[CompConfig->Sharpness];
	cpi->BaselineBicThresh			   = BicThreshTable[CompConfig->Sharpness];

    if(CompConfig->TwoPassVBRMaxSection == DEFAULT_VALUE)
        cpi->TwoPassVBRMaxSection = CompConfig->MaxAllowedDatarate;

    if( CompConfig->FixedQ > 0 )
        cpi->FixedQ = 63 - CompConfig->Quality;
	else
		cpi->FixedQ = -1;

    // compression mode dependant
    switch(CompConfig->Mode)
    {
    case MODE_REALTIME: 
    	cpi->Speed = 4;
        if(CompConfig->QuickCompress == DEFAULT_VALUE)
            cpi->QuickCompress = 2;
        if(CompConfig->Pass == DEFAULT_VALUE)
            cpi->Pass = 0;                            
        break;
    case MODE_GOODQUALITY:
        if(CompConfig->QuickCompress == DEFAULT_VALUE)
            cpi->QuickCompress = 1;
        if(CompConfig->Pass == DEFAULT_VALUE)
            cpi->Pass = 0;                            
        break;
    case MODE_BESTQUALITY:
        if(CompConfig->QuickCompress == DEFAULT_VALUE)
            cpi->QuickCompress = 0;
        if(CompConfig->Pass == DEFAULT_VALUE)
            cpi->Pass = 0;                            

        break;
    case MODE_FIRSTPASS:
        if(CompConfig->QuickCompress == DEFAULT_VALUE)
            cpi->QuickCompress = 1;
        if(CompConfig->Pass == DEFAULT_VALUE)
            cpi->Pass = 1;                            
        cpi->PreProcFilterLevel = 0;
        cpi->FixedQ = FIRSTPASS_Q;
        cpi->ForceKeyFrameEvery = 99999;
        cpi->AutoKeyFrameThreshold = 50;
        cpi->MinimumDistanceToKeyFrame = 0;
        cpi->AllowSpatialResampling = 0;
        cpi->DropFramesAllowed = 0;
        break;

    case MODE_SECONDPASS:
        if(CompConfig->QuickCompress == DEFAULT_VALUE)
            cpi->QuickCompress = 1;
        if(CompConfig->Pass == DEFAULT_VALUE)
            cpi->Pass = 2;                            
        break;
    case MODE_SECONDPASS_BEST:
        if(CompConfig->QuickCompress == DEFAULT_VALUE)
            cpi->QuickCompress = 0;
        if(CompConfig->Pass == DEFAULT_VALUE)
            cpi->Pass = 2;                            
        break;

    } 

	// Are we planning local file playback or streamed
	cpi->EndUsage = CompConfig->EndUsage;

	// We auto-adjust worst quality for 1 pass modes only and 
	// disable when coding real time.
	if ( (CompConfig->Mode < MODE_SECONDPASS) && (cpi->QuickCompress != 2) )
		cpi->AutoWorstQ = TRUE;
	else
		cpi->AutoWorstQ = FALSE;
	
    // endusage dependent
	// 1 pass + local file playback
    if(CompConfig->EndUsage == USAGE_LOCAL_FILE_PLAYBACK && CompConfig->Mode < MODE_SECONDPASS)
    {

        cpi->MaxAllowedDatarate = 200;
        cpi->StartingBufferLevel = 4 * cpi->Configuration.TargetBandwidth;
        cpi->OptimalBufferLevel = 4 * cpi->Configuration.TargetBandwidth;
        cpi->MaxBufferLevel = 5 * cpi->Configuration.TargetBandwidth;
        cpi->VBMode = 1;                          
        cpi->TwoPassVBREnabled = 0;   
    }
	// 2 pass local file playback
    else if(CompConfig->EndUsage == USAGE_LOCAL_FILE_PLAYBACK && CompConfig->Mode >= MODE_SECONDPASS)
    {
        cpi->MaxAllowedDatarate = 400;
        cpi->StartingBufferLevel = 10 * cpi->Configuration.TargetBandwidth;
        cpi->OptimalBufferLevel = 10 * cpi->Configuration.TargetBandwidth;
        cpi->MaxBufferLevel = 10 * cpi->Configuration.TargetBandwidth;
        cpi->VBMode = 1;                          
        cpi->TwoPassVBREnabled = 1;               
    }
	// 1 or 2 pass streaming playback
    else
    {
        cpi->VBMode = 0;                          
        cpi->TwoPassVBREnabled = 0;               
    }


    //if(cpi->QuickCompress == 0)
        //cpi->QuickCompress = 3;
    //if(cpi->QuickCompress == 3)
    //    cpi->QuickCompress = 0;


    // Set the output frame rate.
    cpi->Configuration.OutputFrameRate = CompConfig->FrameRate;
    if ( cpi->Configuration.OutputFrameRate < 1 )
        cpi->Configuration.OutputFrameRate = CompConfig->OutputFrameRate;
    else if ( cpi->Configuration.OutputFrameRate > 1000 )
        cpi->Configuration.OutputFrameRate = 1000;

    // Set key frame data rate target and frequency
    cpi->KeyFrameDataTargetOrig = (CompConfig->KeyFrameDataTarget * 1024);
    cpi->KeyFrameDataTarget     = cpi->KeyFrameDataTargetOrig;
    if(cpi->KeyFrameDataTarget > (int) cpi->Configuration.TargetBandwidth / 2)
        cpi->KeyFrameDataTarget = (int) cpi->Configuration.TargetBandwidth / 2;

    cpi->KeyFrameFrequency = CompConfig->KeyFrameFrequency;

    cpi->BytesOffTarget = cpi->StartingBufferLevel;				// Set the current buffer level
    cpi->BufferLevel = cpi->StartingBufferLevel;				// Set the current buffer level

    cpi->LastKeyFrameBufferLevel = cpi->StartingBufferLevel;	// Used to monitor changes in buffer level when considering re-sampling.

    cpi->pb.Configuration.Interlaced  = CompConfig->Interlaced;
    cpi->pb.Configuration.HScale      = CompConfig->HScale;
    cpi->pb.Configuration.HRatio      = CompConfig->HRatio;
    cpi->pb.Configuration.VScale      = CompConfig->VScale;
    cpi->pb.Configuration.VRatio      = CompConfig->VRatio;
    cpi->pb.Configuration.ScalingMode = CompConfig->ScalingMode;

    // Set the quality settings.
    ConfigureQuality ( cpi, Quality );

    /* Set the video frame size. */
    if ( CompConfig->FrameSize != 
        (unsigned int) ((cpi->YuvInputData.YWidth << 16) | cpi->YuvInputData.YHeight) )
    {
        ChangeEncoderSize ( cpi, ((CompConfig->FrameSize & 0xFFFF0000) >> 16),
            CompConfig->FrameSize & 0x0000FFFF);

        cpi->InputConfig.YWidth   = ((CompConfig->FrameSize & 0xFFFF0000) >> 16);
        cpi->InputConfig.YHeight  = CompConfig->FrameSize & 0x0000FFFF;
        cpi->InputConfig.YStride  = cpi->InputConfig.YWidth;
        cpi->InputConfig.UVWidth  = cpi->InputConfig.YWidth /2 ;
        cpi->InputConfig.UVHeight = (CompConfig->FrameSize & 0x0000FFFF) /2;
        cpi->InputConfig.UVStride = cpi->InputConfig.YWidth/2;

        cpi->SizeStep = 0;
    }

    if(cpi->BufferedMode )
        pickSizeStep ( cpi, CompConfig );

    cpi->InterFrameTarget  =  cpi->Configuration.TargetBandwidth / cpi->Configuration.OutputFrameRate;
    cpi->PerFrameBandwidth = (cpi->Configuration.TargetBandwidth / cpi->Configuration.OutputFrameRate);
    // Calculate a new target bytes per frame allowing for predicted key frame frequency and size.
    if ( (INT32)cpi->Configuration.TargetBandwidth > ((cpi->KeyFrameDataTarget * cpi->Configuration.OutputFrameRate)/cpi->KeyFrameFrequency) )
        cpi->InterFrameTarget =  (INT32)((cpi->Configuration.TargetBandwidth - ((cpi->KeyFrameDataTarget * cpi->Configuration.OutputFrameRate)/cpi->KeyFrameFrequency)) / cpi->Configuration.OutputFrameRate);
    else
        cpi->InterFrameTarget = 1; 


    cpi->pass = cpi->Pass;
    if(cpi->pass)
        Pass2Initialize(cpi,CompConfig);
}

/****************************************************************************
 *
 *  ROUTINE       : StartEncoder
 *
 *  INPUTS        : COMP_CONFIG_VP6 *CompConfig : Encoder configuration.
 *
 *  OUTPUTS       : CP_INSTANCE **cpi           : Pointer to pointer to encoder instance.
 *
 *  RETURNS       : BOOL: TRUE=success, FALSE=failure.
 *
 *  FUNCTION      : Creates a new encoder instance & initializes it.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
BOOL CCONV StartEncoder ( CP_INSTANCE **cpi, COMP_CONFIG_VP6 *CompConfig )
{
#ifdef TIMING
    // DEBUG CODE
    ITime1=timeGetTime();
    {
        FILE *fp = fopen( "d:\\Times.txt", "at" );
        fprintf(fp, "StartEncoder: %d\n",ITime1);
        fclose(fp);
    }
#endif

    // Create an instance of the encoder
    *cpi = CreateCPInstance();

    // Initialisation default config.
    (*cpi)->pb.Configuration.HFragPixels = 8;
    (*cpi)->pb.Configuration.VFragPixels = 8;
    (*cpi)->pb.postproc = CreatePostProcInstance ( &((*cpi)->pb.Configuration) );
    (*cpi)->pb.quantizer = VP6_CreateQuantizer();


	// profile 4 is actually encode version 8 
	if(CompConfig->Profile == 4) 
	{
	    (*cpi)->pb.VpProfile  = 3;
		(*cpi)->pb.Vp3VersionNo = 8;
	}
	else
	{
	    (*cpi)->pb.VpProfile  = CompConfig->Profile;
		(*cpi)->pb.Vp3VersionNo = 6;
	}

    ChangeEncoderConfig ( *cpi, CompConfig );

    /* set the encoder version number */

    /* Initialise the compression process. */
    (*cpi)->CurrentFrame                = 1;
    (*cpi)->BpbCorrectionFactor         = 1.0;
    (*cpi)->KeyFrameBpbCorrectionFactor = 0.4;
	(*cpi)->GfuBpbCorrectionFactor      = 2.0;
    (*cpi)->TotalByteCount              = 0;
    (*cpi)->TotalMotionScore            = 0;

	(*cpi)->NiTotQi = 0;
	(*cpi)->NiFrames = 0;
	(*cpi)->NiAvQi = (*cpi)->Configuration.WorstQuality;

    // This makes sure encoder version specific tables are initialised
    VP6_InitQTables ( (*cpi)->pb.quantizer, (*cpi)->pb.Vp3VersionNo );

    // Indicate that the next frame to be compressed is the first in the current clip.
    (*cpi)->ThisIsFirstFrame = TRUE;

    // Initialize the drop frame flags
    (*cpi)->DropFrame = FALSE;
	(*cpi)->MaxConsecDroppedFrames = 4;

#if defined PSNR_ON
    // DEBUG: Clear down PSNR variables
    (*cpi)->TotalSqError =0.0;
    (*cpi)->TotPsnr  = 0.0;
    (*cpi)->TotYPsnr = 0.0;
    (*cpi)->TotUPsnr = 0.0;
    (*cpi)->TotVPsnr = 0.0;
    (*cpi)->MinPsnr  = 999.00;
    (*cpi)->MinYPsnr = 999.00;
    (*cpi)->MinUPsnr = 999.00;
    (*cpi)->MinVPsnr = 999.00;
    (*cpi)->MaxPsnr  = 0.0;
    (*cpi)->MaxYPsnr = 0.0;
    (*cpi)->MaxUPsnr = 0.0;
    (*cpi)->MaxVPsnr = 0.0;
#endif

#ifdef MAPCA    
    InitMERefDs();
#endif
    return TRUE;
}

/****************************************************************************
 *
 *  ROUTINE       : ChangeCompressorSetting
 *
 *  INPUTS        : CP_INSTANCE *cpi  : Pointer to encoder instance.
 *                  C_SETTING Setting : Compreesor seeting to change.
 *                  int Value         : Value to set setting to.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Sets the specified compressor setting to the 
 *                  specified value.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void CCONV ChangeCompressorSetting ( CP_INSTANCE *cpi, C_SETTING Setting, int Value )
{
    switch ( Setting )
    {
    case C_SET_RECOVERY_FRAME:
        cpi->GfRecoveryFrame = TRUE;
        break;

    case C_SET_GOLDENFRAME:
        cpi->pb.RefreshGoldenFrame = TRUE;
        break;

    case C_SET_REFERENCEFRAME:
        CopyFrame ( cpi->pb.postproc, (YUV_BUFFER_CONFIG *) Value, cpi->pb.LastFrameRecon );
        CopyFrame ( cpi->pb.postproc, (YUV_BUFFER_CONFIG *) Value, cpi->pb.GoldenFrame );
        break;

    case C_SET_INTERNAL_SIZE:
        sscanf ( (unsigned char *)Value, "%d %d %d %d", &cpi->ForceHRatio, &cpi->ForceHScale, &cpi->ForceVRatio, &cpi->ForceVScale );
        cpi->ForceInternalSize = 1;
        cpi->ThisIsKeyFrame = TRUE;
        break;

    case C_SET_KEY_FRAME:
        cpi->ThisIsKeyFrame = TRUE;
        break;

    case C_SET_FIXED_Q:
        if ( (Value >= 0) && (Value < 64) )
            cpi->FixedQ = 63 - Value;
        break;

    case C_SET_FIRSTPASS_FILE:
        break;

    case C_SET_TESTMODE:
        cpi->pb.testMode = Value;
        break;

    default:
        if ( (Setting >= C_SET_EXPERIMENTAL_MIN) && (Setting <= C_SET_EXPERIMENTAL_MAX) )
        {
            INT32 nExperimental = Setting - C_SET_EXPERIMENTAL_MIN;

            if (nExperimental >= (INT32)cpi->nExperimentals)
                cpi->nExperimentals = nExperimental + 1;

            cpi->Experimental[nExperimental] = Value;

            switch(nExperimental)
            {
            case 0:
                cpi->DisableGolden = Value;
                break;
            case 1:
                cpi->VBMode = Value;
                break;
            case 2:
                cpi->BestAllowedQ = Value;
                break;
            case 3:
                cpi->UnderShootPct = Value;
                break;
            case 4:
                cpi->MaxAllowedDatarate = Value;
                break;
            case 5:
                cpi->MaximumBufferSize = Value;
		        cpi->MaxBufferLevel    = cpi->OptimalBufferLevel + ((cpi->MaximumBufferSize * cpi->Configuration.TargetBandwidth) / 100);
                break;
            case 250:
                cpi->TwoPassVBREnabled = Value;
                break;
            case 251:
                cpi->TwoPassVBRBias = Value;
                break;
            case 252:
                cpi->TwoPassVBRMaxSection = Value;
                break;
            case 253:
                cpi->TwoPassVBRMinSection = Value; 
                break;
            case 255:
                cpi->Pass = Value;
                cpi->pass = Value;
                if(cpi->pass == 2)
                {
                    char dummy[1024];
                    cpi->fs = fopen("firstpass.fst","r");
                    cpi->ss = fopen("firstpass.sst","r");

                    fgets(dummy,1024,cpi->fs);
                    fgets(dummy,1024,cpi->ss);

                    {   // calculate a q value to use 


                		int    actualMBS =                       // number of macroblocks
                              (cpi->pb.MBRows - (BORDER_MBS*2)) 
                            * (cpi->pb.MBCols - (BORDER_MBS*2));

                        double fpBitRate;                        // first pass bitrate
                        double target;                           // target bitrate
                        double NewQ;

                        const double RoomForVariation = 5;       // 5 q steps above

                        const double FirstPassQ = 32;            // 

                        InputStats(cpi->ss,&cpi->fpmss);

                        fpBitRate = cpi->fpmss.BitsPerMacroblock * actualMBS * cpi->Configuration.OutputFrameRate;
                        target = (double) cpi->Configuration.TargetBandwidth;

                        NewQ = (INT32)  63 -  ( RoomForVariation + FirstPassQ + .5 + log(fpBitRate/target) / log(1.05));
                        if(NewQ < cpi->Configuration.WorstQuality )
                            NewQ = cpi->Configuration.WorstQuality;

                        if(NewQ > cpi->Configuration.ActiveBestQuality)
                            NewQ = cpi->Configuration.ActiveBestQuality;

                        cpi->Configuration.WorstQuality = (INT32) NewQ;
                        cpi->Configuration.ActiveWorstQuality = cpi->Configuration.WorstQuality;

                        
                        /*
                        NewQ += 5*RoomForVariation;
                        if(NewQ < cpi->Configuration.WorstQuality )
                            NewQ = cpi->Configuration.WorstQuality;

                        if(NewQ > cpi->Configuration.ActiveBestQuality)
                            NewQ = cpi->Configuration.ActiveBestQuality;

                        cpi->Configuration.ActiveBestQuality = NewQ;

                        */



                    }
                }
                else if (cpi->pass == 1)
                {
                    cpi->fs = fopen("firstpass.fst","w");
                    fprintf(cpi->fs,
                        "%8s %8s %8s %8s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s \n",
                        "","#","key","golden","bits/mb","sq bits/mb","Inter","Intra","Motion","VarX","VarY",
                        "%Motion","%NewMotion","%Golden");

                    cpi->ss = fopen("firstpass.sst","w");
                    fprintf(cpi->ss,
                        "%8s %8s %8s %8s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s \n",
                        "","#","key","golden","bits/mb","sq bits/mb","Inter","Intra","Motion","VarX","VarY",
                        "%Motion","%NewMotion","%Golden");


                }
                break;
            }
        }


        break;
    }
}

/****************************************************************************
 *
 *  ROUTINE       : CopyOrResize
 *
 *  INPUTS        : CP_INSTANCE *cpi  : Pointer to encoder instance.
 *					BOOL ResetPreproc : Should the preprocessor be reset (e.g for a key frame)
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Copies and if necessary scales the frame cpi->YuvInputData
 *                  into the frame defined by cpi->InputConfig.
 *
 *  SPECIAL NOTES : cpi->pb.ThisFrameRecon is used as temporary workspace
 *                  for the scaler.
 *
 ****************************************************************************/
void CopyOrResize ( CP_INSTANCE *cpi, BOOL ResetPreproc )
{
    INT32  i;
    unsigned char *LocalDataPtr;
    unsigned char *InputDataPtr;
    YUV_BUFFER_CONFIG yuvConfig = cpi->InputConfig;  //  For tempFilter

    // Copy over input YUV to internal YUV buffers.
    if( cpi->InputConfig.YWidth != cpi->YuvInputData.YWidth ||
        cpi->InputConfig.YHeight!= cpi->YuvInputData.YHeight )
    {
        UINT8 tmpHeight;

        if( cpi->InputConfig.YHeight*2 == cpi->YuvInputData.YHeight )
            tmpHeight = 9;
        else
            tmpHeight = 11;

        cpi->InputConfig.YBuffer = (char *) cpi->yuv1ptr;
        cpi->InputConfig.UBuffer = (char *) &cpi->yuv1ptr[(cpi->pb.Configuration.VideoFrameHeight*cpi->pb.Configuration.VideoFrameWidth)];
        cpi->InputConfig.VBuffer = (char *) &cpi->yuv1ptr[((cpi->pb.Configuration.VideoFrameHeight*cpi->pb.Configuration.VideoFrameWidth)*5)/4];

        ScaleFrame ( &cpi->YuvInputData, &cpi->InputConfig, cpi->pb.ThisFrameRecon,tmpHeight,
                      cpi->pb.Configuration.HScale, cpi->pb.Configuration.HRatio,
                      cpi->pb.Configuration.VScale, cpi->pb.Configuration.VRatio,
                      cpi->pb.Configuration.Interlaced); 
    }
    else
    {
        // First copy over the Y data
        LocalDataPtr = cpi->yuv1ptr;
        InputDataPtr = (unsigned char *)cpi->YuvInputData.YBuffer;
        for ( i=0; i<cpi->YuvInputData.YHeight; i++ )
        {
            memcpy ( LocalDataPtr, InputDataPtr, cpi->YuvInputData.YWidth );
            LocalDataPtr += cpi->YuvInputData.YWidth;
            InputDataPtr += cpi->YuvInputData.YStride;
        }

        // Now copy over the U data
        LocalDataPtr = &cpi->yuv1ptr[(cpi->YuvInputData.YHeight * cpi->YuvInputData.YWidth)];
        InputDataPtr = (unsigned char *)cpi->YuvInputData.UBuffer;
        for ( i=0; i<cpi->YuvInputData.UVHeight; i++ )
        {
            memcpy ( LocalDataPtr, InputDataPtr, cpi->YuvInputData.UVWidth );
            LocalDataPtr += cpi->YuvInputData.UVWidth;
            InputDataPtr += cpi->YuvInputData.UVStride;
        }

        // Now copy over the V data
        LocalDataPtr = &cpi->yuv1ptr[((cpi->YuvInputData.YHeight * cpi->YuvInputData.YWidth) * 5) / 4];
        InputDataPtr = (unsigned char *)cpi->YuvInputData.VBuffer;
        for ( i=0; i<cpi->YuvInputData.UVHeight; i++ )
        {
            memcpy ( LocalDataPtr, InputDataPtr, cpi->YuvInputData.UVWidth );
            LocalDataPtr += cpi->YuvInputData.UVWidth;
            InputDataPtr += cpi->YuvInputData.UVStride;
        }
    }


	if ( cpi->PreProcFilterLevel != 0 )
    {

		// Take a copy of the un-preprocessed frame
#if defined FILE_PSNR 
        memcpy(cpi->yuv0ptr, cpi->yuv1ptr, (cpi->pb.YPlaneSize + (2 * cpi->pb.UVPlaneSize))); 
#endif

#if defined PSNR_ON 
        memcpy(cpi->yuv0ptr, cpi->yuv1ptr, (cpi->pb.YPlaneSize + (2 * cpi->pb.UVPlaneSize))); 
#endif

		// If appropriate reset the proprocessor frame counter.
		if ( ResetPreproc )
			cpi->preproc.frame = 0;

        if ( yuvConfig.YStride < 0 )
        {
            yuvConfig.YBuffer = &cpi->yuv1ptr[(yuvConfig.YHeight - 1) * yuvConfig.YWidth];
            yuvConfig.UBuffer = &cpi->yuv1ptr[yuvConfig.YHeight * yuvConfig.YWidth * 5 / 4 - yuvConfig.YWidth / 2];
            yuvConfig.VBuffer = &cpi->yuv1ptr[yuvConfig.YHeight * yuvConfig.YWidth * 3 / 2 - yuvConfig.YWidth / 2];
            tempFilter ( &cpi->preproc,
                yuvConfig.YBuffer + (yuvConfig.YHeight - 1) * yuvConfig.YStride ,
                yuvConfig.YBuffer + (yuvConfig.YHeight - 1) * yuvConfig.YStride ,
                yuvConfig.YHeight * yuvConfig.YWidth * 3 / 2 , cpi->PreProcFilterLevel);
        }
        else
        {
            yuvConfig.YBuffer = cpi->yuv1ptr;
            yuvConfig.UBuffer = &cpi->yuv1ptr[yuvConfig.YHeight * yuvConfig.YWidth];
            yuvConfig.VBuffer = &cpi->yuv1ptr[yuvConfig.YHeight * yuvConfig.YWidth * 5 / 4];
            tempFilter ( &cpi->preproc, yuvConfig.YBuffer, yuvConfig.YBuffer,
                yuvConfig.YHeight * yuvConfig.YWidth * 3 / 2, cpi->PreProcFilterLevel );
        }
    }

    return;
}


/****************************************************************************
 *
 *  ROUTINE       : EncodeFrameYuv
 *
 *  INPUTS        : CP_INSTANCE *cpi                      : Pointer to encoder instance.
 *                  YUV_INPUT_BUFFER_CONFIG *YuvInputData : Pointer to input frame (YUV).
 *                  unsigned char *OutPutPtr              : Output buffer.
 *                 
 *  OUTPUTS       : unsigned int *is_key                  : Flag whether frame coded
 *                                                          as intra-frame or not.
 *
 *  RETURNS       : UINT32: Number of bytes written to output buffer.
 *
 *  FUNCTION      : Encodes the specified frame creating an output buffer
 *                  containing the compressed bitstream for the frame.
 *
 *  SPECIAL NOTES : The format of the input image is planar YUV 4:2:0.
 *
 ****************************************************************************/
UINT32 CCONV EncodeFrameYuv ( CP_INSTANCE *cpi, YUV_INPUT_BUFFER_CONFIG *YuvInputData, unsigned char *OutPutPtr, unsigned int *is_key )
{
    UINT8 iskey;
    UINT32 ret_val;

    if ( cpi->FrameRateDropCount )
    {
        --cpi->FrameRateDropCount;
        return 0;
    }
    
    cpi->FrameRateDropCount = cpi->FrameRateDropFrames;
    cpi->pb.Configuration.ExpandedFrameWidth  = YuvInputData->YWidth;
    cpi->pb.Configuration.ExpandedFrameHeight = YuvInputData->YHeight;
    cpi->pb.OutputWidth  = YuvInputData->YWidth;
    cpi->pb.OutputHeight = YuvInputData->YHeight;

    if ( cpi->PreProcFilterLevel )
    {
        int OldFrameSize = cpi->YuvInputData.YHeight *  cpi->YuvInputData.YWidth * 3/2;
        int FrameSize = YuvInputData->YHeight * YuvInputData->YWidth * 3/2;

        if ( OldFrameSize != FrameSize )
        {
            if ( !InitPreProc ( &cpi->preproc, FrameSize ) )
            {
                EDeleteFrameInfo ( cpi );
                return FALSE;
            }
        }
    }

    // remember our input buffer (incase we want to do something to it later!)
    memcpy ( &cpi->YuvInputData, YuvInputData, sizeof(YUV_INPUT_BUFFER_CONFIG) );

    cpi->ThisFrameSize = 0;         // Reset the frame size monitor variable

    cpi->DataOutputBuffer = OutPutPtr;
    cpi->pb.DataOutputInPtr = cpi->DataOutputBuffer;

#if defined(_MSC_VER)
	ClearSysState();
#endif

	// Decide whether to allow selective bicubic filtered prediction
	if ( cpi->pb.VpProfile == SIMPLE_PROFILE )
	{
        // NOTE: Use huffman only allowed if using multiple data streams
        cpi->pb.MultiStream          = TRUE;
		cpi->pb.UseHuffman           = TRUE;    
		cpi->pb.UseLoopFilter        = NO_LOOP_FILTER;
		cpi->pb.PredictionFilterMode = BILINEAR_ONLY_PM;	
	}
	else
	{
        // NOTE: Use huffman only allowed if using multiple data streams
		cpi->pb.MultiStream                  = FALSE;
		cpi->pb.UseHuffman                   = FALSE;
		cpi->pb.UseLoopFilter                = LOOP_FILTER_BASIC;
		cpi->pb.PredictionFilterMode         = AUTO_SELECT_PM;	

		// Vp6.2 and later specific
		if ( cpi->pb.Vp3VersionNo > 7 )
		{
			cpi->pb.PredictionFilterVarThresh    = 31;							// Default bicubic variance threshold
			cpi->pb.PredictionFilterAlpha		 = cpi->BaselineAlpha;			// Default Aplha Index for bicubic filter.
		}
		else
		{
			cpi->pb.PredictionFilterVarThresh    = (2 << 5);    // Variance threshold for using bicubic (range 0 to 32) << 5. (note however 0 = no threshold)
			cpi->pb.PredictionFilterAlpha		 = 16;			// Filter Alpha index 32 provides for backwards compatibility with VP61
		}

		// Size of frame influences default limit on motion length for use of bicubic.
		if ( cpi->pb.Configuration.VideoFrameWidth >= 480 )
			cpi->pb.PredictionFilterMvSizeThresh = 4;			// Restrict bicubic to mvs of < +/- (1 << (X-1)) pels. 0 Indicates unrestricted.
		else
			cpi->pb.PredictionFilterMvSizeThresh = 3;			// Restrict bicubic to mvs of < +/- (1 << (X-1)) pels. 0 Indicates unrestricted.

		cpi->pb.UseLoopFilter        = NO_LOOP_FILTER;
		cpi->pb.PredictionFilterMode = BICUBIC_ONLY_PM;	

	}

    // Variables used to track inter vs intra prediction error for mbs that use motion
	cpi->MotionIntraErr = 0;
	cpi->MotionInterErr = 0;
	
    // Set default KF boost
    cpi->KFBoost = 4;

    // 2nd pass datarate control
    if(cpi->pass == 2)
    {
        Pass2Control(cpi);
    }

    // Special case for first frame
    if ( cpi->ThisIsFirstFrame )
    {
		cpi->pb.RefreshGoldenFrame = TRUE;						// KF is also GF update

		// Stats and other first frame initialisation
        ClipBytes = 0;
		cpi->NiAvQi = cpi->Configuration.WorstQuality;

		// Now code the first frame
        CompressFirstFrame ( cpi );
        cpi->ThisIsFirstFrame = FALSE;
        cpi->ThisIsKeyFrame   = FALSE;
    }
	// A key frame explicitly requested by the calling application
    else if ( cpi->ThisIsKeyFrame )
    {
		cpi->pb.RefreshGoldenFrame = TRUE;						// KF is also GF update
        CompressKeyFrame ( cpi );
        cpi->ThisIsKeyFrame = FALSE;
    }
    else
    {
        /* Compress the frame. */
        CompressFrame ( cpi, (unsigned int) cpi->CurrentFrame );
    }


	// Keep a record from which we can calculate the average Q excluding GF updates and key frames
	if ( (cpi->pb.FrameType != BASE_FRAME) && !cpi->pb.RefreshGoldenFrame )
	{
		cpi->NiFrames++;

		// Calculate the average Q for normal inter frames (not key or GFU frames)
		// This is used as a basis for setting active worst quality.
		if ( cpi->NiFrames > 150 )
		{
			cpi->NiTotQi += cpi->pb.quantizer->FrameQIndex;
			cpi->NiAvQi = (cpi->NiTotQi/cpi->NiFrames);
		}
		// Early in the clip ... average the current frame Q value with the default
		// entered by the user as a dampening measure (often there are very easy intro credits).
		else
		{
			cpi->NiTotQi += ((cpi->Configuration.WorstQuality + cpi->pb.quantizer->FrameQIndex + 1) / 2);
			cpi->NiAvQi = (cpi->NiTotQi/cpi->NiFrames);
		}

      // If the average is higher than what was used in the last frame 
      // (after going through the recode loop to keep the frame size within range)
      // then use the last frame value + 1.
      // The +1 is designed to stop Q and hence the data rate, from progressively 
      // falling away during difficult sections.
      if ( cpi->pb.quantizer->FrameQIndex < cpi->NiAvQi )
       cpi->NiAvQi = cpi->pb.quantizer->FrameQIndex + 1;
    }

    // Clip size stats
    ClipBytes += (cpi->ThisFrameSize >> 3);
  
    // Update stats variables. 
    cpi->LastFrameSize = (UINT32)cpi->ThisFrameSize;
    cpi->CurrentFrame++;

	// If we have had a GF update then reset the counter till next one due.
	if ( cpi->pb.RefreshGoldenFrame )
	{
		cpi->FramesTillGfUpdateDue = cpi->GfUpdateInterval;
		cpi->LastGfOrKFrameQ = cpi->pb.quantizer->FrameQIndex;
		cpi->pb.RefreshGoldenFrame = FALSE;
	}

	// Decrement count till next GF update due
	if ( cpi->FramesTillGfUpdateDue > 0 )
		cpi->FramesTillGfUpdateDue--;

    // return whether or not we are a key frame 
    iskey = VP6_GetFrameType ( &cpi->pb );
    if ( iskey == 0 )
        *is_key = 1;
    else
        *is_key = 0;

#if defined(_MSC_VER)
	ClearSysState();
#endif
    if(cpi->pass==1)
    {
        Pass1Output(cpi);
    }
#if defined(_MSC_VER)
    if ( cpi->pb.testMode )
        vp6_appendframe ( &cpi->pb );
#endif
    cpi->GfRecoveryFrame = FALSE;
    cpi->TotalBitsLeftInClip -= cpi->ThisFrameSize ;
    // Set the output bytes buffered count and reset the  buffer input pointer. 
    cpi->pb.DataOutputInPtr = cpi->DataOutputBuffer;
    ret_val = (cpi->ThisFrameSize >> 3);		

    cpi->LastInterError = cpi->InterError;
    cpi->LastIntraError = cpi->IntraError;

//TEMP STATS
// DEBUG Code
if ( FALSE )
{
    FILE  *StatsFilePtr;

    // Open stats file and write out data
    StatsFilePtr = fopen( "buffers.stt", "a" );
    if ( StatsFilePtr )
    {
		fprintf( StatsFilePtr, "%12ld ", (UINT32)cpi->CurrentFrame );
		fprintf( StatsFilePtr, "%12ld ", (cpi->BufferLevel * 100)/cpi->OptimalBufferLevel );
		fprintf( StatsFilePtr, "%12ld ", (100 * cpi->BytesOffTarget / (cpi->TotalByteCount * 8)));
		fprintf( StatsFilePtr, "%12ld ", cpi->NiAvQi );
		fprintf( StatsFilePtr, "%12ld ", cpi->Configuration.ActiveWorstQuality );
		fprintf( StatsFilePtr, "%12ld\n", ((cpi->ThisFrameSize * 100)/cpi->ThisFrameTarget) );
        fclose ( StatsFilePtr );
	}
}

#if defined MEASURE_SECTION_COSTS
	{
		UINT32 i;

		// Temps Stats for section data rate analysis
		for ( i = 0; i < 10; i++ )
		{
			ClipSectionBits[i] += (Sectionbits[i] / 256);
			Sectionbits[i] = 0;
		}
	}
#endif

    return ret_val;
}

/****************************************************************************
 *
 *  ROUTINE       : StopEncoder
 *
 *  INPUTS        : None.
 *
 *  OUTPUTS       : CP_INSTANCE **cpi : Pointer to pointer to encoder instance.
 *
 *  RETURNS       : BOOL: Always TRUE.
 *
 *  FUNCTION      : Stops the encoder and de-allocates memory used for
 *                  encoder data structures.
 *
 *  SPECIAL NOTES : Also include lots of debug/test code for outputting
 *                  timing and run statistics to file.
 *
 ****************************************************************************/
BOOL CCONV StopEncoder ( CP_INSTANCE **cpi )
{
#ifdef TIMING
    ITime2 = timeGetTime();
    ITotalTime = ITime2-ITime1;
    {
        FILE *fp = fopen( "d:\\Times.txt", "at" );
        fprintf ( fp, "StopEncoder: %d\n", ITime2 );
        fprintf ( fp, "The total time spent is %d\n", ITotalTime );
        fprintf ( fp, "------------------------------------\n" );
        fclose ( fp );
    }
#endif

#if defined MEASURE_SECTION_COSTS
    // DEBUG Code
    if ( TRUE && *cpi )
    {
		UINT32 i;
		UINT32 Sum = 0;
        FILE  *StatsFilePtr;
		
		for ( i = 0; i < 6; i++ )
		{
			Sum += ClipSectionBits[i];
		}

		if ( Sum )
		{
			// Open stats file and write out data
			StatsFilePtr = fopen( "Section_bits.stt", "a" );
			if ( StatsFilePtr )
			{
				fprintf( StatsFilePtr, "Header %4ld  ", ((ClipSectionBits[0]+(Sum/200)) * 100)/Sum );
				fprintf( StatsFilePtr, "Mode %4ld  ", ((ClipSectionBits[1]+(Sum/200)) * 100)/Sum );
				fprintf( StatsFilePtr, "Mv %4ld  ", ((ClipSectionBits[2]+(Sum/200)) * 100)/Sum );
				fprintf( StatsFilePtr, "Context %4ld  ", ((ClipSectionBits[3]+(Sum/200)) * 100)/Sum );
				fprintf( StatsFilePtr, "DC %4ld  ", ((ClipSectionBits[4]+(Sum/200)) * 100)/Sum );
				fprintf( StatsFilePtr, "AC %4ld  ", ((ClipSectionBits[5]+(Sum/200)) * 100)/Sum );
				fprintf( StatsFilePtr, "\n" );
				fclose ( StatsFilePtr );
			}
		}
	}
#endif

#if defined PSNR_ON
	if ( *cpi )
    {
        // TEST Code
        if ( (*cpi)->CurrentFrame && !(*cpi)->AllowSpatialResampling )
        {
            FILE *StatsFilePtr;
            UINT32 FrameCount = ((UINT32)(*cpi)->CurrentFrame) -1;
            double FrameSize = 1.5 * (*cpi)->pb.YPlaneSize;
            double OverallPSNR = 10.0 * log10((255.0 * 255.0 * FrameSize * (*cpi)->CurrentFrame) / (*cpi)->TotalSqError);

            // Open stats file and write out data
            StatsFilePtr = fopen( "psnr.stt", "a" );
            if ( StatsFilePtr )
            {
				// Fudge to deal with 29.97 fps material
				if ( (*cpi)->Configuration.OutputFrameRate == 30 )
				{
					fprintf( StatsFilePtr, "%6.3f %10.2f %6.3f\n",
							 (*cpi)->TotPsnr / (double)(FrameCount),
							 (((double)ClipBytes/1024) * 8 * 29.97) / ((UINT32)(*cpi)->CurrentFrame - 1) ,
                             OverallPSNR);
				}
				else
				{
					fprintf( StatsFilePtr, "%6.3f %10.2f %6.3f\n",
							 (*cpi)->TotPsnr / (double)(FrameCount),
							 (((double)ClipBytes/1024) * 8 * (*cpi)->Configuration.OutputFrameRate) / ((UINT32)(*cpi)->CurrentFrame - 1),
                             OverallPSNR);
				}

				fclose( StatsFilePtr );
            }

        }
	}
#endif


#if 0
    // DEBUG Code
    if ( FALSE )
    {
		UINT32 i;
        FILE  *StatsFilePtr;
	
        // Open stats file and write out data
        StatsFilePtr = fopen( "tmp.stt", "a" );
        if ( StatsFilePtr )
        {
			fprintf( StatsFilePtr, "%12ld %12ld\n", BcCount, TotTokens );
            fclose ( StatsFilePtr );
		}

        StatsFilePtr = fopen( "tmp2.stt", "a" );
        if ( StatsFilePtr  && NzCount[1][0] )
        {
			memcpy ( (*cpi)->FrameNzCount, NzCount, sizeof((*cpi)->FrameNzCount) );
			PredictScanOrder( (*cpi) );

			for ( i=0; i<64; i++ )
			{
				fprintf ( StatsFilePtr, "%2ld,", (*cpi)->NewScanOrderBands[i] );
				if ( (i%8) == 7 )
					fprintf ( StatsFilePtr, "\n" );
			}
			fprintf ( StatsFilePtr, "\n" );
            fclose ( StatsFilePtr );
		}

		if ( scanupdates[1][0] > 0 )
		{
			FILE *StatsFilePtr;
			UINT32 i, Sum, Sum2, Prob;

			StatsFilePtr = fopen( "scanupdates.stt", "a" );
			if ( StatsFilePtr  )
			{
				for ( i=0; i<64; i++ )
				{
					Sum = scanupdates[i][0] + scanupdates[i][1];
					Sum2 = scanupdates[i][0];

					if ( Sum > 0 )
					{
						Prob = (Sum2 * 255)/Sum;
						if ( Prob == 0 )
							Prob = 1;
						fprintf( StatsFilePtr, "%3ld, ", Prob );
					}
					else
						fprintf( StatsFilePtr, "%3ld, ", 255 );

					if ( (i % 8) == 7 )
						fprintf( StatsFilePtr, "\n");
				}
				fprintf ( StatsFilePtr, "\n" );
	            fclose ( StatsFilePtr );
			}
		}
	}
#endif

    if ( *cpi )
    {
#if defined FILE_PSNR 
        // TEST Code
        if ( (*cpi)->CurrentFrame && !(*cpi)->AllowSpatialResampling )
        {
            FILE *StatsFilePtr;
            UINT32 FrameCount = ((UINT32)(*cpi)->CurrentFrame) -1;
            double PSNR = (*cpi)->TotPsnr / (double)(FrameCount);
            double KBS = ((double)ClipBytes * 8 * (*cpi)->Configuration.OutputFrameRate ) / ((double) FrameCount);
            double LGKBS = log10(KBS);
            double FrameSize = 1.5 * cpi->pb.YPlaneSize;
            double OverallPSNR = 10.0 * log10((255.0 * 255.0 * FrameSize * cpi->CurrentFrame) / (double)Total);

            // Open stats file and write out data
            StatsFilePtr = fopen( "psnr.stt", "a" );
            if ( StatsFilePtr )
            {

				// Fudge to deal with 29.97 fps material
				if ( (*cpi)->Configuration.OutputFrameRate == 30 )
				{
					fprintf( StatsFilePtr, "%6.3f %10.2f %10.6f\n",
							 PSNR,
							 (((double)ClipBytes/1024) * 8 * 29.97) / (FrameCount),

							 PSNR/
							 log10((((double)ClipBytes/1024) * 8 * 29.97) / (FrameCount))
                             );
				}
				else
				{
					fprintf( StatsFilePtr, "%6.3f %10.2f %10.6f\n",
							 PSNR,
							 KBS/1024,
                             PSNR / LGKBS );
				}

				fclose( StatsFilePtr );
            }

        }
#endif


        AvgStats ( &(*cpi)->fpmss);
        if((*cpi)->fpmss.count)
            OutputStats((*cpi)->ss,&(*cpi)->fpmss);

        if((*cpi)->fs)
            fclose((*cpi)->fs);

        if((*cpi)->ss)
            fclose((*cpi)->ss);


        VP6_DeleteFragmentInfo ( &(*cpi)->pb );
        VP6_DeleteFrameInfo ( &(*cpi)->pb );
        EDeleteFragmentInfo ( (*cpi) );
        EDeleteFrameInfo ( (*cpi) );
        VP6_DeleteQuantizer ( &(*cpi)->pb.quantizer );
        DeletePostProcInstance ( &(*cpi)->pb.postproc );
        DeleteCPInstance ( cpi );
    }

 // test output code for filter taps
	if(0)
	{
		UINT32 i,j,k;
        FILE  *StatsFilePtr;
		double dval;
		double aval = -0.05;
		int y1,y2,y3,y4;
		double d2, d3;
		int sum;

        // Open stats file and write out data
        StatsFilePtr = fopen( "filters.stt", "a" );
        if ( StatsFilePtr )
        {
			fprintf( StatsFilePtr, " **** \n" );
			for ( i = 0; i < 32; i++ )
			{

				fprintf( StatsFilePtr, "    {\n" );
				dval = 0.0;
				for ( j = 0; j < 8; j++ )
				{
					d2 = dval * dval;
					d3 = dval * dval * dval;

					y1 = (int)floor(0.5 + (		((aval*dval)  -		(2.0*aval*d2)       +	(aval*d3)) * 128));
					y2 = (int)floor(0.5 + (		(1.0		  -		((aval+3.0)*d2)     +	((aval+2.0)*d3)) * 128));
					y3 = (int)floor(0.5 + (		(-(aval*dval) +		((2.0*aval+3.0)*d2) -	((aval+2.0)*d3)) * 128));
					y4 = (int)floor(0.5 + (		(					(aval*d2)          -	(aval*d3)) * 128));

					sum = y1 + y2 + y3 + y4;
					if ( sum < 128 )
					{
						if ( sum < 127 )
						{
							y2++;
							y3++;
						}
						else
						{
							if ( y2 >= y3 )
								y2++;
							else
								y3++;
						}
					}
					else if ( sum > 128 )
					{
						if ( sum > 129 )
						{
							y2--;
							y3--;
						}
						else
						{
							if ( y2 >= y3 )
								y2--;
							else
								y3--;
						}
					}
					fprintf( StatsFilePtr, "        { ");
					for(k=0;k<8;k++)
						fprintf(StatsFilePtr,"%3ld,",y1);
					fprintf( StatsFilePtr, "  ");
					for(k=0;k<8;k++)
						fprintf(StatsFilePtr,"%3ld,",y2);
					fprintf( StatsFilePtr, "  ");
					for(k=0;k<8;k++)
						fprintf(StatsFilePtr,"%3ld,",y3);
					fprintf( StatsFilePtr, "  ");
					for(k=0;k<8;k++)
						fprintf(StatsFilePtr,"%3ld,",y4);
					fprintf( StatsFilePtr, " }");


					if (y1 + y2 + y3 + y4 != 128)
					{
						fprintf( StatsFilePtr, " **** %ld %ld", (y1 + y2 + y3 + y4), sum );
					}

					fprintf( StatsFilePtr, "\n" );

					dval += 0.125;
				}
				aval -= 0.05;
				fprintf( StatsFilePtr, "    },\n" );
				fprintf( StatsFilePtr, "\n" );
			}


			fprintf( StatsFilePtr, "%ld\n", i );
			fclose( StatsFilePtr );
        }
	}

    return TRUE;
}

/****************************************************************************
 *
 *  ROUTINE       : VPGetState
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *                  void *ret        : Pointer to COMPRESSOR_STATE object
 *                                     representing encoder state.
 *  OUTPUTS       : None.    
 *                      
 *  RETURNS       : UINT32: Size of the returned COMPRESSOR_STATE object.
 *
 *  FUNCTION      : Fills in the supplied COMPRESSOR_STATE object with
 *                  details of the compressor state.
 *
 *  SPECIAL NOTES : The buffer supplied by the caller (ret) should
 *                  be large enough to hold a COMPRESSOR_STATE object.
 *
 ****************************************************************************/
UINT32 CCONV VPGetState ( CP_INSTANCE *cpi, void *ret )
{
    INT32 i;
    COMPRESSOR_STATE *cs = (COMPRESSOR_STATE *) ret;

    if ( !ret )
        return sizeof ( COMPRESSOR_STATE );

    for ( i=0; i<KEY_FRAME_CONTEXT; i++ )
    {
        cs->PriorKeyFrameSize[i]     = cpi->PriorKeyFrameSize[i];
        cs->PriorKeyFrameDistance[i] = cpi->PriorKeyFrameDistance[i];
    }

    cs->CurrentFrame        = cpi->CurrentFrame;
    cs->LastFrameSize       = cpi->LastFrameSize;
    cs->DropCount           = cpi->DropCount;
    cs->KeyFrameCount       = cpi->KeyFrameCount;
    cs->TotKeyFrameBytes    = cpi->TotKeyFrameBytes;
    cs->LastKeyFrameSize    = cpi->LastKeyFrameSize;
    cs->LastKeyFrame        = cpi->LastKeyFrame;
    cs->TotalByteCount      = cpi->TotalByteCount;
    cs->ActiveMaxQ          = cpi->Configuration.ActiveWorstQuality;
    cs->BpbCorrectionFactor = cpi->BpbCorrectionFactor;

    return sizeof ( COMPRESSOR_STATE );
}

/****************************************************************************
 *
 *  ROUTINE       : VPSetState
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *                  void *arg        : Pointer to COMPRESSOR_STATE object
 *                                     representing encoder state.
 *
 *  OUTPUTS       : None.    
 *                      
 *  RETURNS       : void
 *
 *  FUNCTION      : Sets the compressor state to that specified by the
 *                  supplied COMPRESSOR_STATE object.
 *
 *  SPECIAL NOTES : arg should point to the COMPRESSOR_STATE object that
 *                  contains the required state of the compressor.
 *
 ****************************************************************************/
void CCONV VPSetState ( CP_INSTANCE *cpi, void *arg )
{
    INT32 i;
    COMPRESSOR_STATE *cs = (COMPRESSOR_STATE *) arg;

    for ( i=0; i<KEY_FRAME_CONTEXT; i++ )
    {
        cpi->PriorKeyFrameSize[i]     = cs->PriorKeyFrameSize[i];
        cpi->PriorKeyFrameDistance[i] = cs->PriorKeyFrameDistance[i];
    }

    cpi->CurrentFrame        = cs->CurrentFrame;
    cpi->LastFrameSize       = cs->LastFrameSize;

    cpi->DropCount           = cs->DropCount;
    cpi->KeyFrameCount       = cs->KeyFrameCount;
    cpi->TotKeyFrameBytes    = cs->TotKeyFrameBytes;
    cpi->LastKeyFrameSize    = cs->LastKeyFrameSize;
    cpi->LastKeyFrame        = cs->LastKeyFrame;
    cpi->TotalByteCount      = cs->TotalByteCount;
    cpi->BpbCorrectionFactor = cs->BpbCorrectionFactor;
    cpi->Configuration.ActiveWorstQuality = cs->ActiveMaxQ;
}

/****************************************************************************
 *
 *  ROUTINE       : VPGetPB
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : int: Pointer to the compressor's decoder object (cast to int)
 *
 *  FUNCTION      : Returns pointer to the compressor's decoder object as
 *                  an int.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
int CCONV VPGetPB ( CP_INSTANCE *cpi )
{
    return (int) &cpi->pb;
}
