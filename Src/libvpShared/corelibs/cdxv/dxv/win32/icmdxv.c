// dxvmpg.cpp : Defines the entry point for the console application.
//
#include <stdlib.h>
#include <stdio.h>

#include "dkpltfrm.h" /* platform specifics */
#include "duktypes.h" /* predefined general types used at duck */

#include "duck_mem.h" /* interface to memory manager */
#include "dxl_main.h" /* interface to dxv */

#include <windows.h>
#include <mmsystem.h>
#include <vfw.h>



/* formats that might be supported by a codec and dxv */
/* call me crazy ... but I want to make this public ... ie in the HEADER ! */
BMIMapping DXL_BMIMap[] = 
{
	{ DXL_MKFOURCC('Y','V','1','2'),	12, 3, DXYV12 },

	{ DXL_MKFOURCC('I','Y','U','V'),	12, 1, DXI420 },

	{ DXL_MKFOURCC('Y','U','Y','2'),	16, 1, DXYUY2 },

	{ DXL_MKFOURCC('Y','V','Y','U'),	16, 1, DXYVYU },

	{ DXL_MKFOURCC('U','Y','V','Y'),	16, 1, DXUYVY },

	{ 0,								24, 1, DXRGB24 },

	{ 0,								32, 1, DXRGB32 }

};	



static char *MakeFourCCString(unsigned long fcc, char *buff)
{
	sprintf(buff,"%c%c%c%c",
		(fcc & 0xFF) >> 0,
		(fcc & 0xFF00) >> 8,
		(fcc & 0xFF0000) >> 16,
		(fcc & 0xFF000000) >> 24
		); 
	return buff;
}


/* Report to a little HTML file */
void DXL_ReportBMIMapping(char *filename)
{
	FILE *fp;
	int len = sizeof(DXL_BMIMap) / sizeof(BMIMapping);

	fp = fopen(filename,"w");

	if (fp)
	{
		int t;
		char temp[5];
		fprintf(fp, "<table BORDER=1>");
		fprintf(fp, "<tr><td>FOURCC</td><td>BitDepth</td><td>Planes</td><td>dxvBitDepth</td></tr>");

		for(t = 0; t < len; t++)
		{
			fprintf(fp, "<tr><td>%s<br>%x</td> <td>%ld</td> <td>%ld</td>  <td>%ld</td></tr>",
				MakeFourCCString(DXL_BMIMap[t].biCompression,temp),
				DXL_BMIMap[t].biCompression,
				DXL_BMIMap[t].biBitCount,
				DXL_BMIMap[t].biPlanes,
				DXL_BMIMap[t].bd 
				);
		}

		fprintf(fp,"</table>");
		fclose(fp);
	}

}




/*********  USAGE

In addition to regular DXV services, this library will invoke VFW 
codecs for decompression services.

Because of a bug in the frame parsing, the library is compiled to 
decompress TM2X via its codec as well. So be certain to have a
TM2X VFW codec installed.

The library has only been tested with TM2X(2.5.1.8), 
Indeo 5.2 and MPEG4.2.  Other codecs may work, but only if 
they support RGB32, RGB24, and RGB16 (555 and/or 565).
MS-CRAM and Cinepak crashed painfully in tests so far.

The library assumes support for all 4 RGB truecolor modes 
mentioned above, (NOTE: TM2X doesn't support RGB24 yet!)

- 5/19/99 - 
We added black-lining blitters for stretched modes.  Note that
24 bit display uses a 32bit offscreen buffer which is blitted
down to 24bit when stretched, this makes the asm code that much
simpler.

To use:

in addition to the regular DXV, 
link s_icm.lib to your application and do the following

substitute:
DXL_InitVideoEx(int lmaxScreens,int lmaxImages);
in place of: 
DXL_InitVideo(int lmaxScreens,int lmaxImages)

use:
movie->xImage = 
DXL_AlterXImage(movie->xImage,movie->vData,
HFB_GetStreamInfo(movie->vStream)->a.BitmapInfo.ulHandler,
0,
HFB_GetStreamInfo(movie->vStream)->a.BitmapInfo.usWidth,
HFB_GetStreamInfo(movie->vStream)->a.BitmapInfo.usHeight);

in place of:
movie->xImage = DXL_CreateXImage(movie->vData);

and, prior to any calls to:
DXL_dxImageToVScreen(movie->xImage, movie->vScreen);

you must call:
DXL_SetXImageCSize(movie->xImage, movie->vLength);

********/


