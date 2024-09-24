#include "dxl_main.h"
#include "duck_dxl.h"


/*-------------------------------------------------------------------

File : dxv_util.c  

Any extra functions whose lifespan/utility might be "questionable".
Functions that are not part of the "core", but yet are not really 
anything but Dxv specific.

-------------------------------------------------------------------*/


/* This function used during the development of ICM wrapper */
/*----------------------------------------------------------*/
char *DXL_DumpRegistry(char *buf);
char *DXL_DumpRegistry(char *buf)
{
        int sprintf( char *buffer, const char *format, ...);

        unsigned long *g = DXL_GetFourCCList();

    	int i = 0;
    
    	while(g)
	    {   
       		sprintf(buf,"fourCC[%d] = %c%c%c%c\n",i,
		        (g[i] & 0xFF000000) >> 24,
		        (g[i] & 0xFF0000) >> 16,
		        (g[i] & 0xFF00) >> 8,
		        (g[i] & 0xFF) >> 0
		    );
                
            g++;
    	}

	    return buf;
}
