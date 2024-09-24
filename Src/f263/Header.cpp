#include "Decoder.h"


/*
 * decode headers from one input stream
 * until an End of Sequence or picture start code
 * is found
 */
int Decoder::getheader()
{
  unsigned int sorenson_version;

  /* look for startcode */
  startcode();
  buffer.getbits(PSC_LENGTH);
  sorenson_version = buffer.getbits(5);
  if (sorenson_version <= 1) 
	{
		escapemode=sorenson_version;
    getpicturehdr();
  }
	else
		return 0;

  return 1;
}

/* align to start of next startcode */

void Decoder::startcode()
{
  /* search for new picture start code */
  while (buffer.showbits(PSC_LENGTH)!=1l) 
         buffer.flushbits(1);
}

/* decode picture header */

void Decoder::getpicturehdr()
{
  int pei, tmp;

  buffer.getbits(8);

  tmp = buffer.getbits(3);	
	switch(tmp)
	{
		    case 0:
        horizontal_size = buffer.getbits(8);
        vertical_size = buffer.getbits(8);
        break;
    case 1:
        horizontal_size = buffer.getbits(16);
        vertical_size = buffer.getbits(16);
        break;
    case 2:
        horizontal_size = 352;
        vertical_size = 288;
        break;
    case 3:
        horizontal_size = 176;
        vertical_size = 144;
        break;
    case 4:
        horizontal_size = 128;
        vertical_size = 96;
        break;
    case 5:
        horizontal_size = 320;
        vertical_size = 240;
        break;
    case 6:
        horizontal_size = 160;
        vertical_size = 120;
        break;

	}
	pict_type = buffer.getbits(2);
	deblock=buffer.getbits(1); // deblocking flag
  quant = buffer.getbits(5);	

  pei = buffer.getbits(1);
pspare:
  if (pei) {
     /* extra info for possible future backward compatible additions */
    buffer.getbits(8);  /* not used */
    pei = buffer.getbits(1);
    if (pei) goto pspare; /* keep on reading pspare until pei=0 */
  }

}

