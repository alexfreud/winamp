//==========================================================================
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1999 - 2001  On2 Technologies Inc. All Rights Reserved.
//
//--------------------------------------------------------------------------


#ifndef _CCLIB_H
#define _CCLIB_H
#include "cpuidlib.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * **-CCLIB.H
 *
 * ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage **
 * ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage **
 * ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage **
 *
 *
 *
 *   The library contains color space conversion functions.  The proper way to use this library is to
 *   call InitCCLib with a value of "SpecialProc" BEFORE attempting any color space conversions.  DeInitCCLib
 *   should be called when you are done with the libary.  It will preform any clean up that is necessary.
 *
 *
 *
 *
 * ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage **
 * ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage **
 * ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage  ** Usage **
 */



/*
 * **-InitCCLib
 *
 * This function MUST be called before attempting to use any of the functions in the library.
 * This function will initilize all the function pointers to point to valid routines.
 *
 * Assumptions:
 *   Assumes that it is safe to write to the function pointers.  
 *
 * Input:
 *   CpuType - If CpuType type is set to "SpecialProc" the code will autodetect the CPU and initilize the function
 *             pointers appropiatly.  If CpuType is set to any other value it will assume that that was the CPUType
 *             detected.  NOTE: You should be careful when forcing the CPU to a specific type.  If you force the
 *             CPU type to one that is not valid for your system you will most likely crash.
 *
 * Output:
 *    Return Non-Zero value if there was a problem initilizing the function pointers
 *
 *    Function pointers RGB32toYV12FuncPtr
 *                      RGB24toYV12FuncPtr
 *                      YVYUtoYV12FuncPtr
 *
 *    Initilized to point to the proper routines for this system
 */
int InitCCLib( PROCTYPE CpuType );


/*
 * **-DeInitCCLib
 *
 * You should call this function when you are done using the color conversion library.
 *
 * Assumptions:
 *   You are done with the color conversion library and would like it to clean up after itself
 *
 * Input:
 *   None
 *
 * Output:
 *   No explicit return value
 *
 *   color conversion library cleaned up
 */
void DeInitCCLib( void );

/*
 * *** N O T E *** N O T E *** *** N O T E *** N O T E *** *** N O T E *** N O T E *** *** N O T E *** N O T E *** *** N O T E *** N O T E ***
 *
 *
 *                There are macros below to reduce the pain needed to use these functions
 *
 *
 * *** N O T E *** N O T E *** *** N O T E *** N O T E *** *** N O T E *** N O T E *** *** N O T E *** N O T E *** *** N O T E *** N O T E ***
 */

