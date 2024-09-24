#include "precomp_wasabi_bfc.h"
#include "bigstring.h"

BigString::BigString() {
  mem = NULL;
  m_linecount = 0;
}

BigString::~BigString() {
  if (mem != NULL) {
    FREE(mem);
  }
  strings.deleteAll();
}

const char *BigString::getValue() /*const*/ {
  if (mem != NULL) return mem;
  size_t l = 0;
  foreach(strings)
    l += strings.getfor()->len();
  endfor;
  mem = (char *)MALLOC(l+1);
  char *p = mem;
  String *s = NULL;
  size_t sl = 0;
  foreach(strings)
    s = strings.getfor();
    sl = s->len();
    if (sl > 0) MEMCPY((void *)p, (void *)s->getValue(), sl);
    p += sl;
  endfor;
  *p = 0;
  return mem;
}

void BigString::setValue(const char *val) {
  if (mem != NULL) {
    FREE(mem);
    mem = NULL;
  }
  strings.deleteAll();
  cat(val);
}

int BigString::isempty() {
  if (strings.getNumItems() == 0) return 1;
  foreach(strings)
    if (!strings.getfor()->isempty()) return 0;
  endfor;
  return 1;
}

void BigString::reset() {
  if (mem != NULL) {
    FREE(mem);
    mem = NULL;
  }
  strings.deleteAll();
  m_linecount = 0;
}

void BigString::catn(const char *s, int n) {
  String *str = new String();
  str->catn(s, n);
  cat(str->getValue());
}

void BigString::cat(const char *s) {
  if (mem != NULL) {
    FREE(mem);
    mem = NULL;
  }
  char *p = (char *)s;
  while (p && *p) {
    if (*p == '\r' || *p == '\n') {
      if (*(p+1) == '\n' && *p == '\r') p++;
      m_linecount++;
    }
    p++;
  }
  strings.addItem(new String(s));
}

char BigString::lastChar() {
  return strings.getLast()->lastChar();
}

char BigString::firstChar() {
  const char *s = strings.getFirst()->getValue();
  return s ? *s : 0;
}

int BigString::getLineCount() {
  return m_linecount;
}
