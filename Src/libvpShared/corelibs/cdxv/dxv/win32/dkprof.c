/***********************************************\
??? dkprof.c
? profiling functions
? also see perf.asm and pentium.asm
\***********************************************/
#include "duck_mem.h"
#include "dkprof.h"

#define MAX_PROFILE 15

int profStarted = 0;
 
enum PROFILESECTION { 
    LOSSLESSDX = 0, 
    PLANARDX, 
    BLITME,
    RD_FRAME_DESC,
    RASTER_CONFIG,
    DELTA_TABLES,
    HANDLER_CONFIG, 
    STRING_DECODER,
    STRING_DATA,
    TSC0,
    TSC1,
    TSC2,
    TSC3
};

PSECTION pSectionArray[MAX_PROFILE];

unsigned long pentiumKiloCycles(void);

#if 1
/***********************************************/
void tscStart(enum PROFILESECTION sel) 
{
    PSECTION *pSection;

    if(profStarted) {
        pSection = &pSectionArray[sel];
        pSection->pkc1 = pentiumKiloCycles();
    }        
}

/***********************************************/
void tscEnd(enum PROFILESECTION sel) 
{
    PSECTION *pSection;

    if(profStarted) {
        pSection = &pSectionArray[sel];

        pSection->pkc2 = pentiumKiloCycles();
        pSection->pkc2 = (pSection->pkc2 - pSection->pkc1);
        pSection->avgKc += pSection->pkc2;
        pSection->numTimes += 1;

        if(pSection->pkc2 < pSection->minKc)
            pSection->minKc = pSection->pkc2;

        if(pSection->pkc2 > pSection->maxKc)
            pSection->maxKc = pSection->pkc2;
    }
}

/***********************************************/
void tscInit() 
{
    int i;

    for(i=0; i<MAX_PROFILE; i++) {
        duck_memset(&pSectionArray[i],0,sizeof(PSECTION));
        pSectionArray[i].minKc = 0xffffffff;
    }

    profStarted = 1;
}

/***********************************************/
void tscUninit() 
{
    profStarted = 0;
}

/***********************************************/
unsigned long tscProcessCounts(unsigned long *cnt, enum PROFILESECTION sel) 
{
    unsigned long rv = 0;

    *cnt = 0;
    if(profStarted) {
        if(pSectionArray[sel].numTimes) {
            rv = pSectionArray[sel].avgKc /= pSectionArray[sel].numTimes;
            *cnt = pSectionArray[sel].numTimes;
            duck_memset(&pSectionArray[sel],0,sizeof(PSECTION));
            pSectionArray[sel].minKc = 0xffffffff;
        }
        /* reset all vars */
    }
    return (rv);
}
#endif