/*
 * **-RGB32toYV12FuncPtr
 *
 * This function pointer points to the fastest version of the function that will convert a RGB32 buffer to planer YV12 output
 * Alpha is ignored.
 *
 * InitCCLib MUST be called before using this function pointer or you will go off into the weeds.
 *
 * Inputs:
 *       RGBABuffer   - Pointer to buffer containing RGB data.  We assume that data looks like
 *       
 *                                     +---+---+---+---+---+---+---+---+
 *                     Memory Address  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
 *                                     +---+---+---+---+---+---+---+---+
 *                           Contents  | B | G | R | A | B | G | R | A |
 *                                     +---+---+---+---+---+---+---+---+
 *
 *       ImageWidth  - Width (in pixels) of the image to be processed
 *
 *       ImageHeight - Height (in pixels) of the image to be processed
 *       
 *       YBuffer     - Pointer to buffer where we should place the converted Y data.  The caller needs to
 *                     ensure that sufficent memory is allocated.  We do not check.
 *                   
 *       UBuffer     - Pointer to buffer where we should place the converted U data.  The caller needs to
 *                     ensure that sufficent memory is allocated.  We do not check.
 *                   
 *       VBuffer     - Pointer to buffer where we should place the converted U data.  The caller needs to
 *                     ensure that sufficent memory is allocated.  We do not check.
 *  
 * Outputs:
 *       YBuffer     - Buffer filled with RGB data converted to YV12 format
 *
 *       UBuffer     - Buffer filled with RGB data converted to YV12 format
 *
 *       VBuffer     - Buffer filled with RGB data converted to YV12 format
 *
 * Assumptions:
 *       Assumes that InitCCLib has been called to initilize this function pointer
 *
 *       We assume that the width and height of the image passed in is even.  If it is not
 *       the last line and column will get bad U and V values.  This is due to us averging
 *       4x4 block to get U and V values.
 *
 * Formulas:
 *       Cb = U
 *       Cr = V
 *
 *       Y  =  0.257R + 0.504G + 0.098B + 16
 *       Cb = -0.148R - 0.291G + 0.439B + 128
 *       Cr =  0.439R - 0.368G - 0.071B + 128
 *
 * The formulas above were obtained from the book Video Demistyfied.
 *
 * The YV12 format drops every other U and V across and every other U, V vertical line.
 * To calculate U and V we will average the 4 RGB values before we convert to U and V.
 * This is slightly less accurate than converting the 4 RGB values to 4 U and V values
 * and then averaging the U and V values.  The plus side of averaging before is that
 * we the coversion is about 10% faster than if we were to convert the values and then 
 * average.
 *
 * We process the image in 2x2 blocks.  From left to right then from top to bottom.
 * Given the following image we will process it in the following order
 *
 *  1) (0,0), (0,1), (1,0), (1,1)
 *  2) (0,2), (0,3), (1,2), (1,3)
 *  3) (2,0), (2,1), (2,2), (2,3)
 *  4) (3,0), (3,1), (3,2), (3,3)
 *
 *    +-----+-----+-----+-----+
 *    | 0,0 | 0,1 | 0,2 | 0,3 |
 *    +-----+-----+-----+-----+
 *    | 1,0 | 1,1 | 1,2 | 1,3 |
 *    +-----+-----+-----+-----+
 *    | 2,0 | 2,1 | 2,2 | 2,3 |
 *    +-----+-----+-----+-----+
 *    | 3,0 | 3,1 | 3,2 | 3,3 |
 *    +-----+-----+-----+-----+
 *
 * To try and avoid rounding errors we are going to scale the number and only
 * convert when we write the number to memory.
 *
 * When we finally scale the numbers down we will round values with fractions
 * greater than .5 up and less than .5 down.  To achieve this we add in a round
 * factor which is equal to half of the amount that we divide by.
 *
 * The values that this function generates for Y, Cr, Cb are very accurate.
 * Utilizing double precision floating point will not generate more accurate
 * results.
 *
 * When converting from the 32-bit Y, Cb, Cr to the 8-bit Y, Cb, Cr values we do
 * not need to worry about over flowing the 8-bit value.  Using the worst R, G, B
 * values we get the following Min and Max values for Y, Cb, Cr.
 *
 *        +=====+=====+=====++=====+=====+=====++=========+
 *        |  R	|  G  |  B  ||  Y  | Cb  | Cr  ||         |
 *        +=====+=====+=====++=====+=====+=====++=========+
 *        | 255 | 255 | 0   || 210 | 16  | 146 || Min Cb  |
 *        +-----+-----+-----++-----+-----+-----++---------+
 *        | 0	| 0   | 255 || 40  | 239 | 109 || Max Cb  |
 *        +-----+-----+-----++-----+-----+-----++---------+
 *        | 0	| 255 | 255 || 169 | 165 | 16  || Min Cr  |
 *        +-----+-----+-----++-----+-----+-----++---------+
 *        | 255 | 0   | 0   || 81  | 90  | 239 || Max Cr  |
 *        +-----+-----+-----++-----+-----+-----++---------+
 *        | 0	| 0   | 0   || 16  | 128 | 128 || Min Y   |
 *        +-----+-----+-----++-----+-----+-----++---------+
 *        | 255 | 255 | 255 || 235 | 128 | 128 || Max Y   |
 *        +-----+-----+-----++-----+-----+-----++---------+
 *
 * 
 */
