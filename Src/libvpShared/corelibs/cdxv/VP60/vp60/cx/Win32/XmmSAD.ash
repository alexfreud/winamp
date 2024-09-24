;------------------------------------------------
XMMGetSADParams  STRUC
                    dd  6 dup (?)   ;6 pushed regs
                    dd  ?           ;return address
    NewDataPtr      dd  ?
    PixelsPerLine   dd  ?
    RefDataPtr      dd  ?
    RefPixelsPerLine   dd  ?
    ErrorSoFar      dd  ?
    BestSoFar       dd  ?
XMMGetSADParams  ENDS
;------------------------------------------------
