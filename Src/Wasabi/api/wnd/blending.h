#ifndef _BLENDING_H_
#define _BLENDING_H_

//#include <bfc/common.h>
#include <bfc/platform/types.h>
class Blenders
{
public:
	static void init();
	static unsigned int inline BLEND_ADJ1(unsigned int a, unsigned int b, int alpha);
	static unsigned int inline BLEND_ADJ2(unsigned int a, unsigned int b);
	static unsigned int inline BLEND_ADJ3(unsigned int a, unsigned int b, int alpha);
	static unsigned int inline BLEND_MUL(unsigned int a, int v);
	static unsigned int inline BLEND_AVG(unsigned int a, unsigned int b);
	static unsigned int inline BLEND4(unsigned int *p1, unsigned int w, int xp, int yp);

#ifndef NO_MMX
	static int inline MMX_AVAILABLE() { return mmx_available; }
	static unsigned int inline BLEND_ADJ1_MMX(unsigned int a, unsigned int b, int alpha);
	static unsigned int inline BLEND_ADJ2_MMX(unsigned int a, unsigned int b);
	static unsigned int inline BLEND_ADJ3_MMX(unsigned int a, unsigned int b, int alpha);
	static unsigned int inline BLEND_MUL_MMX(unsigned int a, int v);
	static unsigned int inline BLEND_AVG_MMX(unsigned int a, unsigned int b);
	static unsigned int inline BLEND4_MMX(unsigned int *p1, unsigned int w, int xp, int yp);
	static void inline BLEND_MMX_END()
	{
#ifdef WIN32
		if (mmx_available) __asm emms;
#endif
#ifdef LINUX
if (mmx_available) __asm__ volatile ( "emms" : : );
#endif

	}
#endif

//private:
	static uint8_t alphatable[256][256];
#ifndef NO_MMX
	static int mmx_available;
#endif
};


// NON MMX

// average blend of a and b.
unsigned int inline Blenders::BLEND_AVG(unsigned int a, unsigned int b)
{
	return ((a >> 1)&~((1 << 7) | (1 << 15) | (1 << 23))) + ((b >> 1)&~((1 << 7) | (1 << 15) | (1 << 23)));
}


// multiplies 32 bit color A by scalar V (0-255)
unsigned int inline Blenders::BLEND_MUL(unsigned int a, int v)
{
	register int t;
	t = Blenders::alphatable[a & 0xFF][v];
	t |= Blenders::alphatable[(a & 0xFF00) >> 8][v] << 8;
	t |= Blenders::alphatable[(a & 0xFF0000) >> 16][v] << 16;
	t |= Blenders::alphatable[(a & 0xFF000000) >> 24][v] << 24;
	return t;
}


// V is scalar (0-255), (1.0-V)*b + V*a
unsigned int inline Blenders::BLEND_ADJ1(unsigned int a, unsigned int b, int v)
{
	register int t;
	t = Blenders::alphatable[b & 0xFF][0xFF - v] + Blenders::alphatable[a & 0xFF][v];
	t |= (Blenders::alphatable[(b & 0xFF00) >> 8][0xFF - v] + Blenders::alphatable[(a & 0xFF00) >> 8][v]) << 8;
	t |= (Blenders::alphatable[(b & 0xFF0000) >> 16][0xFF - v] + Blenders::alphatable[(a & 0xFF0000) >> 16][v]) << 16;
	t |= (Blenders::alphatable[(b & 0xFF000000) >> 24][0xFF - v] + Blenders::alphatable[(a & 0xFF000000) >> 24][v]) << 24;
	return t;
}

// returns a*(1.0-Alpha(b)) + b
unsigned int inline Blenders::BLEND_ADJ2(unsigned int a, unsigned int b)
{
	register int t, z;
	int v = 0xff - ((b >> 24) & 0xff);
	t = Blenders::alphatable[a & 0xFF][v] + (b & 0xFF);
	if (t > 0xFF) t = 0xff;
	z = (Blenders::alphatable[(a & 0xFF00) >> 8][v] << 8) + (b & 0xFF00);
	if (z > 0xFF00) z = 0xff00;
	t |= z;
	z = (Blenders::alphatable[(a & 0xFF0000) >> 16][v] << 16) + ((b & 0xFF0000));
	if (z > 0xFF0000) z = 0xff0000;
	t |= z;
	z = (Blenders::alphatable[(a & 0xFF000000) >> 24][v]) + ((b & 0xFF000000) >> 24);
	if (z > 0xFF) z = 0xff;
	return t | (z << 24);
}

