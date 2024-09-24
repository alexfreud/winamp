/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/cookie/public/nsICookie.idl
 */

#ifndef __gen_nsICookie_h__
#define __gen_nsICookie_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
/** 
 * An optional interface for accessing the HTTP or
 * javascript cookie object
 * 
 * @status FROZEN
 */
typedef PRInt32 nsCookieStatus;

typedef PRInt32 nsCookiePolicy;


/* starting interface:    nsICookie */
#define NS_ICOOKIE_IID_STR "e9fcb9a4-d376-458f-b720-e65e7df593bc"

#define NS_ICOOKIE_IID \
  {0xe9fcb9a4, 0xd376, 0x458f, \
    { 0xb7, 0x20, 0xe6, 0x5e, 0x7d, 0xf5, 0x93, 0xbc }}

class NS_NO_VTABLE nsICookie : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICOOKIE_IID)

  /**
     * the name of the cookie
     */
  /* readonly attribute ACString name; */
  NS_IMETHOD GetName(nsACString & aName) = 0;

  /**
     * the cookie value
     */
  /* readonly attribute ACString value; */
  NS_IMETHOD GetValue(nsACString & aValue) = 0;

  /**
     * true if the cookie is a domain cookie, false otherwise
     */
  /* readonly attribute boolean isDomain; */
  NS_IMETHOD GetIsDomain(PRBool *aIsDomain) = 0;

  /**
     * the host (possibly fully qualified) of the cookie
     */
  /* readonly attribute AUTF8String host; */
  NS_IMETHOD GetHost(nsACString & aHost) = 0;

  /**
     * the path pertaining to the cookie
     */
  /* readonly attribute AUTF8String path; */
  NS_IMETHOD GetPath(nsACString & aPath) = 0;

  /**
     * true if the cookie was transmitted over ssl, false otherwise
     */
  /* readonly attribute boolean isSecure; */
  NS_IMETHOD GetIsSecure(PRBool *aIsSecure) = 0;

  /**
     * expiration time (local timezone) expressed as number of seconds since Jan 1, 1970
     */
  /* readonly attribute PRUint64 expires; */
  NS_IMETHOD GetExpires(PRUint64 *aExpires) = 0;

  /**
     * P3P status of cookie.  Values are
     *
     *   STATUS_UNKNOWN -- cookie collected in a previous session and this info no longer available
     *   STATUS_ACCEPTED -- cookie was accepted as it
     *   STATUS_DOWNGRADED -- cookie was accepted but downgraded to a session cookie
     *   STATUS_FLAGGED -- cookie was accepted with a warning being issued to the user
     *   STATUS_REJECTED -- cookie was not accepted
     */
  enum { STATUS_UNKNOWN = 0 };

  enum { STATUS_ACCEPTED = 1 };

  enum { STATUS_DOWNGRADED = 2 };

  enum { STATUS_FLAGGED = 3 };

  enum { STATUS_REJECTED = 4 };

  /* readonly attribute nsCookieStatus status; */
  NS_IMETHOD GetStatus(nsCookieStatus *aStatus) = 0;

  /**
     * Site's compact policy.  Values are
     *
     *   POLICY_UNKNOWN -- cookie collected in a previous session and this info no longer available
     *   POLICY_NONE -- site did not send a compact policy along with the cookie
     *   POLICY_NO_CONSENT -- site collects identfiable information without user involvement
     *   POLICY_IMPLICIT_CONSENT -- site collects identifiable information unless user opts out
     *   POLICY_EXPLICIT_CONSENT -- site does not collect identifiable information unless user opts in
     *   POLICY_NO_II -- site does not collect identifiable information
     */
  enum { POLICY_UNKNOWN = 0 };

  enum { POLICY_NONE = 1 };

  enum { POLICY_NO_CONSENT = 2 };

  enum { POLICY_IMPLICIT_CONSENT = 3 };

  enum { POLICY_EXPLICIT_CONSENT = 4 };

  enum { POLICY_NO_II = 5 };

  /* readonly attribute nsCookiePolicy policy; */
  NS_IMETHOD GetPolicy(nsCookiePolicy *aPolicy) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICOOKIE \
  NS_IMETHOD GetName(nsACString & aName); \
  NS_IMETHOD GetValue(nsACString & aValue); \
  NS_IMETHOD GetIsDomain(PRBool *aIsDomain); \
  NS_IMETHOD GetHost(nsACString & aHost); \
  NS_IMETHOD GetPath(nsACString & aPath); \
  NS_IMETHOD GetIsSecure(PRBool *aIsSecure); \
  NS_IMETHOD GetExpires(PRUint64 *aExpires); \
  NS_IMETHOD GetStatus(nsCookieStatus *aStatus); \
  NS_IMETHOD GetPolicy(nsCookiePolicy *aPolicy); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICOOKIE(_to) \
  NS_IMETHOD GetName(nsACString & aName) { return _to GetName(aName); } \
  NS_IMETHOD GetValue(nsACString & aValue) { return _to GetValue(aValue); } \
  NS_IMETHOD GetIsDomain(PRBool *aIsDomain) { return _to GetIsDomain(aIsDomain); } \
  NS_IMETHOD GetHost(nsACString & aHost) { return _to GetHost(aHost); } \
  NS_IMETHOD GetPath(nsACString & aPath) { return _to GetPath(aPath); } \
  NS_IMETHOD GetIsSecure(PRBool *aIsSecure) { return _to GetIsSecure(aIsSecure); } \
  NS_IMETHOD GetExpires(PRUint64 *aExpires) { return _to GetExpires(aExpires); } \
  NS_IMETHOD GetStatus(nsCookieStatus *aStatus) { return _to GetStatus(aStatus); } \
  NS_IMETHOD GetPolicy(nsCookiePolicy *aPolicy) { return _to GetPolicy(aPolicy); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICOOKIE(_to) \
  NS_IMETHOD GetName(nsACString & aName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetName(aName); } \
  NS_IMETHOD GetValue(nsACString & aValue) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetValue(aValue); } \
  NS_IMETHOD GetIsDomain(PRBool *aIsDomain) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetIsDomain(aIsDomain); } \
  NS_IMETHOD GetHost(nsACString & aHost) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHost(aHost); } \
  NS_IMETHOD GetPath(nsACString & aPath) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPath(aPath); } \
  NS_IMETHOD GetIsSecure(PRBool *aIsSecure) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetIsSecure(aIsSecure); } \
  NS_IMETHOD GetExpires(PRUint64 *aExpires) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetExpires(aExpires); } \
  NS_IMETHOD GetStatus(nsCookieStatus *aStatus) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStatus(aStatus); } \
  NS_IMETHOD GetPolicy(nsCookiePolicy *aPolicy) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPolicy(aPolicy); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsCookie : public nsICookie
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICOOKIE

  nsCookie();

private:
  ~nsCookie();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsCookie, nsICookie)

nsCookie::nsCookie()
{
  /* member initializers and constructor code */
}

nsCookie::~nsCookie()
{
  /* destructor code */
}

/* readonly attribute ACString name; */
NS_IMETHODIMP nsCookie::GetName(nsACString & aName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute ACString value; */
NS_IMETHODIMP nsCookie::GetValue(nsACString & aValue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean isDomain; */
NS_IMETHODIMP nsCookie::GetIsDomain(PRBool *aIsDomain)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute AUTF8String host; */
NS_IMETHODIMP nsCookie::GetHost(nsACString & aHost)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute AUTF8String path; */
NS_IMETHODIMP nsCookie::GetPath(nsACString & aPath)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean isSecure; */
NS_IMETHODIMP nsCookie::GetIsSecure(PRBool *aIsSecure)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute PRUint64 expires; */
NS_IMETHODIMP nsCookie::GetExpires(PRUint64 *aExpires)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsCookieStatus status; */
NS_IMETHODIMP nsCookie::GetStatus(nsCookieStatus *aStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsCookiePolicy policy; */
NS_IMETHODIMP nsCookie::GetPolicy(nsCookiePolicy *aPolicy)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsICookie_h__ */