extern void (*RGB32toYV12FuncPtr)( unsigned char *RGBABuffer, int ImageWidth, int ImageHeight,
                                   unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer );

/*
 * **-RGB24toYV12FuncPtr
 *
 * This function is 99.99% the same as CC_RGB32toYV12 see comments for CC_RGB32toYV12 if you want to know how this
 * function works.  The only difference from CC_RGB32toYV12 is we assume that
 * the input buffer is of the RGB 24 format given below.
 *
 *                                     +---+---+---+---+---+---+---+---+
 *                     Memory Address  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
 *                                     +---+---+---+---+---+---+---+---+
 *                           Contents  | B | G | R | B | G | R | B | G |
 *                                     +---+---+---+---+---+---+---+---+
 *
 */
extern void (*RGB24toYV12FuncPtr)( unsigned char *RGBBuffer, int ImageWidth, int ImageHeight,
                                   unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer );

/*
 * **-YVYUtoYV12FuncPtr
 *
 * This function pointer points to the fastest version of the following function that will run on
 * this system.
 *
 * InitCCLib MUST be called before trying to use this pointer.  If you do not you will be in the
 * weeds
 *
 * The function will convert a YVYU (a.k.a. YUV 4:2:2) format YUV buffer to YV12 format buffer.
 * The YVYU format has two lines of U and V data per two lines of Y data.  The YV12 format only
 * has one line of U, V data per two lines of Y data.  To fit the extra U, V data into a single U, V
 * line we will average the two U, V lines.
 *
 * Example:
 *               +--------+--------+--------+--------+--------+--------+--------+--------+-----+
 *               | Y(0,0) | U(0,0) | Y(0,1) | V(0,0) | Y(0,2) | U(0,1) | Y(0,1) | V(0,1) | ... |
 *               +--------+--------+--------+--------+--------+--------+--------+--------+-----+
 *               | Y(1,0) | U(1,0) | Y(1,1) | V(1,0) | Y(1,2) | U(1,1) | Y(1,1) | V(1,1) | ... |
 *               +--------+--------+--------+--------+--------+--------+--------+--------+-----+
 *               | Y(2,0) | U(2,0) | Y(2,1) | V(2,0) | Y(2,2) | U(2,1) | Y(2,1) | V(2,1) | ... |
 *               +--------+--------+--------+--------+--------+--------+--------+--------+-----+
 *               | Y(3,0) | U(3,0) | Y(3,1) | V(3,0) | Y(3,2) | U(3,1) | Y(3,1) | V(3,1) | ... |
 *               +--------+--------+--------+--------+--------+--------+--------+--------+-----+
 *               |  ...   |  ...   |  ...   |  ...   |  ...   |  ...   |  ...   |  ...   | ... |
 *               +--------+--------+--------+--------+--------+--------+--------+--------+-----+
 *
 *                                                  
 *                                                  ==
 *
 *               +--------+--------+--------+--------+-----+
 *               | Y(0,0) | Y(0,1) | Y(0,2) | Y(0,1) | ... |
 *               +--------+--------+--------+--------+-----+
 *               | Y(1,0) | Y(1,1) | Y(1,2) | Y(1,1) | ... |
 *               +--------+--------+--------+--------+-----+
 *               | Y(2,0) | Y(2,1) | Y(2,2) | Y(2,1) | ... |
 *               +--------+--------+--------+--------+-----+
 *               | Y(3,0) | Y(3,1) | Y(3,2) | Y(3,1) | ... |
 *               +--------+--------+--------+--------+-----+
 *               |  ...   |  ...   |  ...   |  ...   | ... |
 *               +--------+--------+--------+--------+-----+
 *
 *
 *               +--------------------+--------------------+------+
 *               | AVG[U(0,0),U(1,0)] | AVG[U(0,1),U(1,1)] |  ... |
 *               +--------------------+--------------------+------+
 *               | AVG[U(2,0),U(3,0)] | AVG[U(2,1),U(3,1)] |  ... |
 *               +--------------------+--------------------+------+
 *               |        ...         |       ...          |  ... |
 *               +--------------------+--------------------+------+
 *
 *
 *               +--------------------+--------------------+------+
 *               | AVG[V(0,0),U(1,0)] | AVG[V(0,1),U(1,1)] |  ... |
 *               +--------------------+--------------------+------+
 *               | AVG[V(2,0),U(3,0)] | AVG[V(2,1),U(3,1)] |  ... |
 *               +--------------------+--------------------+------+
 *               |        ...         |       ...          |  ... |
 *               +--------------------+--------------------+------+
 *
 * A single pass of the core look will process two horizontal lines of the image at once.
 * The makes it easier to average the U and V values.
 *
 *
 * Inputs:
 *  YVYUBuffer - Pointer to buffer containing YVYU data.  We assume that the data looks like
 *
 *                                     +---+---+---+---+---+---+---+---+
 *                     Memory Address  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
 *                                     +---+---+---+---+---+---+---+---+
 *                           Contents  | Y | V | Y | U | Y | V | Y | U |
 *                                     +---+---+---+---+---+---+---+---+
 *
 *       ImageWidth  - Width (in pixels) of the image to be processed
 *
 *       ImageHeight - Height (in pixels) of the image to be processed
 *       
 *       YBuffer     - Pointer to buffer where we should place the converted Y data.  The caller needs to
 *                     ensure that sufficent memory is allocated.  We do not check.
 *                   
 *       UBuffer     - Pointer to buffer where we should place the converted U data.  The caller needs to
 *                     ensure that sufficent memory is allocated.  We do not check.
 *                   
 *       VBuffer     - Pointer to buffer where we should place the converted U data.  The caller needs to
 *                     ensure that sufficent memory is allocated.  We do not check.
 *  
 * Outputs:
 *       YBuffer     - Buffer filled with YVYU data converted to YV12 format
 *
 *       UBuffer     - Buffer filled with YVYU data converted to YV12 format
 *
 *       VBuffer     - Buffer filled with YVYU data converted to YV12 format
 *
 * Assumptions:
 *       Assumes that InitCCLib has been called to initilize this function pointer
 *
 *       Height of the image that we are processing is assumed to be even.  If
 *       the height is not even the last line of the image will be corrupted.
 *
 *       For the C version the width of the image must be a multiple of two.  For
 *       the assembly version the width of the image must be a multiple of 8.
 *  
 */