static dxvBitDepth bitDepths[] = 
{
	DXRGB32,DXRGB24,DXRGB16,DXRGBNULL
};

/* define an xImage structure based on the core xImage struct */



typedef struct tXImageCODEC{  
	xImageBaseStruct;

	DK_BITMAPINFOHEADER bihIn;
	unsigned long bihInFields[3];
	DK_BITMAPINFOHEADER bihOut;
	unsigned long bihOutFields[3];
	HIC hic;
	int changeVScreen;

	BMIMapping* maps[20];
	int maxMaps;

} DXL_CODEC,*DXL_CODEC_HANDLE;





char* DXL_DecodeICERR(int err, char *storage, int length)
{
	(void)length; // not used

	switch (err)
	{
	case ICERR_UNSUPPORTED  :
		strcpy(storage,"ICERR_UNSUPPORTED");
		break;

	case ICERR_BADFORMAT :
		strcpy(storage,"ICERR_BADFORMAT");
		break;

	case ICERR_MEMORY  :
		strcpy(storage,"ICERR_MEMORY");
		break;

	case ICERR_ERROR :
		strcpy(storage,"ICERR_ERROR");
		break;

	default :
		strcpy(storage,"Defaulted to ICERR_ERROR");
		break;

	}

	return storage;
}




DK_BITMAPINFOHEADER* DXL_VSCREEN_2_BMI
(
 DXL_XIMAGE_HANDLE xImage, 
 DXL_VSCREEN_HANDLE vScreen, 
 DK_BITMAPINFOHEADER *bmih,
 dxvBitDepth* bd1
 )
{
	unsigned char *addr;
	dxvBlitQuality bq;
	dxvBitDepth bd;
	short pitch;
	short height;
	int t;

	DXL_CODEC_HANDLE src = (DXL_CODEC_HANDLE ) xImage;


	duck_memcpy(bmih,&((DXL_CODEC_HANDLE ) xImage)->bihIn,sizeof(DK_BITMAPINFOHEADER));

	DXL_GetVScreenAttributes(vScreen, (void **) &addr, &bq, &bd, &pitch, &height );

	for(t = 0; t < src->maxMaps; t++)
	{
		if (src->maps[t]->bd == bd)
		{
			bmih->biBitCount		= src->maps[t]->biBitCount;
			bmih->biCompression		= src->maps[t]->biCompression;
			bmih->biPlanes			= src->maps[t]->biPlanes;

			bmih->biWidth = pitch / (bmih->biBitCount / 8);
			bmih->biHeight = height;
			bmih->biSizeImage = pitch * bmih->biHeight;

			fprintf(stderr,"\nBMI from VScreen attributes ...\n");
			fprintf(stderr,"\t pitch = %ld\n", pitch);
			fprintf(stderr,"\t width = %ld\n", bmih->biWidth);
			fprintf(stderr,"\t height = %ld\n", bmih->biHeight);
			fprintf(stderr,"\t biCompression = %c%c%c%c\n", 
				((char *) &bmih->biCompression)[0],
				((char *) &bmih->biCompression)[1],
				((char *) &bmih->biCompression)[2],
				((char *) &bmih->biCompression)[3]
			);

			fflush(stderr);

			return bmih;
		}
	}

	*bd1 = bd;

	return 0;
}




