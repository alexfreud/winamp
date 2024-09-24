/****************************************************************************
*        
*   Module Title :	   Decodemode.c     
*
*   Description  :     functions for decoding modes and motionvectors 
*
*   AUTHOR       :     James Bankoski
*
*****************************************************************************
*   Revision History
*
*   1.00 JBB 30OCT01  New Configuration baseline.
*   1.01 JBB 04AP402  Reworked lower footprint mode compression scheme
*
*****************************************************************************
*/ 
//************************************************************************************
// Decoding the Modes: 
//
//  Decode Mode Tree Looks like this :
//
//
//
//
//                                            zz 
//                                                             
//                               0                        Mode Same As Last
//                                                                
//                    
//              1                                       2
//
//       3             4                  5                          6
//
//  NoMV   +MV    Nest  Near        Intra   FourMV          7                 8
//                                                      
//                                                   00Gold   GoldMV    GNrst   GNear
//
//
// 30 probabilitity contexts are set up at each branch (in probMode) corresponding to 
//
//   3 for what situation we are in at the mode level ( all modes available, 
//     no nearest mv found, and no near mv found) 
//
//  10 one for each possible last mode
//
// Note: if the last mode was near then the probability of getting near at position 4 
// above is set to 0 (it would have been coded as same as last). Note also that the 
// probablity of getting near when no near mv is available is also always set to 0.
//
// These probs are created from the 20 that can be xmitted in the bitstream (probXmitted)
//    For each mode 2 probabilities can be transmitted:
//        probability that the mode will appear if the last mode was the same
//        probability that the mode will appear if the last mode is not that mode
//
//************************************************************************************

/****************************************************************************
*  Header Files
*****************************************************************************
*/
#include "pbdll.h"
#include "decodemode.h"
#include "decodemv.h"


/****************************************************************************
*  Implicit Imports
*****************************************************************************
*/        
#define STRICT              /* Strict type checking. */

#ifdef MAPCA
    #include <eti/mm.h>
#endif

/****************************************************************************
*  Exported data structures.
*****************************************************************************
*/        


