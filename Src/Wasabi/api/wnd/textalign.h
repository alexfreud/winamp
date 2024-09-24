/*

Darkain Made This.  :)
// and BU tweaked it

*/

/*
Darkain:  i wanted one base for ALL text alignment in ALL classes.
*/


#ifndef _TEXTALIGN_H
#define _TEXTALIGN_H

typedef enum {
  TEXTALIGN_LEFT,
  TEXTALIGN_CENTER, //what ever default center style is... see bellow
  TEXTALIGN_RIGHT,
  TEXTALIGN_EVENSPACING,  //add more space between letters/words to make it fit in 100% of the area
  TEXTALIGN_FITTOWIDTH,  //make the font larger or smaller to fit in 100% of the area
  TEXTALIGN_LEFT_ELLIPSIS, //align left, and truncate text to fit if too large
  TEXTALIGN_CENTER_CENTER,  //if text is too wide, it will still center on the middle, choping off left and right sides
  TEXTALIGN_CENTER_LEFT,	//will chop off right side if too big
  TEXTALIGN_CENTER_RIGHT,	//will chop off left side if too big
  TEXTALIGN_SCROLL,  //if text is too large, it will use default scrolling (see bellow)
  TEXTALIGN_SCROLL_BACKFORTH,  //text will scroll back and forth if too large
  TEXTALIGN_SCROLL_TICKER,  //text will scroll in one direction, and loop
} TextAlign;


#endif // TEXT_ALIGN_H
