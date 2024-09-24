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
#include "twopass.h"
#include <math.h>
/****************************************************************************
*  Imports
****************************************************************************/
extern const UINT32 GfuDataRateBoost[64];
extern const UINT32 GfuMotionCorrection[32];
extern const UINT32 GfUsageCorrection[64];

/****************************************************************************
 *
 *  ROUTINE       : ZeroStats
 *
 *  INPUTS        : 
 *                  FIRSTPASS_STATS *stats  Stats to empty the accumulator of
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : 
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void ZeroStats( FIRSTPASS_STATS *section)
{
    section->count = 0;

    section->MotionSpeed = 0 ;
    section->VarianceX = 0 ;
    section->VarianceY = 0 ;
    section->PercentGolden = 0;
    section->PercentMotionY = 0 ;
    section->PercentMotion = 0 ;
    section->PercentNewMotion = 0 ;
    section->MeanInterError = 0 ;
    section->MeanIntraError = 0 ;
    section->BitsPerMacroblock = 0 ;
    section->SqBitsPerMacroblock = 0 ;
    section->PSNR = 0 ;
    section->isGolden = 0;
    section->isKey = 0;

}
/****************************************************************************
 *
 *  ROUTINE       : AccumulateStats
 *
 *  INPUTS        : FIRSTPASS_STATS *section stats to accumulate into
 *                  FIRSTPASS_STATS *stats   Stats to add to accumulated values
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Accumulates firstpass statistics
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void AccumulateStats(FIRSTPASS_STATS *section, FIRSTPASS_STATS *frame)
{
    section->count ++;

    section->MotionSpeed += frame->MotionSpeed;
    section->VarianceX += frame->VarianceX;
    section->VarianceY += frame->VarianceY;
    section->PercentGolden += frame->PercentGolden;
    section->PercentMotionY += frame->PercentMotionY;
    section->PercentMotion += frame->PercentMotion;
    section->PercentNewMotion += frame->PercentNewMotion;
    section->MeanInterError += frame->MeanInterError;
    section->MeanIntraError += frame->MeanIntraError;
    section->BitsPerMacroblock += frame->BitsPerMacroblock;
    section->SqBitsPerMacroblock += frame->SqBitsPerMacroblock;
    section->PSNR += frame->PSNR;
    section->isGolden += frame->isGolden;
    section->isKey += frame->isKey;
}
/****************************************************************************
 *
 *  ROUTINE       : AvgStats
 *
 *  INPUTS        : 
 *                  FIRSTPASS_STATS *stats  Stats to convert to averages using count
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : 
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void AvgStats ( FIRSTPASS_STATS *section)
{
    if(!section->count)
        return;

    section->MotionSpeed /= section->count;
    section->VarianceX /= section->count;
    section->VarianceY /= section->count;
    section->PercentGolden /= section->count;
    section->PercentMotionY /= section->count;
    section->PercentMotion /= section->count;
    section->PercentNewMotion /= section->count;
    section->MeanInterError /= section->count;
    section->MeanIntraError /= section->count;
    section->BitsPerMacroblock /= section->count;
    section->SqBitsPerMacroblock /= section->count;
    section->PSNR /= section->count;
}
/****************************************************************************
 *
 *  ROUTINE       : OutputStats
 *
 *  INPUTS        : FILE *F                 File to output the stats to
 *                  FIRSTPASS_STATS *stats  Stats to fill in
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : 
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void OutputStats( FILE *f, FIRSTPASS_STATS *stats)
{
    fprintf(f,
        "%8d %8d %8d %8d %12.04f %12.04f %12.04f %12.04f %12.04f %12.04f %12.04f %12.04f %12.04f %12.04f \n",
        stats->frame,
        stats->count,
        stats->isKey,
        stats->isGolden,
        stats->BitsPerMacroblock,
        stats->SqBitsPerMacroblock,
        stats->MeanInterError,
        stats->MeanIntraError,
        stats->MotionSpeed,
        stats->VarianceX,
        stats->VarianceY,
        stats->PercentMotion,
        stats->PercentNewMotion,
        stats->PercentGolden);
}
/****************************************************************************
 *
 *  ROUTINE       : InputStats
 *
 *  INPUTS        : FILE *F                 File to read the stats in
 *                  FIRSTPASS_STATS *stats  Stats to fill in
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : 
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void InputStats( FILE *f, FIRSTPASS_STATS *stats)
{
    fscanf(f,
        "%d %d %d %d %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg \n",
        &stats->frame,
        &stats->count,
        &stats->isKey,
        &stats->isGolden,
        &stats->BitsPerMacroblock,
        &stats->SqBitsPerMacroblock,
        &stats->MeanInterError,
        &stats->MeanIntraError,
        &stats->MotionSpeed,
        &stats->VarianceX,
        &stats->VarianceY,
        &stats->PercentMotion,
        &stats->PercentNewMotion,
        &stats->PercentGolden);
}
/****************************************************************************
 *
 *  ROUTINE       : Pass2Initialize
 *
 *  INPUTS        : CP_INSTANCE *cpi            : Pointer to encoder instance.
 *                  COMP_CONFIG_VP6 *CompConfig : Encoder configuration.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Initialize 1st or 2nd pass of the compressor
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void CCONV Pass2Initialize ( CP_INSTANCE *cpi, COMP_CONFIG_VP6 *CompConfig )
{
   if(cpi->pass == 2)
    {

        int    actualMBS =(cpi->pb.MBRows - (BORDER_MBS*2)) * (cpi->pb.MBCols - (BORDER_MBS*2));
        double fpBitRate;                        // first pass bitrate
        double target;                           // target bitrate
        double NewQ;
        double Sigma;
        double RoomForVariation;
        double tmp;
        char dummy[1024];
        ClearSysState();
        cpi->fs = fopen(CompConfig->FirstPassFile,"r");
        strncpy(dummy,CompConfig->FirstPassFile,1024);
        strcat(dummy,".sst");
        cpi->ss = fopen(dummy,"r");

        fgets(dummy,1024,cpi->fs);
        fgets(dummy,1024,cpi->ss);

        InputStats(cpi->ss,&cpi->fpmss);

        tmp = cpi->fpmss.SqBitsPerMacroblock - cpi->fpmss.BitsPerMacroblock*cpi->fpmss.BitsPerMacroblock;
        Sigma = sqrt(tmp);
        RoomForVariation = (Sigma+2) / 3.5;       // 5 q steps above
        RoomForVariation = (Sigma+2) / 15;       // 5 q steps above
		RoomForVariation = 1;

        fpBitRate = cpi->fpmss.BitsPerMacroblock * actualMBS * cpi->Configuration.OutputFrameRate;
        target = (double) cpi->Configuration.TargetBandwidth;

        NewQ = (INT32)  FIRSTPASS_Q  -  ( RoomForVariation + .5 + log(fpBitRate/target) / log(1.04));
        if(NewQ < cpi->Configuration.WorstQuality )
            NewQ = cpi->Configuration.WorstQuality;
        
        if(NewQ > cpi->Configuration.ActiveBestQuality)
            NewQ = cpi->Configuration.ActiveBestQuality;

        if(NewQ > 50) 
            NewQ = 50;

        
        cpi->PassedInWorstQ = cpi->Configuration.WorstQuality;
        cpi->Configuration.WorstQuality = (INT32) NewQ;
        cpi->CalculatedWorstQ = (INT32) NewQ;
        
        cpi->Configuration.ActiveWorstQuality = cpi->Configuration.WorstQuality;

        cpi->TotalBitsLeftInClip = 1.0 * cpi->ActualTargetBitRate * cpi->fpmss.count / cpi->Configuration.OutputFrameRate;
        cpi->FramesYetToEncode = cpi->fpmss.count;
        //cpi->TotalBitsPerMB = cpi->fpmss.BitsPerMacroblock * cpi->fpmss.count;
        cpi->TotalBitsPerMB = cpi->fpmss.MeanInterError * cpi->fpmss.count;
   
   }
    else if (cpi->pass == 1)
    {
        char dummy[1024];
        ZeroStats( &cpi->fpmss);

        cpi->fs = fopen(CompConfig->FirstPassFile,"w");
        fprintf(cpi->fs,
            "%8s %8s %8s %8s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s \n",
            "","#","key","golden","bits/mb","sq bits/mb","Inter","Intra","Motion","VarX","VarY",
            "%Motion","%NewMotion","%Golden");

        strncpy(dummy,CompConfig->FirstPassFile,1024);
        strcat(dummy,".sst");
        cpi->ss = fopen(dummy,"w");
        fprintf(cpi->ss,
            "%8s %8s %8s %8s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s \n",
            "","#","key","golden","bits/mb","sq bits/mb","Inter","Intra","Motion","VarX","VarY",
            "%Motion","%NewMotion","%Golden");


    }

}
/****************************************************************************
 *
 *  ROUTINE       : Pass2Control
 *
 *  INPUTS        : CP_INSTANCE *cpi                      : Pointer to encoder instance.
 *                 
 *  OUTPUTS       : unsigned int *is_key                  : Flag whether frame coded
 *                                                          as intra-frame or not.
 *
 *  RETURNS       : 
 *
 *  FUNCTION      : Determines Section info, and does datarate control 
 *                  that is only possible in 2nd pass
 *
 *  SPECIAL NOTES : 
 *
 ****************************************************************************/
