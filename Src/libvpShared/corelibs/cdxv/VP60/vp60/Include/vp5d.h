#ifndef vp5d_h
#define vp5d_h 1

// Interface between vp3d.dll and Albany's DXV adaptor/blitter.
// Timothy S. Murphy 13 September 1999.


// The main object "defined" here.

struct VP3decompressor;


// Some conveniences.

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;


// FourCC codes.  Should agree with microsoft's definition
// sans their stupid types and include files.

typedef ulong FourCC;

#define MakeFourCC( a, b, c, d) ( \
	(ulong) (uchar) a \
	| (ulong) (uchar) b << 8 \
	| (ulong) (uchar) c << 16 \
	| (ulong) (uchar) d << 24 \
)

// A temporary fourCC for Eric & I to use til the bit stream stabilizes.
// (Eric - "hurl4cc" should NOT appear anywhere in your code, I just put it
// here so you can check the fourCC representations in memory and files.)

#define hurl4cc MakeFourCC( 'H', 'U', 'R', 'L')

// The actual fourCC for now; similar remarks apply.

#define VP30 1

#if VP30
#	define wilk4cc MakeFourCC( 'V', 'P', '3', '0')
#else
#	define wilk4cc MakeFourCC( 'W', 'I', 'L', 'K')
#endif


// Array of fourCC codes, has length _and_ is null-terminated.
// As Donald Knuth once said,
// "Some people occasionally like a little extra redundancy sometimes."

typedef struct { const FourCC * codes;  uint numCodes;}  FourCClist;


// YUV buffer configuration.

typedef struct {

	ulong Ywidth, Yheight, UVwidth, UVheight;

	long Ystride, UVstride;

	const uchar *Ybuf, *Ubuf, *Vbuf;

} YUVbufferLayout;


#if __cplusplus
#	define Decompressor VP3decompressor
	extern "C" {
#else
#	define Decompressor struct VP3decompressor
#endif

#if defined(MACPPC)
#define _stdcall 
#endif


// Return array of fourCC codes supported.

const FourCClist * _stdcall VP3DfourCClist();


// Create a decompressor for a particular supported stream type.
// Returns 0 on failure.

Decompressor * _stdcall VP3DcreateDecompressor( FourCC streamType);

void _stdcall VP3DdestroyDecompressor( Decompressor *);


// Advance to next frame, returning reference to updated YUV buffer.

const YUVbufferLayout * _stdcall VP3DnextFrame
( 
	Decompressor *, const uchar * CXdata, ulong CXdataLengthInBytes
);

void _stdcall VP3DblitBGR(
	const Decompressor *, uchar * outRGB, long outStride, long outHeight
);


#if __cplusplus
	}
#endif

#undef Decompressor

#endif	// vp3d_h
