#include "ds2.h"

UINT DS2::bytes2ms(UINT bytes) {return MulDiv(bytes,1000,fmt_mul);}
UINT DS2::ms2bytes(UINT ms) {return _align_var(MulDiv(ms,fmt_mul,1000));}
