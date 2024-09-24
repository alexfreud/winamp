/****************************************************************************
 *        
 *   Module Title :     clamp.c
 *
 *   Description  :     c
 *
 *   AUTHOR       :     Jim Bankoski
 *
 *****************************************************************************
 *   Revision History
 *
 *   1.09 YWX 26-Sep-01 Changed the default bandHeight from 5 to 4
 *   1.08 YWX 23-Jul-00 Changed horizontal scaling function names
 *   1.07 JBB 04 Dec 00 Added new Center vs Scale Bits
 *   1.06 YWX 01-Dec-00 Removed bi-cubic scale functions
 *   1.05 YWX 18-Oct-00 Added 1-2 scale functions
 *   1.04 YWX 11-Oct-00 Added ratio check to determine scaling or centering
 *   1.03 YWX 09-Oct-00 Added functions that do differen scaling in horizontal
 *                      and vertical directions
 *   1.02 YWX 04-Oct-00 Added 3-5 scaling functions
 *   1.01 YWX 03-Oct-00 Added a set of 4-5 scaling functions
 *   1.00 JBB 15 Sep 00 New Configuration baseline.
 *
 *****************************************************************************
 */

/****************************************************************************
*  Header Files
*****************************************************************************
*/
#include "postp.h"
#include <stdio.h>

/****************************************************************************
 *  Imported
 *****************************************************************************
 */

void ClampLevels_C( 
	POSTPROC_INSTANCE *pbi,
	INT32        BlackClamp,			// number of values to clamp from 0 
	INT32        WhiteClamp,			// number of values to clamp from 255
	UINT8		*Src,					// reconstruction buffer : passed in
	UINT8		*Dst					// postprocessing buffer : passed in
	)
{

	unsigned char clamped[255];
	int			  width = pbi->HFragments*8;
	int			  height = pbi->VFragments*8;				// Y plane will be done in two passes
	UINT8		 *SrcPtr = Src + pbi->ReconYDataOffset;
	UINT8		 *DestPtr = Dst + pbi->ReconYDataOffset;
	UINT32		  LineLength = pbi->YStride * 2;				// pitch is doubled for interlacing

	// set up clamping table so we can avoid ifs while clamping
	int i;
	for(i=0;i<255;i++)
	{
		if(i<BlackClamp)
			clamped[i] = BlackClamp;

		if(i>WhiteClamp)
			clamped[i] = WhiteClamp;
	}

	Block = 0;	

	// clamping is for y only!
	for ( row = 0 ; row < height ; row ++)
	{
		for (col = 0; col < width ; col ++)
		{
			SrcPtr[col]=clamped[DestPtr[col]];
		}
		SrcPtr += LineLength;
		DestPtr += LineLength;
    }


}
/****************************************************************************
 *  Module constants.
 *****************************************************************************
 */        
 
       
/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/


/****************************************************************************
*  Module Static Variables
*****************************************************************************
*/
