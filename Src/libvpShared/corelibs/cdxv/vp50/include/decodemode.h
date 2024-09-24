/****************************************************************************
*        
*   Module Title :     decodemode.h
*
*   Description  :     functions for decoding modes and motionvectors 
*
*   AUTHOR       :     James Bankoski
*
*****************************************************************************
*   Revision History
*
*   1.00 JBB 30OCT01  New Configuration baseline.
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#ifndef STRICT
#define STRICT              /* Strict type checking. */
#endif

/****************************************************************************
*  Implicit Imports
*****************************************************************************
*/        
extern UINT8 Stats[9][4][4][4];
extern UINT8 NNStats[7][4][4][4];
extern UINT8 NN2Stats[7][4][4][4];
extern UINT8 blockStats[3][4][4][4];

#define MODETYPES 3
#define MODEVECTORS 16
#define PROBVECTORXMIT 174
#define PROBIDEALXMIT 254


/****************************************************************************
*  Exported data structures.
*****************************************************************************
*/        


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
	BLOCK
} MODETYPE;


#ifndef MAPCA
__inline
#endif
    int mbClass(int i);


/****************************************************************************
*  Imports
*****************************************************************************
*/
extern HNODE MBCodingMode[9];
extern HNODE NN2MBCodingMode[8];
extern HNODE NNMBCodingMode[7];
extern HNODE BlockCodingMode[3];
extern UINT8 BaselineXmittedProbs[4][2][MAX_MODES];

/****************************************************************************
*  Function Prototypes
*****************************************************************************
*/
void DecodeModeProbs(PB_INSTANCE *pbi);

extern void FindNearestandNextNearest(PB_INSTANCE* pbi, UINT32 MBrow, UINT32 MBcol,
    MOTION_VECTORA* nearest, MOTION_VECTORA* nextnearest, UINT8 Frame,int *type);

extern void	BuildModeTree(PB_INSTANCE *pbi);