/****************************************************************************
*  Module statics.
*****************************************************************************
*/        
//*****************************************************************************
// ModeVQ: This structure holds a table of probability vectors for encoding modes
// To build this table a number of clips were run through and allowed to 
// select each of the probabilities that were best for them on each frame.  These 
// choices were output and a vector quantizer was used to optimize the selection 
// of 16 vectors for each MODETYPE (allmodes available, nonearest, and no near)
//*****************************************************************************
UINT8 ModeVq[MODETYPES][MODEVECTORS][MAX_MODES*2]=
{
9,15,32,25,7,19,9,21,1,12,14,12,3,18,14,23,3,10,0,4,
48,39,1,2,11,27,29,44,7,27,1,4,0,3,1,6,1,2,0,0,
21,32,1,2,4,10,32,43,6,23,2,3,1,19,1,6,12,21,0,7,
69,83,0,0,0,2,10,29,3,12,0,1,0,3,0,3,2,2,0,0,
11,20,1,4,18,36,43,48,13,35,0,2,0,5,3,12,1,2,0,0,
70,44,0,1,2,10,37,46,8,26,0,2,0,2,0,2,0,1,0,0,
8,15,0,1,8,21,74,53,22,42,0,1,0,2,0,3,1,2,0,0,
141,42,0,0,1,4,11,24,1,11,0,1,0,1,0,2,0,0,0,0,
8,19,4,10,24,45,21,37,9,29,0,3,1,7,11,25,0,2,0,1,
46,42,0,1,2,10,54,51,10,30,0,2,0,2,0,1,0,1,0,0,
28,32,0,0,3,10,75,51,14,33,0,1,0,2,0,1,1,2,0,0,
100,46,0,1,3,9,21,37,5,20,0,1,0,2,1,2,0,1,0,0,
27,29,0,1,9,25,53,51,12,34,0,1,0,3,1,5,0,2,0,0,
80,38,0,0,1,4,69,33,5,16,0,1,0,1,0,0,0,1,0,0,
16,20,0,0,2,8,104,49,15,33,0,1,0,1,0,1,1,1,0,0,
194,16,0,0,1,1,1,9,1,3,0,0,0,1,0,1,0,0,0,0,

41,22,1,0,1,31,0,0,0,0,0,1,1,7,0,1,98,25,4,10,
123,37,6,4,1,27,0,0,0,0,5,8,1,7,0,1,12,10,0,2,
26,14,14,12,0,24,0,0,0,0,55,17,1,9,0,36,5,7,1,3,
209,5,0,0,0,27,0,0,0,0,0,1,0,1,0,1,0,0,0,0,
2,5,4,5,0,121,0,0,0,0,0,3,2,4,1,4,2,2,0,1,
175,5,0,1,0,48,0,0,0,0,0,2,0,1,0,2,0,1,0,0,
83,5,2,3,0,102,0,0,0,0,1,3,0,2,0,1,0,0,0,0,
233,6,0,0,0,8,0,0,0,0,0,1,0,1,0,0,0,1,0,0,
34,16,112,21,1,28,0,0,0,0,6,8,1,7,0,3,2,5,0,2,
159,35,2,2,0,25,0,0,0,0,3,6,0,5,0,1,4,4,0,1,
75,39,5,7,2,48,0,0,0,0,3,11,2,16,1,4,7,10,0,2,
212,21,0,1,0,9,0,0,0,0,1,2,0,2,0,0,2,2,0,0,
4,2,0,0,0,172,0,0,0,0,0,1,0,2,0,0,2,0,0,0,
187,22,1,1,0,17,0,0,0,0,3,6,0,4,0,1,4,4,0,1,
133,6,1,2,1,70,0,0,0,0,0,2,0,4,0,3,1,1,0,0,
251,1,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

2,3,2,3,0,2,0,2,0,0,11,4,1,4,0,2,3,2,0,4,
49,46,3,4,7,31,42,41,0,0,2,6,1,7,1,4,2,4,0,1,
26,25,1,1,2,10,67,39,0,0,1,1,0,14,0,2,31,26,1,6,
103,46,1,2,2,10,33,42,0,0,1,4,0,3,0,1,1,3,0,0,
14,31,9,13,14,54,22,29,0,0,2,6,4,18,6,13,1,5,0,1,
85,39,0,0,1,9,69,40,0,0,0,1,0,3,0,1,2,3,0,0,
31,28,0,0,3,14,130,34,0,0,0,1,0,3,0,1,3,3,0,1,
171,25,0,0,1,5,25,21,0,0,0,1,0,1,0,0,0,0,0,0,
17,21,68,29,6,15,13,22,0,0,6,12,3,14,4,10,1,7,0,3,
51,39,0,1,2,12,91,44,0,0,0,2,0,3,0,1,2,3,0,1,
81,25,0,0,2,9,106,26,0,0,0,1,0,1,0,1,1,1,0,0,
140,37,0,1,1,8,24,33,0,0,1,2,0,2,0,1,1,2,0,0,
14,23,1,3,11,53,90,31,0,0,0,3,1,5,2,6,1,2,0,0,
123,29,0,0,1,7,57,30,0,0,0,1,0,1,0,1,0,1,0,0,
13,14,0,0,4,20,175,20,0,0,0,1,0,1,0,1,1,1,0,0,
202,23,0,0,1,3,2,9,0,0,0,1,0,1,0,1,0,0,0,0
};

// These are the probabilities that we reset to after each keyframe.  
// It was created as the average probabilities of the trees.
UINT8 BaselineXmittedProbs[4][2][10]=
{
 42,  2,  7, 42, 22,  3,  2,  5,  1,  0, 69,  1,  1, 44,  6,  1,  0,  1,  0,  0,
  8,  1,  8,  0,  0,  2,  1,  0,  1,  0,229,  1,  0,  0,  0,  1,  0,  0,  1,  0,
 35,  1,  6, 34,  0,  2,  1,  1,  1,  0,122,  1,  1, 46,  0,  1,  0,  0,  1,  0,
 64,  0, 64, 64, 64,  0,  0,  0,  0,  0, 64,  0, 64, 64, 64,  0,  0,  0,  0,  0,
};


/****************************************************************************
 * 
 *  ROUTINE       : BuildModeTree
 *
 *  INPUTS        : 
 *						
 *  OUTPUTS       : 
 *
 *  RETURNS       :     
 *
 *  FUNCTION      : Fills in probabilities at each branch of the huffman tree
 *                  based upon the frequencies transmitted in the bitstream. 
 *                  probXmitted 
 *
 *
 *  ERRORS        : None.
 *
 ****************************************************************************/