int DXL_ReportBestBMIMatch(DXL_XIMAGE_HANDLE xImage, BMIMapping** map, int *maxMaps, int doConsoleReport)
{
	int t;
	int ret;
	char buff[5];
	int len = sizeof(DXL_BMIMap)/sizeof(BMIMapping);
	int matches = 0;
	DXL_CODEC_HANDLE src = (DXL_CODEC_HANDLE ) xImage;
	DK_BITMAPINFOHEADER temp;

	(void)doConsoleReport; //unused

	src->bihIn.dxFlavor = 2;  /* use the extended ICM functions */

	duck_memcpy(&temp,&src->bihIn,sizeof(DK_BITMAPINFOHEADER));

	for(t = 0; t < len; t++)  /* for each one we support with out mapping */
	{

		temp.biBitCount =	DXL_BMIMap[t].biBitCount;
		temp.biCompression = DXL_BMIMap[t].biCompression;
		temp.biPlanes =		DXL_BMIMap[t].biPlanes;
		temp.biSizeImage = temp.biBitCount * temp.biWidth * temp.biHeight / 8;
		ret =  ICDecompressQuery(src->hic, &(src->bihIn), &temp );

		if (ret == ICERR_OK)
		{
			fprintf(stderr,"format of %s supported, planes = %ld, rank = %ld\n",
				MakeFourCCString(temp.biCompression, buff ), temp.biPlanes, matches + 1);
			fflush(stderr);


			if (matches < *maxMaps)
			{
				src->maps[matches] = map[matches] = &DXL_BMIMap[t];
				matches += 1;
			}
		}
		else 
		{
			fprintf(stderr,"format of %s NOT supported, planes = %ld\n",
				MakeFourCCString(temp.biCompression, buff ), temp.biPlanes);
			fflush(stderr);

		}
	} 


	src->maxMaps = *maxMaps = matches;

	/* This could be done somewhere else ! */
	duck_memset(&src->bihOut,0,sizeof(DK_BITMAPINFOHEADER));

	return matches;
}			



static int decompress1(DXL_XIMAGE_HANDLE xImage, DXL_VSCREEN_HANDLE vScreen)
{
	/* Keep the warnings away ! */
	DXL_CODEC_HANDLE src = (DXL_CODEC_HANDLE ) xImage;
	//	DWORD dwFlags = 0;
	DWORD ret;
	dxvBitDepth bd;


	int changeOutput = src->changeVScreen;

	if (changeOutput)
	{

		/* should be cleared first time in so width zero ! */
		if (src->bihOut.biWidth != 0)
			ICDecompressEnd(src->hic);


		if ( DXL_VSCREEN_2_BMI(xImage, vScreen, (DK_BITMAPINFOHEADER *) &(src->bihOut), &bd ) == 0)
		{
			/* user asks for unsupported surface FOURCC */
			fprintf(stderr, "User asks for unsupported dxvBitDepth = %ld\n", bd  );
			fflush(stderr);

			return ICERR_BADFORMAT;
		}


		ret = ICDecompressBegin(src->hic, &src->bihIn,  &src->bihOut);


		if (ret != ICERR_OK)
		{
			return ret;
		}	

	}


	src->bihIn.biSizeImage = src->fSize;

	ret = ICDecompress( src->hic, 0,
		(BITMAPINFOHEADER *) &src->bihIn, src->addr, 
		(BITMAPINFOHEADER *) &src->bihOut, 
		(char *) vScreen->addr);


	if (ret != ICERR_OK)
	{
		fprintf(stderr,"Oh boy decompress may have failed !\n");
		assert(0);
		exit(0);
	}

	src->changeVScreen = 0;

	return ICERR_OK;
}