void CCONV Pass2Control( CP_INSTANCE *cpi)
{
    INT32 i;
    FIRSTPASS_STATS sectionStats;
    FIRSTPASS_STATS thisFrame;
    FIRSTPASS_STATS nextFrame;
    FIRSTPASS_STATS lastFrame;
    double NewBitsPerMB;

    double total=0;
    double avg = 0;
    int    actualMBS ;

    fpos_t pos1,pos2;

    InputStats(cpi->fs,&thisFrame);
    fgetpos(cpi->fs,&pos1);

    //NewBitsPerMB = cpi->TotalBitsPerMB  -  thisFrame.BitsPerMacroblock;
    NewBitsPerMB = cpi->TotalBitsPerMB  -  thisFrame.MeanInterError;
    // keyframe and section processing !
    if( cpi->FramesToKey == 0 )
    {
        
        cpi->KFForced = cpi->NextKFForced;
        cpi->NextKFForced = 0;


        cpi->ThisIsKeyFrame = TRUE; 
        ZeroStats( &sectionStats);
        //AccumulateStats(&sectionStats, &thisFrame);

        cpi->FramesToKey = 1;
        InputStats(cpi->fs,&nextFrame);

        // find the next keyframe
        while(!feof(cpi->fs))
        {
            memcpy(&lastFrame,&thisFrame,sizeof(thisFrame));
            memcpy(&thisFrame,&nextFrame,sizeof(thisFrame));

            InputStats(cpi->fs,&nextFrame);

            //  mark a key if first pass marked it a keyframe and its within minimum distance to keyframe numbers or 
            //  the next frame gets a big benefit from it being a keyframe

            if( (  thisFrame.isKey 
                && ( cpi->FramesToKey > cpi->MinimumDistanceToKeyFrame 
                     && (   fabs(lastFrame.MeanInterError - thisFrame.MeanInterError) / thisFrame.MeanInterError > .40
                         || fabs(lastFrame.MeanIntraError - thisFrame.MeanIntraError) / thisFrame.MeanIntraError > .40
                         || thisFrame.MeanIntraError * 5 < thisFrame.MeanInterError * 6
                        )
                  || nextFrame.MeanIntraError > nextFrame.MeanInterError  + 2000 
                  )
                     
                ) 
              )
            {
                break;
            }

            cpi->FramesToKey ++;

            // since we don't have a key frame within the next two forcekeyframeevery intervals 
            // set the next keyframe to be forcekeyframe every
            if(cpi->FramesToKey > 2 * cpi->ForceKeyFrameEvery)
            {
                cpi->FramesToKey = cpi->ForceKeyFrameEvery;
                cpi->NextKFForced = 1;
                break;
            }
        }
        if(feof(cpi->fs))
            cpi->FramesToKey ++;

        // distance to keyframe is not 2 times our max distance but it is greater than our max distance
        // since we need a keyframe put it in the center between this key frame and the next 
        if( cpi->FramesToKey > cpi->ForceKeyFrameEvery )
        {
            cpi->FramesToKey /= 2;
            cpi->NextKFForced = 1;
        }

        fgetpos(cpi->fs,&pos2);
        pos2-=pos1;
        fseek(cpi->fs,(INT32) -pos2,SEEK_CUR);


        // determine how big to make this keyframe based on how well the subsequent frames use inter blocks
        total = 1.0;
        for(i = 0 ;i < 4 && i < cpi->FramesToKey ; i++)
        {
            InputStats(cpi->fs,&nextFrame);
            total *= ( nextFrame.MeanIntraError - nextFrame.MeanInterError ) / nextFrame.MeanIntraError;
            avg += total * ( nextFrame.MeanIntraError - nextFrame.MeanInterError ) ;

            // this break out is to insure we handle the situation that is really different from 
            // our last frame but similar to our next frame doesn't get counted in our metric, which 
            // is trying to estimate the average amount of data retained from the keyframe. 
            if(total < .1 || nextFrame.MeanIntraError < 200) 
                break;

        }

        cpi->KFBoost = (INT32 ) avg / 180 ;//(16* total);//16 * avg / 6 );


        if(cpi->FramesToKey < 4 )//&& cpi->BufferLevel < cpi->OptimalBufferLevel / 2)
            cpi->KFBoost = 0;

        fgetpos(cpi->fs,&pos2);
        pos2-=pos1;
        fseek(cpi->fs,(INT32) -pos2,SEEK_CUR);

        // read first pass file up until next keyframe and generate avg section stats.
        total = 1.0;
        for(i = 0 ;i < cpi->FramesToKey-1  ; i++)
        {
            InputStats(cpi->fs,&thisFrame);
            AccumulateStats(&sectionStats, &thisFrame);
        }
        AvgStats(&sectionStats);
        fgetpos(cpi->fs,&pos2);
        pos2-=pos1;
        fseek(cpi->fs,(INT32) -pos2,SEEK_CUR);


        actualMBS = (cpi->pb.MBRows - (BORDER_MBS*2)) * (cpi->pb.MBCols - (BORDER_MBS*2));

        if(cpi->TwoPassVBREnabled)
        // determine bitrate to shoot for for this section 
        {
            //double SectionBitsPerMB = sectionStats.BitsPerMacroblock * sectionStats.count;
            //double Pctg = SectionBitsPerMB / cpi->TotalBitsPerMB ; 
            double SectionErrorPerMB = sectionStats.MeanInterError * sectionStats.count;
            double Pctg = SectionErrorPerMB / cpi->TotalBitsPerMB; 
            double DesiredSectionSize = cpi->TotalBitsLeftInClip * Pctg;
            double DesiredSectionBitRate = cpi->Configuration.OutputFrameRate * DesiredSectionSize / sectionStats.count;

            if(sectionStats.count < 2) 
                DesiredSectionBitRate = cpi->ActualTargetBitRate ;

            if(cpi->TwoPassVBRBias)
            {
                DesiredSectionBitRate = cpi->ActualTargetBitRate * (100 - cpi->TwoPassVBRBias) / 100  + DesiredSectionBitRate * cpi->TwoPassVBRBias / 100  ;
            }

            if(DesiredSectionBitRate < cpi->ActualTargetBitRate * cpi->TwoPassVBRMinSection / 100 )
                DesiredSectionBitRate = cpi->ActualTargetBitRate * cpi->TwoPassVBRMinSection /100 ;

            if(DesiredSectionBitRate > (double) cpi->ActualTargetBitRate * cpi->TwoPassVBRMaxSection / 100 )
                DesiredSectionBitRate = cpi->ActualTargetBitRate * cpi->TwoPassVBRMaxSection / 100 ;


            cpi->Configuration.TargetBandwidth = (INT32) DesiredSectionBitRate; 
            cpi->InterFrameTarget =  (INT32)((cpi->Configuration.TargetBandwidth -
                ((cpi->KeyFrameDataTarget * cpi->Configuration.OutputFrameRate)/cpi->KeyFrameFrequency)) / cpi->Configuration.OutputFrameRate);

            cpi->PerFrameBandwidth = (cpi->Configuration.TargetBandwidth / cpi->Configuration.OutputFrameRate);

			if(0)
            {
        		FILE *sectionstats = fopen("section.stt","a");
                fprintf(sectionstats,"Frame : %8d Count :%4d sq bits/mb:%8.3f BitsPerMB:%8.3f  BitRate: %8d Q:%3d s:%8d buffer:%8d max:%8d \n ",
                    - 1 + (INT32) cpi->CurrentFrame , sectionStats.count, sectionStats.SqBitsPerMacroblock, sectionStats.BitsPerMacroblock, 
                    cpi->Configuration.TargetBandwidth / 1024, cpi->Configuration.WorstQuality, cpi->SizeStep, cpi->BufferLevel ,cpi->MaxBufferLevel);
                fclose(sectionstats);
            }
        }
        else
        // determine q to use for this section
        {
            double SectionErrorPerMB = sectionStats.MeanInterError * sectionStats.count;
            double Pctg = SectionErrorPerMB / cpi->TotalBitsPerMB; 
            double DesiredSectionSize = cpi->TotalBitsLeftInClip * Pctg;
            double DesiredSectionBitRate = cpi->Configuration.OutputFrameRate * DesiredSectionSize / sectionStats.count;
            double target;                           // target bitrate
            double NewQ;
            double RoomForVariation=3;
            double FirstPassBitRate = sectionStats.BitsPerMacroblock * actualMBS * cpi->Configuration.OutputFrameRate;

            if(sectionStats.count < 2) 
                DesiredSectionBitRate = cpi->ActualTargetBitRate ;

            if(cpi->TwoPassVBRBias)
            {
                DesiredSectionBitRate = cpi->ActualTargetBitRate * (100 - cpi->TwoPassVBRBias) / 100  + DesiredSectionBitRate * cpi->TwoPassVBRBias / 100  ;
            }

            if(DesiredSectionBitRate < cpi->ActualTargetBitRate * cpi->TwoPassVBRMinSection / 100 )
                DesiredSectionBitRate = cpi->ActualTargetBitRate * cpi->TwoPassVBRMinSection /100 ;

            if(DesiredSectionBitRate > (double) cpi->ActualTargetBitRate * cpi->TwoPassVBRMaxSection / 100 )
                DesiredSectionBitRate = cpi->ActualTargetBitRate * cpi->TwoPassVBRMaxSection / 100 ;



            // Clamp the Section Datarate between what will fill up the buffer and what will empty it to .25 of the optimal
            {
                double ActualPerFrameBandWidth = cpi->ActualTargetBitRate / cpi->Configuration.OutputFrameRate;
                double UnusedSectionEndBufferLevel = cpi->BufferLevel + (((cpi->MaxAllowedDatarate * ActualPerFrameBandWidth) / 100) * sectionStats.count);
                double QuarterOptimalBufferLevel = cpi->OptimalBufferLevel / 4.0;
                double MaxBitRate = cpi->Configuration.OutputFrameRate * (UnusedSectionEndBufferLevel - QuarterOptimalBufferLevel) / (sectionStats.count + cpi->KFBoost / 16);
                double MinBitRate = cpi->Configuration.OutputFrameRate * (UnusedSectionEndBufferLevel - cpi->MaxBufferLevel ) / (sectionStats.count + cpi->KFBoost / 16);
                if( MaxBitRate < cpi->ActualTargetBitRate / 3) 
                    MaxBitRate = cpi->ActualTargetBitRate / 3;
                if( MinBitRate < cpi->ActualTargetBitRate / 3) 
                    MinBitRate = cpi->ActualTargetBitRate / 3;
                
                if(DesiredSectionBitRate > MaxBitRate)
                    DesiredSectionBitRate = MaxBitRate;

                if(DesiredSectionBitRate < MinBitRate)
                    DesiredSectionBitRate = MinBitRate;

                cpi->Configuration.TargetBandwidth = (INT32) DesiredSectionBitRate; 


                cpi->InterFrameTarget =  (INT32)((cpi->Configuration.TargetBandwidth -
                    ((cpi->KeyFrameDataTarget * cpi->Configuration.OutputFrameRate)/cpi->KeyFrameFrequency)) / cpi->Configuration.OutputFrameRate);


            }

            target = (double) cpi->Configuration.TargetBandwidth;

            // if q is worse than we estimated for the entire clip use it ( this must be a tough section )!! 
            //   otherwise use the one we estimated.
            NewQ = (INT32)  FIRSTPASS_Q  -  ( .5 + log(FirstPassBitRate/target) / log(1.040));
            if( NewQ < cpi->CalculatedWorstQ )
            {
                if(NewQ < cpi->PassedInWorstQ)
                    NewQ = cpi->PassedInWorstQ;

                cpi->Configuration.ActiveWorstQuality = (INT32) NewQ;
                cpi->Configuration.WorstQuality = (INT32) NewQ;
            }
            else
            {
                cpi->Configuration.ActiveWorstQuality = cpi->CalculatedWorstQ;
                cpi->Configuration.WorstQuality = cpi->CalculatedWorstQ;
            }

			if(0)
            {
                FILE *sectionstats = fopen("section.stt","a");
                fprintf(sectionstats,"Frame : %8d Count :%4d sq bits/mb:%8.3f BitsPerMB:%8.3f  BitRate: %8d Q:%3d s:%8d buffer:%8d max:%8d mdr %d %d \n ",
                    - 1 + (INT32) cpi->CurrentFrame , sectionStats.count, sectionStats.SqBitsPerMacroblock, sectionStats.BitsPerMacroblock, 
                    cpi->Configuration.TargetBandwidth / 1024, cpi->Configuration.WorstQuality, cpi->SizeStep,cpi->BufferLevel ,
                    cpi->MaxBufferLevel , cpi->MaxAllowedDatarate * cpi->ActualTargetBitRate / cpi->Configuration.OutputFrameRate / 100,
                    cpi->ThisFrameTarget
                    );
                fclose(sectionstats);
            }
        }


		/*

        // determine q to use for this section
        
            double target;                           // target bitrate
            double NewQ;
            double RoomForVariation=3;
            double FirstPassBitRate = sectionStats.BitsPerMacroblock * actualMBS * cpi->Configuration.OutputFrameRate;
            double SectionBitsPerMB = sectionStats.BitsPerMacroblock * sectionStats.count;
            double Pctg = SectionBitsPerMB / cpi->TotalBitsPerMB ; 
            double DesiredSectionSize = cpi->TotalBitsLeftInClip* Pctg;
            double DesiredSectionBitRate = cpi->Configuration.OutputFrameRate * DesiredSectionSize / sectionStats.count;


            if(cpi->TwoPassVBRBias)
            {
                DesiredSectionBitRate = cpi->ActualTargetBitRate * (100 - cpi->TwoPassVBRBias) / 100  + DesiredSectionBitRate * cpi->TwoPassVBRBias / 100  ;
            }

            // Clamp the Section Datarate between what will fill up the buffer and what will empty it to .25 of the optimal
            {
                double ActualPerFrameBandWidth = cpi->ActualTargetBitRate / cpi->Configuration.OutputFrameRate;
                double UnusedSectionEndBufferLevel = cpi->BufferLevel + (((cpi->MaxAllowedDatarate * ActualPerFrameBandWidth) / 100) * sectionStats.count);
                double QuarterOptimalBufferLevel = cpi->OptimalBufferLevel / 4.0;
                double MaxBitRate = cpi->Configuration.OutputFrameRate * (UnusedSectionEndBufferLevel - QuarterOptimalBufferLevel) / (sectionStats.count + cpi->KFBoost / 16);
                double MinBitRate = cpi->Configuration.OutputFrameRate * (UnusedSectionEndBufferLevel - cpi->MaxBufferLevel ) / (sectionStats.count + cpi->KFBoost / 16);
                if( MaxBitRate < cpi->ActualTargetBitRate / 3) 
                    MaxBitRate = cpi->ActualTargetBitRate / 3;
                if( MinBitRate < cpi->ActualTargetBitRate / 3) 
                    MinBitRate = cpi->ActualTargetBitRate / 3;
                
                if(DesiredSectionBitRate > MaxBitRate)
                    DesiredSectionBitRate = MaxBitRate;

                if(DesiredSectionBitRate < MinBitRate)
                    DesiredSectionBitRate = MinBitRate;

                cpi->Configuration.TargetBandwidth = (INT32) DesiredSectionBitRate; 


                cpi->InterFrameTarget =  (INT32)((cpi->Configuration.TargetBandwidth -
                    ((cpi->KeyFrameDataTarget * cpi->Configuration.OutputFrameRate)/cpi->KeyFrameFrequency)) / cpi->Configuration.OutputFrameRate);


                //cpi->PerFrameBandwidth = (cpi->Configuration.TargetBandwidth / cpi->Configuration.OutputFrameRate);

            }

            target = (double) cpi->Configuration.TargetBandwidth;

            // if q is worse than we estimated for the entire clip use it ( this must be a tough section )!! 
            //   otherwise use the one we estimated.
            NewQ = (INT32)  FIRSTPASS_Q  -  ( RoomForVariation + .5 + log(FirstPassBitRate/target) / log(1.040));
            if( NewQ < cpi->CalculatedWorstQ )
            {
                if(NewQ < cpi->PassedInWorstQ)
                    NewQ = cpi->PassedInWorstQ;

                cpi->Configuration.ActiveWorstQuality = (INT32) NewQ;
                cpi->Configuration.WorstQuality = (INT32) NewQ;
            }
            else
            {
                cpi->Configuration.ActiveWorstQuality = cpi->CalculatedWorstQ;
                cpi->Configuration.WorstQuality = cpi->CalculatedWorstQ;
            }

			if(0)
            {
                FILE *sectionstats = fopen("section.stt","a");
                fprintf(sectionstats,"Frame : %8d Count :%4d sq bits/mb:%8.3f BitsPerMB:%8.3f  BitRate: %8d Q:%3d s:%8d buffer:%8d max:%8d mdr %d %d \n ",
                    - 1 + (INT32) cpi->CurrentFrame , sectionStats.count, sectionStats.SqBitsPerMacroblock, sectionStats.BitsPerMacroblock, 
                    cpi->Configuration.TargetBandwidth / 1024, cpi->Configuration.WorstQuality, cpi->SizeStep,cpi->BufferLevel ,
                    cpi->MaxBufferLevel , cpi->MaxAllowedDatarate * cpi->ActualTargetBitRate / cpi->Configuration.OutputFrameRate / 100,
                    cpi->ThisFrameTarget
                    );
                fclose(sectionstats);
            }
        }

*/
    }

    // its not a keyframe check if its time to update our golden frame?
    else if (cpi->FramesTillGfUpdateDue == 0 )
    {
        FIRSTPASS_STATS GfStats; 
        int count =0;
        //double GfuMotionComplexity;
        //double MaxVariance;
        //int NonZeroMV;
        //int NewMotion = 100 - (int) GfStats.PercentMotion;
        //int ZeroMotion = (int) (GfStats.PercentMotion - GfStats.PercentNewMotion);
        int IntraToInterRatio;
        int GfUsage;

        ZeroStats( &GfStats);
        // ignore the next frame ( it will have this frame as reference no matter what)
        InputStats(cpi->fs,&nextFrame);

        // check next frames
        for(i = 0 ;i < 4 ; i++)
        {

            InputStats(cpi->fs,&nextFrame);
            AccumulateStats(&GfStats, &nextFrame);

            if(nextFrame.isGolden) 
            {
                // throwout the next frame after this one
                InputStats(cpi->fs,&lastFrame);
            }
        }
        AvgStats(&GfStats);

        // + 300 to stop tiny frames from producing huge boosts)
        IntraToInterRatio = (int) (100 * GfStats.MeanIntraError / (GfStats.MeanInterError ));
        IntraToInterRatio = (int) (IntraToInterRatio * GfStats.PercentNewMotion / 100);
        GfUsage = (int) (GfStats.PercentGolden * 8);
      
        cpi->GfuBoost = IntraToInterRatio;

		// Correct boost to take account of recent observed level of GF usage
		if ( (GfUsage >> 3) < 64)
			cpi->GfuBoost = (cpi->GfuBoost * GfUsageCorrection[(GfUsage  >> 3)]) / 16;
		else
			cpi->GfuBoost = (cpi->GfuBoost * GfUsageCorrection[63]) / 16;
        

        cpi->GfuBoost = cpi->GfuBoost* GfuDataRateBoost[cpi->pb.AvgFrameQIndex] / 1000;
        

		// Should we even consider a GF update or is there no point
		if ( ( GfStats.PercentNewMotion > GF_MODE_DIST_THRESH2) &&
			 (GfStats.MotionSpeed <= MAX_GF_UPDATE_MOTION) //&& 
             //(cpi->GfuBoost > 80 ) && 
			 //(MaxVariance <= GF_MAX_VAR_THRESH) 
           )
		{
			cpi->ThisFrameTarget = (cpi->InterFrameTarget * (100 * cpi->GfUpdateInterval)) /
								   ((100 * cpi->GfUpdateInterval) + cpi->GfuBoost);

			cpi->ThisFrameTarget = cpi->ThisFrameTarget + ((cpi->ThisFrameTarget * cpi->GfuBoost) / 100);	

            if(cpi->FramesToKey > 3)
            {
                cpi->pb.RefreshGoldenFrame = TRUE;
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

			if(0)
            {
                FILE *gfstats= fopen("gf.stt","a");
                fprintf(gfstats,"Frame : %8d boost: %d, speed:%d,baseq:%d, intra2inter: %d, newmotion:%d, GfUsage:%d \n",
                    - 1 + (INT32) cpi->CurrentFrame , 
                    cpi->GfuBoost,
                    cpi->GfuMotionSpeed,
                    GfuDataRateBoost[cpi->pb.AvgFrameQIndex],
					100 * GfStats.MeanIntraError / (GfStats.MeanInterError),
					GfStats.PercentNewMotion,
					GfUsage
                    );
                fclose(gfstats);
            }

		}
        else
        {

        }


        fgetpos(cpi->fs,&pos2);
        pos2-=pos1;
        fseek(cpi->fs,(INT32) -pos2,SEEK_CUR);


    }

    // check if we should boost or lower this frame based on our neighbors. 
    else
    {
    }


    cpi->FramesYetToEncode --;
    cpi->FramesToKey --;
    cpi->TotalBitsPerMB = NewBitsPerMB;

}
/****************************************************************************
 *
 *  ROUTINE       : Pass1Output
 *
 *  INPUTS        : CP_INSTANCE *cpi                      : Pointer to encoder instance.
 *                 
 *  OUTPUTS       : 
 *                  
 *
 *  RETURNS       : 
 *
 *  FUNCTION      : output to external file the 1st pass results
 *
 *  SPECIAL NOTES : 
 *
 ****************************************************************************/
void CCONV Pass1Output( CP_INSTANCE *cpi)
{
    PB_INSTANCE *pbi = &cpi->pb;
    int actualMBS= (cpi->pb.MBRows - (BORDER_MBS*2)) * (cpi->pb.MBCols - (BORDER_MBS*2));
    ClearSysState();
    cpi->fps.MeanInterError = 1.0 * cpi->InterErrorb / actualMBS;
    cpi->fps.MeanIntraError = 1.0 * cpi->IntraError / actualMBS;

    cpi->fps.isKey = pbi->FrameType == BASE_FRAME; 
    cpi->fps.isGolden = pbi->RefreshGoldenFrame;
    cpi->fps.PSNR = 60;
    cpi->fps.BitsPerMacroblock = 1.0 * cpi->ThisFrameSize / actualMBS;
    cpi->fps.SqBitsPerMacroblock = cpi->fps.BitsPerMacroblock*cpi->fps.BitsPerMacroblock;
    cpi->fps.QValue = cpi->pb.quantizer->FrameQIndex;
    cpi->fps.MeanInterError ;
    cpi->fps.MeanIntraError ;
    cpi->fps.frame = (UINT32) (cpi->CurrentFrame-1);

    AccumulateStats( &cpi->fpmss, &cpi->fps);
    OutputStats(cpi->fs,&cpi->fps);
}