// returns a*(1-Alpha(b)*W) + b*W, clamped (W is scalar 0-0xff).
unsigned int inline Blenders::BLEND_ADJ3(unsigned int a, unsigned int b, int w)
{
	register int t, z;
	int v = 0xff - Blenders::alphatable[(b >> 24) & 0xff][w];

	t = Blenders::alphatable[a & 0xFF][v] + Blenders::alphatable[b & 0xFF][w];
	if (t > 0xFF) t = 0xFF;
	z = Blenders::alphatable[(a & 0xFF00) >> 8][v] + Blenders::alphatable[(b & 0xFF00) >> 8][w];
	if (z > 0xFF) z = 0xFF;
	t |= z << 8;
	z = Blenders::alphatable[(a & 0xFF0000) >> 16][v] + Blenders::alphatable[(b & 0xFF0000) >> 16][w];
	if (z > 0xFF) z = 0xFF;
	t |= z << 16;
	z = Blenders::alphatable[(a & 0xFF000000) >> 24][v] + Blenders::alphatable[(b & 0xFF000000) >> 24][w];
	if (z > 0xFF) z = 0xFF;
	return t | (z << 24);
}

unsigned int __inline Blenders::BLEND4(unsigned int *p1, unsigned int w, int xp, int yp)
{
	register int t;
	uint8_t a1, a2, a3, a4;
	xp = (xp >> 8) & 0xff;
	yp = (yp >> 8) & 0xff;
	a1 = alphatable[255 - xp][255 - yp];
	a2 = alphatable[xp][255 - yp];
	a3 = alphatable[255 - xp][yp];
	a4 = alphatable[xp][yp];
	t = alphatable[p1[0] & 0xff][a1] + alphatable[p1[1] & 0xff][a2] + alphatable[p1[w] & 0xff][a3] + alphatable[p1[w + 1] & 0xff][a4];
	t |= (alphatable[(p1[0] >> 8) & 0xff][a1] + alphatable[(p1[1] >> 8) & 0xff][a2] + alphatable[(p1[w] >> 8) & 0xff][a3] + alphatable[(p1[w + 1] >> 8) & 0xff][a4]) << 8;
	t |= (alphatable[(p1[0] >> 16) & 0xff][a1] + alphatable[(p1[1] >> 16) & 0xff][a2] + alphatable[(p1[w] >> 16) & 0xff][a3] + alphatable[(p1[w + 1] >> 16) & 0xff][a4]) << 16;
	t |= (alphatable[(p1[0] >> 24) & 0xff][a1] + alphatable[(p1[1] >> 24) & 0xff][a2] + alphatable[(p1[w] >> 24) & 0xff][a3] + alphatable[(p1[w + 1] >> 24) & 0xff][a4]) << 24;
	return t;
}




#ifndef NO_MMX


#ifdef WIN32
#pragma warning( push, 1 )
#pragma warning(disable: 4799)
#endif

#ifdef WIN32
#define MMX_CONST const
#else
#define MMX_CONST
#endif

static unsigned int MMX_CONST Blenders__mmx_revn2[2] = {0x01000100, 0x01000100};
static unsigned int MMX_CONST Blenders__mmx_zero[2];
static unsigned int MMX_CONST Blenders__mmx_one[2] = {1, 0};

#undef MMX_CONST

/// MMX

// average blend of a and b.
unsigned int inline Blenders::BLEND_AVG_MMX(unsigned int a, unsigned int b)
{
	return ((a >> 1)&~((1 << 7) | (1 << 15) | (1 << 23))) + ((b >> 1)&~((1 << 7) | (1 << 15) | (1 << 23)));
}

