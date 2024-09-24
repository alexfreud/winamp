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
void ClampLevels_wmt( 
	POSTPROC_INSTANCE *pbi,
	INT32        BlackClamp,			// number of values to clamp from 0 
	INT32        WhiteClamp,			// number of values to clamp from 255
	UINT8		*Src,					// reconstruction buffer : passed in
	UINT8		*Dst					// postprocessing buffer : passed in
	)
{
#if defined(_WIN32_WCE)
	return;
#else

	__declspec(align(16)) unsigned char blackclamp[16];
	__declspec(align(16)) unsigned char whiteclamp[16];
	__declspec(align(16)) unsigned char bothclamp[16];

	int i; 
	int	width  = pbi->HFragments *8;
	int	height = pbi->VFragments *8;
	UINT8 *SrcPtr     = Src + pbi->ReconYDataOffset;
	UINT8 *DestPtr    = Dst + pbi->ReconYDataOffset;
	UINT32 LineLength = pbi->YStride ;				// pitch is doubled for interlacing
	int row;

	for(i=0;i<16;i++)
	{
		blackclamp[i]=(unsigned char )BlackClamp;
		whiteclamp[i]=(unsigned char )WhiteClamp;
		bothclamp[i]=BlackClamp+WhiteClamp;
	}

	// clamping is for y only!
	for ( row = 0 ; row < height ; row ++)
	{
		__asm
		{
			mov         ecx, [width]
			mov         esi, SrcPtr
			mov			edi, DestPtr
			xor		    eax,eax
		nextset:
			movdqa      xmm1,[esi+eax]
			psubusb     xmm1,blackclamp
			paddusb     xmm1,bothclamp
			psubusb     xmm1,whiteclamp
			movdqa      [edi+eax],xmm1             ;write first 4 pixels
			add         eax,16
			cmp         eax, ecx
			jl			nextset
		}
		SrcPtr += LineLength;
		DestPtr += LineLength;
    }
#endif
}



void ClampLevels_mmx( 
	POSTPROC_INSTANCE *pbi,
	INT32        BlackClamp,			// number of values to clamp from 0 
	INT32        WhiteClamp,			// number of values to clamp from 255
	UINT8		*Src,					// reconstruction buffer : passed in
	UINT8		*Dst					// postprocessing buffer : passed in
	)
{

#if defined(_WIN32_WCE)
	#pragma pack(8)
	unsigned char blackclamp[16];
	unsigned char whiteclamp[16];
	unsigned char bothclamp[16];
	#pragma pack()
#else
	__declspec(align(8)) unsigned char blackclamp[16];
	__declspec(align(8)) unsigned char whiteclamp[16];
	__declspec(align(8)) unsigned char bothclamp[16];
#endif
	int i; 
	int	width  = pbi->HFragments *8;
	int	height = pbi->VFragments *8;
	UINT8 *SrcPtr     = Src + pbi->ReconYDataOffset;
	UINT8 *DestPtr    = Dst + pbi->ReconYDataOffset;
	UINT32 LineLength = pbi->YStride ;				// pitch is doubled for interlacing
	int row;

	for(i=0;i<8;i++)
	{
		blackclamp[i]=(unsigned char )BlackClamp;
		whiteclamp[i]=(unsigned char )WhiteClamp;
		bothclamp[i]=BlackClamp+WhiteClamp;
	}

	// clamping is for y only!
	for ( row = 0 ; row < height ; row ++)
	{
		__asm
		{
			mov         ecx, [width]
			mov         esi, SrcPtr
			mov			edi, DestPtr
			xor		    eax,eax
		nextset:
			movq        mm1,[esi+eax]
			psubusb     mm1,blackclamp
			paddusb     mm1,bothclamp
			psubusb     mm1,whiteclamp
			movq        [edi+eax],mm1             ;write first 4 pixels
			add         eax,8
			cmp         eax, ecx
			jl			nextset
		}
		SrcPtr += LineLength;
		DestPtr += LineLength;
    }
}