void BuildModeTree
(	
 PB_INSTANCE *pbi
)
{
	int i,j,k;

	// make a huffman tree and code array for each of our modes (note each of the trees is minus the node give by probmodesame)
	for(i=0;i<10;i++)
	{
		unsigned int Counts[MAX_MODES];
		unsigned int total;

		// set up the probabilities for each tree
		for(k=0;k<MODETYPES;k++)
		{
			total=0;
			for(j=0;j<10;j++)
			{	
				if(i==j)
				{
					Counts[j]=0;
				}
				else
				{
					Counts[j]=100*pbi->probXmitted[k][0][j];
				}


				total+=Counts[j];
			}


			pbi->probModeSame[k][i] = 255-
				255 * pbi->probXmitted[k][1][i] 
				/
				(	1 +
					pbi->probXmitted[k][1][i] +	
					pbi->probXmitted[k][0][i]
				);

			// each branch is basically calculated via 
			// summing all posibilities at that branch.
			pbi->probMode[k][i][0]= 1 + 255 *
				(
					Counts[CODE_INTER_NO_MV]+
					Counts[CODE_INTER_PLUS_MV]+
					Counts[CODE_INTER_NEAREST_MV]+
					Counts[CODE_INTER_NEAR_MV]
				) / 
				(   1 +
				    total
				);

			pbi->probMode[k][i][1]= 1 + 255 *
				(
					Counts[CODE_INTER_NO_MV]+
					Counts[CODE_INTER_PLUS_MV]
				) / 
				(
					1 + 
					Counts[CODE_INTER_NO_MV]+
					Counts[CODE_INTER_PLUS_MV]+
					Counts[CODE_INTER_NEAREST_MV]+
					Counts[CODE_INTER_NEAR_MV]
				);

			pbi->probMode[k][i][2]= 1 + 255 *
				(
					Counts[CODE_INTRA]+
					Counts[CODE_INTER_FOURMV]
				) / 
				(
					1 + 
					Counts[CODE_INTRA]+
					Counts[CODE_INTER_FOURMV]+
					Counts[CODE_USING_GOLDEN]+
					Counts[CODE_GOLDEN_MV]+
					Counts[CODE_GOLD_NEAREST_MV]+
					Counts[CODE_GOLD_NEAR_MV]
				);
			
			pbi->probMode[k][i][3]= 1 + 255 *
				(
					Counts[CODE_INTER_NO_MV]
				) / 
				(
					1 +
					Counts[CODE_INTER_NO_MV]+
					Counts[CODE_INTER_PLUS_MV]
				);

			pbi->probMode[k][i][4]= 1 + 255 *
				(
					Counts[CODE_INTER_NEAREST_MV]
				) / 
				(
					1 +
					Counts[CODE_INTER_NEAREST_MV]+
					Counts[CODE_INTER_NEAR_MV]
				) ;

			pbi->probMode[k][i][5]= 1 + 255 *
				(
					Counts[CODE_INTRA]
				) / 
				(
					1 +
					Counts[CODE_INTRA]+
					Counts[CODE_INTER_FOURMV]
				);

			pbi->probMode[k][i][6]= 1 + 255 *
				(
					Counts[CODE_USING_GOLDEN]+
					Counts[CODE_GOLDEN_MV]
				) / 
				(
					1 +
					Counts[CODE_USING_GOLDEN]+
					Counts[CODE_GOLDEN_MV]+
					Counts[CODE_GOLD_NEAREST_MV]+
					Counts[CODE_GOLD_NEAR_MV]
				);

			pbi->probMode[k][i][7]= 1 + 255 *
				(
					Counts[CODE_USING_GOLDEN]
				) / 
				(
					1 +
					Counts[CODE_USING_GOLDEN]+
					Counts[CODE_GOLDEN_MV]
				);

			pbi->probMode[k][i][8]= 1 + 255 *
				(
					Counts[CODE_GOLD_NEAREST_MV]
				) / 
				(
					1 +
					Counts[CODE_GOLD_NEAREST_MV]+
					Counts[CODE_GOLD_NEAR_MV]
				);
		}
	}
}
/****************************************************************************
 * 
 *  ROUTINE       : decodeModeDiff
 *
 *  INPUTS        : 
 *						
 *  OUTPUTS       : diff -> the probability difference value decoded from the bitstream    
 *
 *  RETURNS       :     
 *
 *  FUNCTION      : this function returns a value probability difference value 
 *                  -256 to +256 in steps of 4  transmitted  in the bitstream
 *                  using a fixed tree and hardcoded probabilities 
 *
 *  SPECIAL NOTES : The hard coded probabilities for the difference tree
 *                  were calcualated by taking the average number of times a 
 *                  branch was taken on some sample material ie 
 *                  (bond,bike,beautifulmind)
 *  
 *  
 *
 *  ERRORS        : None.
 *
 ****************************************************************************/
