#include "precomp_wasabi_bfc.h"

#include "utf8.h"

// this STILL doesn't work perfectly but at least it decodes what we write out
// mostly just waiting on our wide character strategy

void COMEXP UTF8_to_ASCII(const char *in, char *out) {
  unsigned const char *src = (unsigned const char *)in;
  unsigned char *dst = (unsigned char *)out;
  *dst = 0;
  for (; *src; src++) {
    int c = *src;
    if ((c & 0x80) == 0) {
      *dst++ = c;
      continue;
    }
    if ((c & 224) != 192) continue;	// fuck you, we only check for single bytes
    int v = (c & 0x3) << 6;
    ++src;
    v |= *src & 0x3f;
    *dst++ = v;
  }
  *dst = 0;
}