// multiplies 32 bit color A by scalar V (0-255)
unsigned int inline Blenders::BLEND_MUL_MMX(unsigned int a, int v)
{
#ifdef WIN32
	__asm
	{
	    movd mm3, [v] // VVVVVVVV

	    movd mm0, [a]
	    packuswb mm3, mm3 // 0000HHVV

	    punpcklbw mm0, [Blenders__mmx_zero]
	    punpcklwd mm3, mm3 // HHVVHHVV

	    punpckldq mm3, mm3 // HHVVHHVV HHVVHHVV

	    pmullw mm0, mm3

	    psrlw mm0, 8

	    packuswb mm0, mm0

	    movd eax, mm0
	}
#else
	__asm__ volatile (
		"movd %0, %%mm3\n"
		"movd %1, %%mm0\n"
		"packuswb %%mm3, %%mm3\n"
		"punpcklbw (Blenders__mmx_zero), %%mm0\n"
		"punpcklwd %%mm3, %%mm3\n"
		"punpckldq %%mm3, %%mm3\n"
		"pmullw %%mm3, %%mm0\n"
		"psrlw $8, %%mm0\n"
		"packuswb %%mm0, %%mm0\n"
		"movd %%mm0, %%eax\n"
	:
	: "m" (v), "m" (a)
				: "%mm0", "%mm3" );
#endif
}


// V is scalar (0-255), (1.0-V)*b + V*a
unsigned int inline Blenders::BLEND_ADJ1_MMX(unsigned int a, unsigned int b, int v)
{
#ifdef WIN32
	__asm
	{
	    movd mm3, [v] // VVVVVVVV

	    movd mm0, [a]
	    packuswb mm3, mm3 // 0000HHVV

	    movd mm1, [b]
	    paddusw mm3, [Blenders__mmx_one]

	    movq mm4, [Blenders__mmx_revn2]
	    punpcklwd mm3, mm3 // HHVVHHVV

	    punpcklbw mm0, [Blenders__mmx_zero]
	    punpckldq mm3, mm3 // HHVVHHVV HHVVHHVV

	    punpcklbw mm1, [Blenders__mmx_zero]
	    psubw mm4, mm3

	    pmullw mm0, mm3
	    pmullw mm1, mm4

	    paddw mm0, mm1

	    psrlw mm0, 8

	    packuswb mm0, mm0

	    movd eax, mm0
	}
#else
	__asm__ volatile (
		"movd %0, %%mm3\n"
		"movd %1, %%mm0\n"
		"packuswb %%mm3, %%mm3\n"
		"movd %2, %%mm1\n"
		"paddusw (Blenders__mmx_one), %%mm3\n"
		"movq (Blenders__mmx_revn2), %%mm4\n"
		"punpcklwd %%mm3, %%mm3\n"
		"punpcklbw (Blenders__mmx_zero), %%mm0\n"
		"punpckldq %%mm3, %%mm3\n"
		"punpcklbw (Blenders__mmx_zero), %%mm1\n"
		"psubw %%mm3, %%mm4\n"
		"pmullw %%mm3, %%mm0\n"
		"pmullw %%mm4, %%mm1\n"
		"paddw %%mm1, %%mm0\n"
		"psrlw $8, %%mm0\n"
		"packuswb %%mm0, %%mm0\n"
		"movd %%mm0, %%eax\n"
	:
	: "m" (v), "m" (a), "m" (b)
				: "%mm0", "%mm1", "%mm3", "%mm4" );
#endif
}

