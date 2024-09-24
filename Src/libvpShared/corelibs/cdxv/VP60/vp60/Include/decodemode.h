/****************************************************************************
*        
*   Module Title :     decodemode.h
*
*   Description  :     Functions for decoding modes and motionvectors 
*
****************************************************************************/
#ifndef __INC_DECODEMODE_H
#define __INC_DECODEMODE_H

#ifndef STRICT
#define STRICT              /* Strict type checking */
#endif

/****************************************************************************
*  Module statics
****************************************************************************/        
#define MODETYPES       3
#define MODEVECTORS     16
#define PROBVECTORXMIT  174
#define PROBIDEALXMIT   254

/****************************************************************************
*  Typedefs
****************************************************************************/        
typedef struct _modeContext
{
	UINT8 left;
	UINT8 above;
	UINT8 last;
} MODE_CONTEXT;

typedef struct _htorp
{
    unsigned char selector : 1;   // 1 bit selector 0->ptr, 1->token
    unsigned char value : 7;
} torp;

typedef struct _hnode
{
	torp left;
	torp right;
} HNODE;

typedef enum _MODETYPE 
{
	MACROBLOCK,
	NONEAREST_MACROBLOCK,
	NONEAR_MACROBLOCK,
	BLOCK,
} MODETYPE;

/****************************************************************************
*  Exports
****************************************************************************/        
extern UINT8 Stats[9][4][4][4];
extern const UINT8 VP6_ModeVq[MODETYPES][MODEVECTORS][MAX_MODES*2];
extern const UINT8 VP6_BaselineXmittedProbs[4][2][MAX_MODES];

extern void	VP6_BuildModeTree ( PB_INSTANCE *pbi );
extern void VP6_decodeModeAndMotionVector ( PB_INSTANCE *pbi, UINT32 MBrow, UINT32 MBcol );

/****************************************************************************
*  Function Prototypes
****************************************************************************/
INLINE int mbClass(int i);
void VP6_DecodeModeProbs(PB_INSTANCE *pbi);

#endif