static int decompress2(DXL_XIMAGE_HANDLE xImage, DXL_VSCREEN_HANDLE vScreen)
{
	/* Keep the warnings away ! */
	DXL_CODEC_HANDLE src = (DXL_CODEC_HANDLE ) xImage;
	DWORD dwFlags = 0;
	DWORD ret;
	dxvBitDepth bd;


	if (src->changeVScreen)
	{

		/* should be cleared first time in so width zero ! */
		if (src->bihOut.biWidth != 0)
			ICDecompressExEnd(src->hic);


		if ( DXL_VSCREEN_2_BMI(xImage, vScreen, (DK_BITMAPINFOHEADER *) &(src->bihOut), &bd ) == 0)
		{
			/* user asks for unsupported surface FOURCC */
			fprintf(stderr, "User asks for unsupported dxvBitDepth = %ld\n", bd  );
			fflush(stderr);

			return ICERR_BADFORMAT;
		}



		ret = ICDecompressExBegin(
			src->hic, 
			dwFlags,
			(BITMAPINFOHEADER *) &(src->bihIn), 
			src->addr,
			0,                    
			0,                    
			src->bihIn.biWidth,                   
			src->bihIn.biHeight,        
			(BITMAPINFOHEADER *) &(src->bihOut),
			(char *) vScreen->addr,
			0,
			0,
			src->bihIn.biWidth,                   
			src->bihIn.biHeight
			);


		if (ret == ICERR_UNSUPPORTED)
		{
			return ICERR_UNSUPPORTED;
		}
		if (ret != ICERR_OK)
		{
			char *storage = (char *) calloc(256,sizeof(char));
			fprintf(stderr,"ICDecompressExBegin returns error code = %ld\n", ret);
			fprintf(stderr,"Decoded as ... %s\n", DXL_DecodeICERR(ret, storage, sizeof(storage) - 1));
			fflush(stderr);

			if (storage)
				free(storage);
			assert(0);
		}	


		src->changeVScreen = 0;
	}


	src->bihIn.biSizeImage = src->fSize;


	ret = ICDecompressEx(
		src->hic,                     
		dwFlags,               
		(BITMAPINFOHEADER *) &src->bihIn,
		src->addr,          
		0,                    
		0,                    
		src->bihIn.biWidth,                   
		src->bihIn.biHeight,                      
		(BITMAPINFOHEADER *) &src->bihOut,
		(char *) vScreen->addr,          
		0,                    
		0,                    
		src->bihIn.biWidth,                   
		src->bihIn.biHeight          
		);



	if (ret != ICERR_OK)
	{
		fprintf(stderr,"Oh boy decompress may have failed !\n");
		assert(0);
		exit(0);
	}


	return 0;
}



static int decompress(DXL_XIMAGE_HANDLE xImage, DXL_VSCREEN_HANDLE vScreen2)
{
	DXL_CODEC_HANDLE xThis = (DXL_CODEC_HANDLE) xImage;

	int retVal = ICERR_OK;

	/* Try the version that handles wack pitch first ! */
	if (xThis->bihIn.dxFlavor == 2)
	{
		retVal = decompress2(xImage, vScreen2);
		if (retVal == ICERR_UNSUPPORTED)
		{
			xThis->bihIn.dxFlavor = 1;
		}
	}

	/* if the wack pitch one failed */
	if (xThis->bihIn.dxFlavor == 1)
	{
		retVal = decompress1(xImage, vScreen2);
	}

	return retVal;
}





/* 
close down a decompressor, releasing the icm decompressor, 
the xImage (decompressor), and the intermediate vScreen (surface)
*/

static int destroyCodec(DXL_XIMAGE_HANDLE xImage)
{
	DXL_CODEC_HANDLE xThis = (DXL_CODEC_HANDLE ) xImage;
	if (xThis)
	{

		if (xThis->hic)
		{

			ICDecompressEnd(xThis->hic);
			ICClose(xThis->hic); 

		}
		duck_free(xThis);

	}

	return DXL_OK;
}




static char* duckToNarrow(char *s)
{
	char dst[256];

	int t=0;

	if (s)
	{
		do 
		{
			dst[t] = s[2*t];
			t = t + 1;
		}
		while ( *((short *) &s[t])   );

		dst[t] = '\0';

		strcpy(s,dst);

		return s;
	}
	else
	{
		return 0;
	}

}  /* end duckToNarrow */





/* 
called during initialization and/or when xImage (decompressor)
attributes change, note that nImage and src are actually
synonymous and should be cleared out a bit (to say the least!)


!!!!!!
This function should be prepared to get data that is NOT of the 
type native to the decoder,  It should do it's best to verify it 
as valid data and should clean up after itself and return NULL
if it doesn't recognize the format of the data
*/

