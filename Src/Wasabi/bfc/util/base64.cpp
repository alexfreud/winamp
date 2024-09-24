/* Based on code from Dave Winer (http://www.scripting.com/midas/base64/) */
#include "precomp_wasabi_bfc.h"
#include "base64.h"
 
char Base64::encodingTable [64] = {
  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
  'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
  'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
  'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'
};
 
int Base64::encode(MemBlock<char> &htext, MemBlock<char> &h64, int linelength) { 
  /*
  encode the handle. some funny stuff about linelength -- it only makes
  sense to make it a multiple of 4. if it's not a multiple of 4, we make it
  so (by only checking it every 4 characters. 
  further, if it's 0, we don't add any line breaks at all.
  */
  unsigned long ixtext;
  unsigned long lentext;
  unsigned long origsize;
  long ctremaining;
  unsigned char inbuf [3] = {0}, outbuf [4] = {0};
  short i;
  short charsonline = 0, ctcopy;
  
  ixtext = 0;

  lentext = htext.getSize(); 

  while (1) {
    ctremaining = lentext - ixtext;

    if (ctremaining <= 0)
      break;
        
    for (i = 0; i < 3; i++) { 
      unsigned long ix = ixtext + i;
      if (ix < lentext)
        inbuf [i] = *htext.getMemory(ix);
      else
        inbuf [i] = 0;
    } 
    
    outbuf [0] = (inbuf [0] & 0xFC) >> 2;
    outbuf [1] = ((inbuf [0] & 0x03) << 4) | ((inbuf [1] & 0xF0) >> 4);
    outbuf [2] = ((inbuf [1] & 0x0F) << 2) | ((inbuf [2] & 0xC0) >> 6);
    outbuf [3] = inbuf [2] & 0x3F;

    origsize = h64.getSize();
    h64.setSize(origsize + 4);
    
    ctcopy = 4;

    switch (ctremaining) {
      case 1: 
        ctcopy = 2; 
        break;
      case 2: 
        ctcopy = 3; 
        break;
    } 
    
    for (i = 0; i < ctcopy; i++)
      *h64.getMemory(origsize + i) = encodingTable[outbuf[i]];
    
    for (i = ctcopy; i < 4; i++)
      *h64.getMemory(origsize + i) = '=';
    
    ixtext += 3;
    charsonline += 4;

    if (linelength > 0) { /*DW 4/8/97 -- 0 means no line breaks*/
      if (charsonline >= linelength) {
        charsonline = 0;
        origsize = h64.getSize();
        h64.setSize(origsize + 1);
        *h64.getMemory() = '\n';
      }
    }
  } 
  return 1;
} 
int Base64::encode(MemBlock<char> &htext, MemBlock<wchar_t> &h64, int linelength) { 
  /*
  encode the handle. some funny stuff about linelength -- it only makes
  sense to make it a multiple of 4. if it's not a multiple of 4, we make it
  so (by only checking it every 4 characters. 
  further, if it's 0, we don't add any line breaks at all.
  */
  unsigned long ixtext;
  unsigned long lentext;
  unsigned long origsize;
  long ctremaining;
  unsigned char inbuf [3] = {0}, outbuf [4] = {0};
  short i;
  short charsonline = 0, ctcopy;
  
  ixtext = 0;

  lentext = htext.getSize(); 

  while (1) {
    ctremaining = lentext - ixtext;

    if (ctremaining <= 0)
      break;
        
    for (i = 0; i < 3; i++) { 
      unsigned long ix = ixtext + i;
      if (ix < lentext)
        inbuf [i] = *htext.getMemory(ix);
      else
        inbuf [i] = 0;
    } 
    
    outbuf [0] = (inbuf [0] & 0xFC) >> 2;
    outbuf [1] = ((inbuf [0] & 0x03) << 4) | ((inbuf [1] & 0xF0) >> 4);
    outbuf [2] = ((inbuf [1] & 0x0F) << 2) | ((inbuf [2] & 0xC0) >> 6);
    outbuf [3] = inbuf [2] & 0x3F;

    origsize = h64.getSize();
    h64.setSize(origsize + 4);
    
    ctcopy = 4;

    switch (ctremaining) {
      case 1: 
        ctcopy = 2; 
        break;
      case 2: 
        ctcopy = 3; 
        break;
    } 
    
    for (i = 0; i < ctcopy; i++)
      *h64.getMemory(origsize + i) = encodingTable[outbuf[i]];
    
    for (i = ctcopy; i < 4; i++)
      *h64.getMemory(origsize + i) = '=';
    
    ixtext += 3;
    charsonline += 4;

    if (linelength > 0) { /*DW 4/8/97 -- 0 means no line breaks*/
      if (charsonline >= linelength) {
        charsonline = 0;
        origsize = h64.getSize();
        h64.setSize(origsize + 1);
        *h64.getMemory() = '\n';
      }
    }
  } 
  return 1;
} 
 
 
int Base64::decode(MemBlock<char> &h64, MemBlock<char> &htext) {
  unsigned long ixtext;
  unsigned long lentext;
  unsigned long origsize;
  unsigned char ch;
  unsigned char inbuf [4] = {0}, outbuf [4] = {0};
  short i, ixinbuf;
  boolean flignore;
  boolean flendtext = false;
    
  ixtext = 0;
 
  lentext = h64.getSize();
 
  ixinbuf = 0;
 
  while (true) {
    if (ixtext >= lentext)
      break;
    ch = *h64.getMemory(ixtext++);
  
    flignore = 0;
  
    if ((ch >= 'A') && (ch <= 'Z'))
      ch = ch - 'A';
    else if ((ch >= 'a') && (ch <= 'z'))
      ch = ch - 'a' + 26;
    else if ((ch >= '0') && (ch <= '9'))
      ch = ch - '0' + 52;
    else if (ch == '+')
      ch = 62;
    else if (ch == '=') /*no op -- can't ignore this one*/
      flendtext = true;
    else if (ch == '/')
      ch = 63;
    else
      flignore = true; 
  
    if (!flignore) {
      short ctcharsinbuf = 3;
      boolean flbreak = false;
       
      if (flendtext) {
        if (ixinbuf == 0)
          break;
        
        if ((ixinbuf == 1) || (ixinbuf == 2))
          ctcharsinbuf = 1;
        else
          ctcharsinbuf = 2;
      
        ixinbuf = 3;
        flbreak = 1;
      }
    
      inbuf [ixinbuf++] = ch;
    
      if (ixinbuf == 4) {
        ixinbuf = 0;

        outbuf [0] = (inbuf [0] << 2) | ((inbuf [1] & 0x30) >> 4);
        outbuf [1] = ((inbuf [1] & 0x0F) << 4) | ((inbuf [2] & 0x3C) >> 2);
        outbuf [2] = ((inbuf [2] & 0x03) << 6) | (inbuf [3] & 0x3F);
    
        origsize = htext.getSize();
        htext.setSize(origsize + ctcharsinbuf);
        
        for (i = 0; i < ctcharsinbuf; i++) 
          *htext.getMemory(origsize + i) = outbuf[i];
      }
    
      if (flbreak)
        break;
    }
  } 
 
  return 1;
} 
int Base64::decode(MemBlock<wchar_t> &h64, MemBlock<char> &htext) {
  unsigned long ixtext;
  unsigned long lentext;
  unsigned long origsize;
  unsigned char ch;
  unsigned char inbuf [4] = {0}, outbuf [4] = {0};
  short i, ixinbuf;
  boolean flignore;
  boolean flendtext = false;
    
  ixtext = 0;
 
  lentext = h64.getSize();
 
  ixinbuf = 0;
 
  while (true) {
    if (ixtext >= lentext)
      break;
    ch = (unsigned char)*h64.getMemory(ixtext++);
  
    flignore = 0;
  
    if ((ch >= 'A') && (ch <= 'Z'))
      ch = ch - 'A';
    else if ((ch >= 'a') && (ch <= 'z'))
      ch = ch - 'a' + 26;
    else if ((ch >= '0') && (ch <= '9'))
      ch = ch - '0' + 52;
    else if (ch == '+')
      ch = 62;
    else if (ch == '=') /*no op -- can't ignore this one*/
      flendtext = true;
    else if (ch == '/')
      ch = 63;
    else
      flignore = true; 
  
    if (!flignore) {
      short ctcharsinbuf = 3;
      boolean flbreak = false;
       
      if (flendtext) {
        if (ixinbuf == 0)
          break;
        
        if ((ixinbuf == 1) || (ixinbuf == 2))
          ctcharsinbuf = 1;
        else
          ctcharsinbuf = 2;
      
        ixinbuf = 3;
        flbreak = 1;
      }
    
      inbuf [ixinbuf++] = ch;
    
      if (ixinbuf == 4) {
        ixinbuf = 0;

        outbuf [0] = (inbuf [0] << 2) | ((inbuf [1] & 0x30) >> 4);
        outbuf [1] = ((inbuf [1] & 0x0F) << 4) | ((inbuf [2] & 0x3C) >> 2);
        outbuf [2] = ((inbuf [2] & 0x03) << 6) | (inbuf [3] & 0x3F);
    
        origsize = htext.getSize();
        htext.setSize(origsize + ctcharsinbuf);
        
        for (i = 0; i < ctcharsinbuf; i++) 
          *htext.getMemory(origsize + i) = outbuf[i];
      }
    
      if (flbreak)
        break;
    }
  } 
 
  return 1;
} 
 
 
