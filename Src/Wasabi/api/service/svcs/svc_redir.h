#ifndef _SVC_REDIR_H
#define _SVC_REDIR_H

#include <bfc/dispatch.h>
#include <api/service/services.h>

/* Domains:
	"Filename"
	"Url"
*/

// if you want to redirect a string easily, look at RedirString in
// bfc/util/redirstr.h

class svc_redirect : public Dispatchable 
{
public:
  static FOURCC getServiceType() { return WaSvc::REDIRECT; }

  int isMine(const wchar_t *str, const wchar_t *domain);
  int redirect(const wchar_t *orig_str, const wchar_t *domain, wchar_t *buf, int buflen);

protected:
  enum {
    ISMINE=100,
    REDIRECT=200,
  };
};

inline int svc_redirect::isMine(const wchar_t *str, const wchar_t *domain) {
  return _call(ISMINE, 0, str, domain);
}

inline int svc_redirect::redirect(const wchar_t *orig_str, const wchar_t *domain, wchar_t *buf, int buflen) {
  return _call(REDIRECT, 0, orig_str, domain, buf, buflen);
}

class svc_redirectI : public svc_redirect {
public:
  virtual int isMine(const wchar_t *str, const wchar_t *domain)=0;
  virtual int redirect(const wchar_t *orig_str, const wchar_t *domain, wchar_t *buf, int buflen)=0;

protected:
  RECVS_DISPATCH;
};

#include <api/service/svc_enum.h>

class RedirectEnum : public SvcEnumT<svc_redirect> {
public:
  RedirectEnum(const wchar_t *filename, const wchar_t *domain=L"Filename") :
    fn(filename), dom(domain) {
  }

  virtual int testService(svc_redirect *svc) {
    if (svc->isMine(fn, dom)) return 1;
    return 0;
  }

private:
  const wchar_t *fn, *dom;
};
#endif
