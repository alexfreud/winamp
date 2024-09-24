#include "precomp.h"
#include "skinfont.h"
#include "api.h"
#include "../bfc/std.h"

SkinFont::SkinFont() {
}

SkinFont::~SkinFont() {
  if (!tempFn.isempty()) {
#ifdef WIN32
    RemoveFontResource(tempFn);
#else
    DebugString( "portme -- SkinFont::~SkinFont\n" );
#endif
    UNLINK(tempFn);
  }
}

int SkinFont::setXmlOption(const char *paramname, const char *strvalue) {
  return 0;
}

void SkinFont::installFont(OSFNSTR filename, OSFNSTR path) {
  FILE *in,*out;
  StringPrintf temp("%s%s", path, filename);
  in = WFOPEN(temp, L"rb");
  if (!in) return;
  int len = FGETSIZE(in);
  MemBlock<char> m(len);
  FREAD(m.getMemory(), len, 1, in);
  tempFn = TMPNAM(NULL);
  out = FOPEN(tempFn, "wb");
  ASSERT(out);
  FWRITE(m.getMemory(), len, 1, out);
  FCLOSE(out);
  FCLOSE(in);
#ifdef WIN32
  AddFontResource(tempFn);
#else
  DebugString( "portme -- SkinFont::installFont\n" );
#endif
}