// returns a*(1.0-Alpha(b)) + b
unsigned int inline Blenders::BLEND_ADJ2_MMX(unsigned int a, unsigned int b)
{
#ifdef WIN32
	__asm
	{
	    movd mm3, [b] // VVVVVVVV
	    movq mm4, [Blenders__mmx_revn2]

	    movd mm0, [a]
	    psrld mm3, 24

	    movd mm1, [b]
	    paddusw mm3, [Blenders__mmx_one]

	    punpcklwd mm3, mm3 // HHVVHHVV
	    punpcklbw mm0, [Blenders__mmx_zero]

	    punpckldq mm3, mm3 // HHVVHHVV HHVVHHVV
	    punpcklbw mm1, [Blenders__mmx_zero]

	    psubw mm4, mm3

	    pmullw mm0, mm4
	    // stall

	    // stall

	    // stall

	    psrlw mm0, 8
	    // stall

	    paddw mm0, mm1
	    // stall

	    packuswb mm0, mm0
	    // stall

	    movd eax, mm0
	}
#else
	__asm__ volatile (
		"movd %1, %%mm3\n"
		"movq (Blenders__mmx_revn2), %%mm4\n"
		"movd %0, %%mm0\n"
		"psrld $24, %%mm3\n"
		"movd %1, %%mm1\n"
		"paddusw (Blenders__mmx_one), %%mm3\n"
		"punpcklwd %%mm3, %%mm3\n"
		"punpcklbw (Blenders__mmx_zero), %%mm0\n"
		"punpckldq %%mm3, %%mm3\n"
		"punpcklbw (Blenders__mmx_zero), %%mm1\n"
		"psubw %%mm3, %%mm4\n"
		"pmullw %%mm4, %%mm0\n"
		"psrlw $8, %%mm0\n"
		"paddw %%mm1, %%mm0\n"
		"packuswb %%mm0, %%mm0\n"
		"movd %%mm0, %%eax\n"
	:
	: "m" (a), "m" (b)
				: "%esi", "%mm0", "%mm1", "%mm3", "%mm4" );
#endif
}

// returns a*(1-Alpha(b)*W) + b*W, clamped (W is scalar 0-0xff).
unsigned int inline Blenders::BLEND_ADJ3_MMX(unsigned int a, unsigned int b, int w)
{
#ifdef WIN32
	__asm
	{
	    movd mm3, [b] // VVVVVVVV
	    movd mm5, [w]

	    movd mm0, [a]
	    psrld mm3, 24

	    movd mm1, [b]
	    paddusw mm3, [Blenders__mmx_one]

	    movq mm4, [Blenders__mmx_revn2]
	    pmullw mm3, mm5

	    packuswb mm5, mm5
	    punpcklbw mm0, [Blenders__mmx_zero]

	    punpcklwd mm5, mm5
	    punpcklbw mm1, [Blenders__mmx_zero]

	    psrlw mm3, 8
	    punpckldq mm5, mm5

	    paddusw mm3, [Blenders__mmx_one]

	    punpcklwd mm3, mm3 // HHVVHHVV

	    punpckldq mm3, mm3 // HHVVHHVV HHVVHHVV


	    psubw mm4, mm3

	    pmullw mm0, mm4
	    pmullw mm1, mm5

	    paddusw mm0, mm1

	    psrlw mm0, 8

	    packuswb mm0, mm0

	    movd eax, mm0
	}
#else
	__asm__ volatile (
		"movd %2, %%mm3\n"
		"movd %0, %%mm5\n"
		"movd %1, %%mm0\n"
		"psrld $24, %%mm3\n"
		"movd %2, %%mm1\n"
		"paddusw (Blenders__mmx_one), %%mm3\n"
		"movq (Blenders__mmx_revn2), %%mm4\n"
		"pmullw %%mm5, %%mm3\n"
		"packuswb %%mm5, %%mm5 \n"
		"punpcklbw (Blenders__mmx_zero), %%mm0\n"
		"punpcklwd %%mm5, %%mm5\n"
		"punpcklbw (Blenders__mmx_zero), %%mm1\n"
		"psrlw $8, %%mm3\n"
		"punpckldq %%mm5, %%mm5\n"
		"paddusw (Blenders__mmx_one), %%mm3\n"
		"punpcklwd %%mm3, %%mm3\n"
		"punpckldq %%mm3, %%mm3\n"
		"psubw %%mm3, %%mm4\n"
		"pmullw %%mm4, %%mm0\n"
		"pmullw %%mm5, %%mm1\n"
		"paddusw %%mm1, %%mm0\n"
		"psrlw $8, %%mm0\n"
		"packuswb %%mm0, %%mm0\n"
		"movd %%mm0, %%eax\n"

	:
	: "m" (w), "m" (a), "m" (b)
				: "%mm0", "%mm1", "%mm4", "%mm3", "%mm5" );
#endif
}