extern void (*YVYUtoYV12FuncPtr)( unsigned char *YVYUBuffer, int ImageWidth, int ImageHeight,
                                  unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer );


/*
 * Macros to make it easier to call the needed functions
 */
#define CC_RGB32toYV12( _RGBABuffer, _ImageWidth, _ImageHeight, _YBuffer, _UBuffer, _VBuffer ) \
        (*RGB32toYV12FuncPtr)( _RGBABuffer, _ImageWidth, _ImageHeight, _YBuffer, _UBuffer, _VBuffer )
                      
#define CC_RGB24toYV12( _RGBBuffer, _ImageWidth, _ImageHeight, _YBuffer, _UBuffer, _VBuffer ) \
        (*RGB24toYV12FuncPtr)( _RGBBuffer, _ImageWidth, _ImageHeight, _YBuffer, _UBuffer, _VBuffer )

#define CC_YVYUtoYV12( _YVYUBuffer, _ImageWidth, _ImageHeight, _YBuffer, _UBuffer, _VBuffer ) \
        (*YVYUtoYV12FuncPtr)( _YVYUBuffer, _ImageWidth, _ImageHeight, _YBuffer, _UBuffer, _VBuffer )


void ConvertRGBtoYUV(
	unsigned char *r_src,unsigned char *g_src,unsigned char *b_src, 
	int width, int height, int rgb_step, int rgb_pitch,
	unsigned char *y_src, unsigned char *u_src, unsigned char *v_src,  
	int uv_width_shift, int uv_height_shift,
	int y_step, int y_pitch,int uv_step,int uv_pitch
	);

#ifdef __cplusplus
}
#endif
#endif /* _CCLIB_H */