static DXL_XIMAGE_HANDLE reCreateCodec(DXL_CODEC_HANDLE src,unsigned char *data,
																			 int type,enum BITDEPTH bitDepth,int w,int h)
{  
#pragma warning(disable: 4210) // nonstandard extension used : function given file scope
	DXL_XIMAGE_HANDLE createCodec(unsigned char *data);
#pragma warning(default: 4210) // nonstandard extension used : function given file scope

	DXL_XIMAGE_HANDLE nImage;
	DK_BITMAPINFOHEADER *bmiHeader = (DK_BITMAPINFOHEADER *) data;
	unsigned long fccHandler;

	(void)h; // unused
	(void)w; // unused
	(void)bitDepth; //unused
	(void)type; //unused

	if (src != NULL)	/* if an xImage/decompressor already exists, destroy it */
		destroyCodec((DXL_XIMAGE_HANDLE ) src);

	/* create a new xImage, specific to this type of decoder, (
	see "DXL_CODEC" struct above and dxl_main.h) */

	nImage = (DXL_XIMAGE_HANDLE)duck_calloc(1,sizeof(DXL_CODEC),DMEM_GENERAL);
	src = (DXL_CODEC_HANDLE ) nImage;

	if (!nImage) return NULL;

	duck_memset(nImage,0,sizeof(DXL_CODEC));


	src->changeVScreen = 1; /* True ... inform decompresss the dest has changed */


	/* set up the "vtable" of interface calls */
	src->create =  		(DXL_XIMAGE_HANDLE (*)(void *)) createCodec;
	src->recreate =  	(DXL_XIMAGE_HANDLE (*)(DXL_XIMAGE_HANDLE,void *,int,int,int,int)) reCreateCodec;

	nImage->destroy = destroyCodec;
	nImage->dx = 		decompress;
	nImage->blit = 		NULL; /* there is no interleaved blitter for codecs */

	src->bdPrefs = 		bitDepths; /* plug in the list of prefered bit depths */

	nImage->addr = data;
	nImage->dkFlags.inUse = 1;

	duck_memcpy(&src->bihIn, bmiHeader,sizeof(DK_BITMAPINFOHEADER));
	duck_memset(&src->bihOut, 0, sizeof(DK_BITMAPINFOHEADER));

	src->w = (short ) (src->bihIn.biWidth);
	src->h = (short ) (src->bihIn.biHeight);

	src->imWidth = (short) src->w;
	src->imHeight = (short) src->h;

	fccHandler = src->bihIn.fccHandler;

	if (fccHandler == 0)
	{
		src->hic=ICLocate(ICTYPE_VIDEO, fccHandler, (BITMAPINFOHEADER *) &src->bihIn, 0, ICMODE_DECOMPRESS); 
	}
	else
	{
		src->hic=ICOpen(ICTYPE_VIDEO, fccHandler, ICMODE_DECOMPRESS); 
	}




	{
		ICINFO i;

		memset(&i,0,sizeof(ICINFO));
		if (ICGetInfo( src->hic, &i, sizeof(ICINFO) ))
		{

			char temp[5];

			unsigned long biCompression = src->bihIn.biCompression;

			fccHandler = src->bihIn.fccHandler = i.fccHandler;


			fprintf(stderr,	"Short Name : %s\n"
				"Driver : %s\n"
				"driver version = %d %d  or as hex = %x\n"
				"Description : %s\n"
				"Codec biCompression = %s\n"
				"Codec fccHandler = %s\n", 
				duckToNarrow( (char *) i.szName),
				duckToNarrow( (char *) i.szDriver), 
				((i.dwVersion & 0x0000FFFF) >> 0 ),  
				((i.dwVersion & 0xFFFF0000) >> 16 ), 
				i.dwVersion,
				duckToNarrow( (char *) i.szDescription), 
				MakeFourCCString(biCompression, temp),
				MakeFourCCString(fccHandler, temp)
				);
			fflush(stderr);

		}
	}


	if (src->hic == 0)
	{
		destroyCodec((DXL_XIMAGE_HANDLE ) src);

		fprintf(stderr, "codec for fourCC = %c%c%c%c, %x won't open\n",
			(fccHandler & 0xFF000000) >> 24,
			(fccHandler & 0xFF0000) >> 16,
			(fccHandler & 0xFF00) >> 8,
			(fccHandler & 0xFF) >> 0,
			fccHandler
			);
		fflush(stderr);


		return 0;
	}


	return nImage;
}

/* in this "glue" case, just calls through to the create function */

#pragma warning(disable:4211) //nonstandard extension used : redefined extern to static
static DXL_XIMAGE_HANDLE createCodec(unsigned char *bmih)
{
	return reCreateCodec(NULL, bmih ,0,(enum BITDEPTH ) 0,0,0);
}
#pragma warning(default:4211) //nonstandard extension used : redefined extern to static