int decodeModeDiff
(
   PB_INSTANCE *pbi
)
{

	int sign;
	if(DecodeBool(&pbi->br, 205)==0)
	{
		return 0;
	}
	
	sign = 1 + -2 * DecodeBool128(&pbi->br);
	
	if( !DecodeBool(&pbi->br,171))
	{
        return sign<<(3-DecodeBool(	&pbi->br,83));
        /*
		if( DecodeBool(	&pbi->br,83))
			return sign*4;
		else
			return sign*8;
            */
	}
	else
	{
		if( !DecodeBool(	&pbi->br,199) ) 
		{
			if(DecodeBool(	&pbi->br,140))
				return sign * 12;

			if(DecodeBool(	&pbi->br,125))
				return sign * 16;

			if(DecodeBool(	&pbi->br,104))
				return sign * 20;

			return sign * 24;

		}
		else 
		{
			int diff =VP5_bitread(&pbi->br,7);
			return sign *diff*4;
		}
	}
	
}


/****************************************************************************
 * 
 *  ROUTINE       :     DecodeModeProbs
 *
 *  INPUTS        :     
 *						
 *  OUTPUTS       :     
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     This function parses the probabilities xmitted in 
 *                      the bitstream. The bitstream may either use the 
 *                      lastframes baselines, or transmit a pointer to a
 *                      vector of new probabilities. It may then also 
 *                      contain updates to each of these probabilities.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void DecodeModeProbs
(
	PB_INSTANCE *pbi
)
{
	int i,j;
	// For each mode type (all modes available, no nearest, no near mode)
	for(j=0;j<MODETYPES;j++)
	{
		// determine whether we are sending a vector for this mode byte
		if(DecodeBool( &pbi->br, PROBVECTORXMIT) )
		{
			// figure out which vector we have encoded
			int whichVector = VP5_bitread(&pbi->br, 4);

			// adjust the vector
			for(i=0;i<MAX_MODES;i++)
			{
				pbi->probXmitted[j][1][i] = ModeVq[j][whichVector][i*2];
				pbi->probXmitted[j][0][i] = ModeVq[j][whichVector][i*2+1];
			}
		} 

		// decode whether updates to bring it closer to ideal 
		if( DecodeBool( &pbi->br, PROBIDEALXMIT) )
		{
			for(i=0;i<10;i++)
			{
				int diff;

				// determine difference 
				diff = decodeModeDiff(pbi);
				diff += pbi->probXmitted[j][1][i];

				pbi->probXmitted[j][1][i] = (diff<0?0:(diff>255?255:diff));

				// determine difference 
				diff = decodeModeDiff(pbi);
				diff += pbi->probXmitted[j][0][i];

				pbi->probXmitted[j][0][i] = (diff<0?0:(diff>255?255:diff));

			}
		}
	}
	
	BuildModeTree(pbi);
}



/****************************************************************************
 * 
 *  ROUTINE       :     decodeModeandMotionVector
 *
 *  INPUTS        :     MBrow -> row 
						MBcol -> column
						
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     decodes a macroblock's mode and motion vectors from 
                        the bitstream 
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

void decodeModeAndMotionVector
(
 PB_INSTANCE *pbi,
 UINT32 MBrow,
 UINT32 MBcol
 )
{
	CODING_MODE mode;//lastmode;
	int type,type2;
	UINT32 k;
	MOTION_VECTORA NearestInterMVect,NearInterMVect; 
	MOTION_VECTORA NearestGoldMVect,NearGoldMVect;
	MOTION_VECTOR mv;
    int x, y;

	FindNearestandNextNearest(pbi,MBrow,MBcol,&NearestInterMVect,&NearInterMVect,1,&type);

	mode = 	DecodeMode(pbi,pbi->LastMode,type);
    pbi->LastMode = mode; 
	
	pbi->predictionMode[MBOffset(MBrow,MBcol)] = mode;
	pbi->mbi.Mode = mode;
    if(mode ==CODE_INTER_FOURMV)
    {
		pbi->mbi.BlockMode[0] = DecodeBlockMode(pbi);
		pbi->mbi.BlockMode[1] = DecodeBlockMode(pbi);
		pbi->mbi.BlockMode[2] = DecodeBlockMode(pbi);
		pbi->mbi.BlockMode[3] = DecodeBlockMode(pbi);

		pbi->mbi.BlockMode[4] = CODE_INTER_FOURMV;
		pbi->mbi.BlockMode[5] = CODE_INTER_FOURMV;

		x=0;
		y=0;
		for(k=0;k<4;k++)
		{
			if(pbi->mbi.BlockMode[k]==CODE_INTER_NO_MV)
            {
				pbi->mbi.Mv[k].x = 0;
                pbi->mbi.Mv[k].y = 0;
            }
			else if( pbi->mbi.BlockMode[k]==CODE_INTER_NEAREST_MV)
            {
				pbi->mbi.Mv[k].x = NearestInterMVect.x;
                pbi->mbi.Mv[k].y = NearestInterMVect.y;                
                x+=NearestInterMVect.x;
				y+=NearestInterMVect.y;
            }
            else if( pbi->mbi.BlockMode[k]==CODE_INTER_NEAR_MV)
            {
				pbi->mbi.Mv[k].x = NearInterMVect.x;
                pbi->mbi.Mv[k].y = NearInterMVect.y;                
                x+=NearInterMVect.x;
				y+=NearInterMVect.y;
            }
            else if ( pbi->mbi.BlockMode[k]==CODE_INTER_PLUS_MV)
            {
				decodeMotionVector(pbi,&mv,NULL);
				pbi->mbi.Mv[k].x = mv.x;
                pbi->mbi.Mv[k].y = mv.y;
                x+=mv.x;
				y+=mv.y;
            }
		}
        x = (x+1+(x>=0))>>2;
        y = (y+1+(y>=0))>>2;

        pbi->MBMotionVector[MBOffset(MBrow,MBcol)].x = pbi->mbi.Mv[3].x;
        pbi->MBMotionVector[MBOffset(MBrow,MBcol)].y = pbi->mbi.Mv[3].y;
        
        pbi->mbi.Mv[4].x = x; 
        pbi->mbi.Mv[4].y = y;

        pbi->mbi.Mv[5].x = x; 
        pbi->mbi.Mv[5].y = y;
        

    }
    else
    {
        if(mode == CODE_INTER_NEAREST_MV)
        {
            x = NearestInterMVect.x;
            y = NearestInterMVect.y;            
        }
        else if(mode == CODE_INTER_NEAR_MV)
        {
            x = NearInterMVect.x;
            y = NearInterMVect.y;
        }
        else 
        {
            switch(mode)
            {
            /*
            case CODE_INTER_NEAREST_MV:
            x = NearestInterMVect.x;
            y = NearestInterMVect.y;            
            break;
            case CODE_INTER_NEAR_MV:
            x = NearInterMVect.x;
            y = NearInterMVect.y;
            break;
                */
            case CODE_GOLD_NEAREST_MV:
                FindNearestandNextNearest(pbi,MBrow,MBcol,&NearestGoldMVect,&NearGoldMVect,2,&type2);
                x = NearestGoldMVect.x;
                y = NearestGoldMVect.y;
                break;
            case CODE_GOLD_NEAR_MV:
                FindNearestandNextNearest(pbi,MBrow,MBcol,&NearestGoldMVect,&NearGoldMVect,2,&type2);
                x = NearGoldMVect.x;
                y = NearGoldMVect.y;
                break;
            case CODE_INTER_PLUS_MV:
            case CODE_GOLDEN_MV:
                decodeMotionVector(pbi,&mv,NULL);
                x = mv.x;
                y = mv.y;
                break;
            default:
                x =0;
                y =0;
            }
        }
        pbi->MBMotionVector[MBOffset(MBrow,MBcol)].x = x;
        pbi->MBMotionVector[MBOffset(MBrow,MBcol)].y = y;
		for(k=0;k<6;k++)
		{
			
            pbi->mbi.Mv[k].x = x;
            pbi->mbi.Mv[k].y = y;
			pbi->mbi.BlockMode[k] = mode;
		}
    }

}