// does bilinear filtering. p1 is upper left pixel, w is width of framebuffer
// xp and yp's low 16 bits are used for the subpixel positioning.
unsigned int inline Blenders::BLEND4_MMX(unsigned int *p1, unsigned int w, int xp, int yp)
{
#ifdef WIN32
	__asm
	{
	    movd mm6, xp
	    mov eax, p1

	    movd mm7, yp
	    mov esi, w

	    movq mm4, Blenders__mmx_revn2
	    psrlw mm6, 8

	    movq mm5, Blenders__mmx_revn2
	    psrlw mm7, 8

	    movd mm0, [eax]
	    punpcklwd mm6, mm6

	    movd mm1, [eax + 4]
	    punpcklwd mm7, mm7

	    movd mm2, [eax + esi*4]
	    punpckldq mm6, mm6

	    movd mm3, [eax + esi*4 + 4]
	    punpckldq mm7, mm7

	    punpcklbw mm0, [Blenders__mmx_zero]
	    psubw mm4, mm6

	    punpcklbw mm1, [Blenders__mmx_zero]
	    pmullw mm0, mm4

	    punpcklbw mm2, [Blenders__mmx_zero]
	    pmullw mm1, mm6

	    punpcklbw mm3, [Blenders__mmx_zero]
	    psubw mm5, mm7

	    pmullw mm2, mm4
	    pmullw mm3, mm6

	    paddw mm0, mm1
	    // stall (mm0)

	    psrlw mm0, 8
	    // stall (waiting for mm3/mm2)

	    paddw mm2, mm3
	    pmullw mm0, mm5

	    psrlw mm2, 8
	    // stall (mm2)

	    pmullw mm2, mm7
	    // stall

	    // stall (mm2)

	    paddw mm0, mm2
	    // stall

	    psrlw mm0, 8
	    // stall

	    packuswb mm0, mm0
	    // stall

	    movd eax, mm0
	}
#else
	__asm__ volatile (
		"movd %2, %%mm6\n"
		"mov %0, %%eax\n"
		"movd %3, %%mm7\n"
		"mov %1, %%esi\n"
		"movq (Blenders__mmx_revn2), %%mm4\n"
		"psrlw $8, %%mm6\n"
		"movq (Blenders__mmx_revn2), %%mm5\n"
		"psrlw $8, %%mm7\n"
		"movd (%%eax), %%mm0\n"
		"punpcklwd %%mm6,%%mm6\n"
		"movd 4(%%eax), %%mm1\n"
		"punpcklwd %%mm7,%%mm7\n"
		"movd (%%eax,%%esi,4), %%mm2\n"
		"punpckldq %%mm6,%%mm6\n"
		"movd 4(%%eax,%%esi,4), %%mm3\n"
		"punpckldq %%mm7,%%mm7\n"
		"punpcklbw (Blenders__mmx_zero), %%mm0\n"
		"psubw %%mm6, %%mm4\n"
		"punpcklbw (Blenders__mmx_zero), %%mm1\n"
		"pmullw %%mm4, %%mm0\n"
		"punpcklbw (Blenders__mmx_zero), %%mm2\n"
		"pmullw %%mm6, %%mm1\n"
		"punpcklbw (Blenders__mmx_zero), %%mm3\n"
		"psubw %%mm7, %%mm5\n"
		"pmullw %%mm4, %%mm2\n"
		"pmullw %%mm6, %%mm3\n"
		"paddw %%mm1, %%mm0\n"
		"psrlw $8, %%mm0\n"
		"paddw %%mm3, %%mm2\n"
		"pmullw %%mm5, %%mm0\n"
		"psrlw $8, %%mm2\n"
		"pmullw %%mm7, %%mm2\n"
		"paddw %%mm2, %%mm0\n"
		"psrlw $8, %%mm0\n"
		"packuswb %%mm0, %%mm0\n"
		"movd %%mm0, %%eax\n"

	:
	: "m" (p1), "m" (w), "m" (xp), "m" (yp)
				: "%mm0", "%mm1", "%mm4", "%mm3", "%mm5" );

#endif
}

#ifdef WIN32
#pragma warning( pop )
#endif

#endif	// ndef NO_MMX


#endif
