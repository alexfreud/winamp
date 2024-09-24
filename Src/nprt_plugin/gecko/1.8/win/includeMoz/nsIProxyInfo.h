/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/base/public/nsIProxyInfo.idl
 */

#ifndef __gen_nsIProxyInfo_h__
#define __gen_nsIProxyInfo_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIProxyInfo */
#define NS_IPROXYINFO_IID_STR "3fe9308b-1608-4fa0-933c-c5ec2c6175fd"

#define NS_IPROXYINFO_IID \
  {0x3fe9308b, 0x1608, 0x4fa0, \
    { 0x93, 0x3c, 0xc5, 0xec, 0x2c, 0x61, 0x75, 0xfd }}

/**
 * This interface identifies a proxy server.
 *
 * @status UNDER_REVIEW
 */
class NS_NO_VTABLE nsIProxyInfo : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IPROXYINFO_IID)

  /**
   * This attribute specifies the hostname of the proxy server.
   */
  /* readonly attribute AUTF8String host; */
  NS_IMETHOD GetHost(nsACString & aHost) = 0;

  /**
   * This attribute specifies the port number of the proxy server.
   */
  /* readonly attribute long port; */
  NS_IMETHOD GetPort(PRInt32 *aPort) = 0;

  /**
   * This attribute specifies the type of the proxy server as an ASCII string.
   *
   * Some special values for this attribute include (but are not limited to)
   * the following:
   *   "http"     HTTP proxy (or SSL CONNECT for HTTPS)
   *   "socks"    SOCKS v5 proxy
   *   "socks4"   SOCKS v4 proxy
   *   "direct"   no proxy
   *   "unknown"  unknown proxy (see nsIProtocolProxyService::resolve)
   */
  /* readonly attribute ACString type; */
  NS_IMETHOD GetType(nsACString & aType) = 0;

  /**
   * This attribute specifies flags that modify the proxy type.  The value of
   * this attribute is the bit-wise combination of the Proxy Flags defined
   * below.  Any undefined bits are reserved for future use.
   */
  /* readonly attribute unsigned long flags; */
  NS_IMETHOD GetFlags(PRUint32 *aFlags) = 0;

  /**
   * This attribute specifies the failover timeout in seconds for this proxy.
   * If a nsIProxyInfo is reported as failed via nsIProtocolProxyService::
   * getFailoverForProxy, then the failed proxy will not be used again for this
   * many seconds.
   */
  /* readonly attribute unsigned long failoverTimeout; */
  NS_IMETHOD GetFailoverTimeout(PRUint32 *aFailoverTimeout) = 0;

  /**
   * This attribute specifies the proxy to failover to when this proxy fails.
   */
  /* attribute nsIProxyInfo failoverProxy; */
  NS_IMETHOD GetFailoverProxy(nsIProxyInfo * *aFailoverProxy) = 0;
  NS_IMETHOD SetFailoverProxy(nsIProxyInfo * aFailoverProxy) = 0;

  /****************************************************************************
   * The following "Proxy Flags" may be bit-wise combined to construct the flags
   * attribute defined on this interface.
   */
/**
   * This flag is set if the proxy is to perform name resolution itself.  If
   * this is the case, the hostname is used in some fashion, and we shouldn't
   * do any form of DNS lookup ourselves.
   */
  enum { TRANSPARENT_PROXY_RESOLVES_HOST = 1U };

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIPROXYINFO \
  NS_IMETHOD GetHost(nsACString & aHost); \
  NS_IMETHOD GetPort(PRInt32 *aPort); \
  NS_IMETHOD GetType(nsACString & aType); \
  NS_IMETHOD GetFlags(PRUint32 *aFlags); \
  NS_IMETHOD GetFailoverTimeout(PRUint32 *aFailoverTimeout); \
  NS_IMETHOD GetFailoverProxy(nsIProxyInfo * *aFailoverProxy); \
  NS_IMETHOD SetFailoverProxy(nsIProxyInfo * aFailoverProxy); \

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIPROXYINFO(_to) \
  NS_IMETHOD GetHost(nsACString & aHost) { return _to GetHost(aHost); } \
  NS_IMETHOD GetPort(PRInt32 *aPort) { return _to GetPort(aPort); } \
  NS_IMETHOD GetType(nsACString & aType) { return _to GetType(aType); } \
  NS_IMETHOD GetFlags(PRUint32 *aFlags) { return _to GetFlags(aFlags); } \
  NS_IMETHOD GetFailoverTimeout(PRUint32 *aFailoverTimeout) { return _to GetFailoverTimeout(aFailoverTimeout); } \
  NS_IMETHOD GetFailoverProxy(nsIProxyInfo * *aFailoverProxy) { return _to GetFailoverProxy(aFailoverProxy); } \
  NS_IMETHOD SetFailoverProxy(nsIProxyInfo * aFailoverProxy) { return _to SetFailoverProxy(aFailoverProxy); } \

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIPROXYINFO(_to) \
  NS_IMETHOD GetHost(nsACString & aHost) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHost(aHost); } \
  NS_IMETHOD GetPort(PRInt32 *aPort) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPort(aPort); } \
  NS_IMETHOD GetType(nsACString & aType) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetType(aType); } \
  NS_IMETHOD GetFlags(PRUint32 *aFlags) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFlags(aFlags); } \
  NS_IMETHOD GetFailoverTimeout(PRUint32 *aFailoverTimeout) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFailoverTimeout(aFailoverTimeout); } \
  NS_IMETHOD GetFailoverProxy(nsIProxyInfo * *aFailoverProxy) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFailoverProxy(aFailoverProxy); } \
  NS_IMETHOD SetFailoverProxy(nsIProxyInfo * aFailoverProxy) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFailoverProxy(aFailoverProxy); } \

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsProxyInfo : public nsIProxyInfo
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPROXYINFO

  nsProxyInfo();

private:
  ~nsProxyInfo();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsProxyInfo, nsIProxyInfo)

nsProxyInfo::nsProxyInfo()
{
  /* member initializers and constructor code */
}

nsProxyInfo::~nsProxyInfo()
{
  /* destructor code */
}

/* readonly attribute AUTF8String host; */
NS_IMETHODIMP nsProxyInfo::GetHost(nsACString & aHost)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long port; */
NS_IMETHODIMP nsProxyInfo::GetPort(PRInt32 *aPort)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute ACString type; */
NS_IMETHODIMP nsProxyInfo::GetType(nsACString & aType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long flags; */
NS_IMETHODIMP nsProxyInfo::GetFlags(PRUint32 *aFlags)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long failoverTimeout; */
NS_IMETHODIMP nsProxyInfo::GetFailoverTimeout(PRUint32 *aFailoverTimeout)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsIProxyInfo failoverProxy; */
NS_IMETHODIMP nsProxyInfo::GetFailoverProxy(nsIProxyInfo * *aFailoverProxy)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsProxyInfo::SetFailoverProxy(nsIProxyInfo * aFailoverProxy)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIProxyInfo_h__ */