/****************************************************************************
 * 
 *  ROUTINE       :     decodeBlockMode
 *
 *  INPUTS        :     mode -> mode we are trying to encode
 *						
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     decodes a block mode from the bitstream as 2 bits
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

CODING_MODE 
DecodeBlockMode
(
	PB_INSTANCE *pbi
)

{

	int choice = DecodeBool128(&pbi->br)<<1;
	choice += DecodeBool128(&pbi->br);


	switch(choice)
	{
	case 0:return CODE_INTER_NO_MV;//0
	case 1:return CODE_INTER_PLUS_MV;//2
	case 2:return CODE_INTER_NEAREST_MV;//3
	case 3:return CODE_INTER_NEAR_MV;//4
	}
	return (CODING_MODE)0;

}   

/****************************************************************************
 * 
 *  ROUTINE       :     decodeMode
 *
 *  INPUTS        :     lastmode -> mode of the last coded macroblock
 *						mode -> mode we are trying to encode
 *						type -> MODE_TYPE (all modes available, nonearest 
 *						        macroblock, no near macroblock)
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     decodes a MBmode from the bitstream using modecodearray
 *                      and probabilities that the value is the same as 
 *                      lastmode stored in probModeSame, and the probability 
 *                      of mode occuring if lastmode != mode stored in 
 *                      probMode
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
CODING_MODE DecodeMode
(
	PB_INSTANCE *pbi,
	CODING_MODE lastmode,
	UINT32 type
)
{
	CODING_MODE	mode;
	if(DecodeBool(&pbi->br,pbi->probModeSame[type][lastmode]))
	{
		mode = lastmode;
	}
	else
	{ // 0
		UINT8 * Stats =pbi->probMode[type][lastmode]; 
		if(DecodeBool(&pbi->br,Stats[0]))
		{  // 2
			if(DecodeBool(&pbi->br,Stats[2]))
			{ //6
				if(DecodeBool(&pbi->br,Stats[6]))
				{ // 8

					mode = CODE_GOLD_NEAREST_MV + DecodeBool(&pbi->br,Stats[8]);
					/*
					if(DecodeBool(&pbi->br,Stats[8]))
					{
						mode = CODE_GOLD_NEAR_MV;
					}
					else
					{
						mode = CODE_GOLD_NEAREST_MV;
					}
					*/
					
				}
				else
				{ // 7
					mode = CODE_USING_GOLDEN + DecodeBool(&pbi->br,Stats[7]);
					/*
					if(DecodeBool(&pbi->br,Stats[7]))
					{
						mode = CODE_GOLDEN_MV;
					}
					else
					{
						mode = CODE_USING_GOLDEN;
					}
					*/
				}

			}
			else
			{ //5
				//mode = CODE_INTRA + 6*DecodeBool(&pbi->br,Stats[5]);
				
				if(DecodeBool(&pbi->br,Stats[5]))
				{
					mode = CODE_INTER_FOURMV;
				}
				else
				{
					mode = CODE_INTRA;
				}
				
			}
		}
		else
		{ // 1
			if(DecodeBool(&pbi->br,Stats[1]))
			{ // 4
				mode = CODE_INTER_NEAREST_MV + DecodeBool(&pbi->br,Stats[4]);
				/*
				if(DecodeBool(&pbi->br,Stats[4]))
				{
					mode = CODE_INTER_NEAR_MV;
				}
				else
				{
					mode = CODE_INTER_NEAREST_MV;
				}
				*/

			}
			else
			{ // 3
				mode = CODE_INTER_NO_MV + 2 * DecodeBool(&pbi->br,Stats[3]);
				/*
				if(DecodeBool(&pbi->br,Stats[3]))
				{
					mode = CODE_INTER_PLUS_MV;
				}
				else
				{
					mode = CODE_INTER_NO_MV;
				}
				*/
			}
		}
	}
	return mode;
}

