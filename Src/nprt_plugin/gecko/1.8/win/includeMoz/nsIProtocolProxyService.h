/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/base/public/nsIProtocolProxyService.idl
 */

#ifndef __gen_nsIProtocolProxyService_h__
#define __gen_nsIProtocolProxyService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsICancelable; /* forward declaration */

class nsIProtocolProxyCallback; /* forward declaration */

class nsIProtocolProxyFilter; /* forward declaration */

class nsIProxyInfo; /* forward declaration */

class nsIChannel; /* forward declaration */

class nsIURI; /* forward declaration */


/* starting interface:    nsIProtocolProxyService */
#define NS_IPROTOCOLPROXYSERVICE_IID_STR "e38ab577-786e-4a7f-936b-7ae4c7d877b2"

#define NS_IPROTOCOLPROXYSERVICE_IID \
  {0xe38ab577, 0x786e, 0x4a7f, \
    { 0x93, 0x6b, 0x7a, 0xe4, 0xc7, 0xd8, 0x77, 0xb2 }}

/**
 * nsIProtocolProxyService provides methods to access information about
 * various network proxies.
 *
 * @status UNDER_REVIEW
 */
class NS_NO_VTABLE nsIProtocolProxyService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IPROTOCOLPROXYSERVICE_IID)

  /**
     * This flag may be passed to the resolve method to request that it fail
     * instead of block the calling thread.  Proxy Auto Config (PAC) may
     * perform a synchronous DNS query, which may not return immediately.  So,
     * calling resolve without this flag may result in locking up the calling
     * thread for a lengthy period of time.
     *
     * By passing this flag to resolve, one can failover to asyncResolve to
     * avoid locking up the calling thread if a PAC query is required.
     *
     * When this flag is passed to resolve, resolve may throw the exception
     * NS_BASE_STREAM_WOULD_BLOCK to indicate that it failed due to this flag
     * being present.
     */
  enum { RESOLVE_NON_BLOCKING = 1U };

  /**
     * This method returns a nsIProxyInfo instance that identifies a proxy to
     * be used for loading the given URI.  Otherwise, this method returns null
     * indicating that a direct connection should be used.
     *
     * @param aURI
     *        The URI to test.
     * @param aFlags
     *        A bit-wise OR of the RESOLVE_ flags defined above.  Pass 0 to
     *        specify the default behavior.
     *
     * NOTE: If this proxy is unavailable, getFailoverForProxy may be called
     * to determine the correct secondary proxy to be used.
     *
     * NOTE: If the protocol handler for the given URI supports
     * nsIProxiedProtocolHandler, then the nsIProxyInfo instance returned from
     * resolve may be passed to the newProxiedChannel method to create a
     * nsIChannel to the given URI that uses the specified proxy.
     *
     * NOTE: However, if the nsIProxyInfo type is "http", then it means that
     * the given URI should be loaded using the HTTP protocol handler, which
     * also supports nsIProxiedProtocolHandler.
     *
     * NOTE: If PAC is configured, and the PAC file has not yet been loaded,
     * then this method will return a nsIProxyInfo instance with a type of
     * "unknown" to indicate to the consumer that asyncResolve should be used
     * to wait for the PAC file to finish loading.  Otherwise, the consumer
     * may choose to treat the result as type "direct" if desired.
     *
     * @see nsIProxiedProtocolHandler::newProxiedChannel 
     */
  /* nsIProxyInfo resolve (in nsIURI aURI, in unsigned long aFlags); */
  NS_IMETHOD Resolve(nsIURI *aURI, PRUint32 aFlags, nsIProxyInfo **_retval) = 0;

  /**
     * This method is an asychronous version of the resolve method.  Unlike
     * resolve, this method is guaranteed not to block the calling thread
     * waiting for DNS queries to complete.  This method is intended as a
     * substitute for resolve when the result is not needed immediately.
     *
     * @param aURI
     *        The URI to test.
     * @param aFlags
     *        A bit-wise OR of the RESOLVE_ flags defined above.  Pass 0 to
     *        specify the default behavior.
     * @param aCallback
     *        The object to be notified when the result is available.
     *
     * @return An object that can be used to cancel the asychronous operation.
     */
  /* nsICancelable asyncResolve (in nsIURI aURI, in unsigned long aFlags, in nsIProtocolProxyCallback aCallback); */
  NS_IMETHOD AsyncResolve(nsIURI *aURI, PRUint32 aFlags, nsIProtocolProxyCallback *aCallback, nsICancelable **_retval) = 0;

  /**
     * This method may be called to construct a nsIProxyInfo instance from
     * the given parameters.  This method may be useful in conjunction with
     * nsISocketTransportService::createTransport for creating, for example,
     * a SOCKS connection.
     *
     * @param aType
     *        The proxy type.  This is a string value that identifies the proxy
     *        type.  Standard values include:
     *          "http"    - specifies a HTTP proxy
     *          "socks"   - specifies a SOCKS version 5 proxy
     *          "socks4"  - specifies a SOCKS version 4 proxy
     *          "direct"  - specifies a direct connection (useful for failover)
     *        The type name is case-insensitive.  Other string values may be
     *        possible.
     * @param aHost
     *        The proxy hostname or IP address.
     * @param aPort
     *        The proxy port.
     * @param aFlags
     *        Flags associated with this connection.  See nsIProxyInfo.idl
     *        for currently defined flags.
     * @param aFailoverTimeout
     *        Specifies the length of time (in seconds) to ignore this proxy if
     *        this proxy fails.  Pass PR_UINT32_MAX to specify the default
     *        timeout value, causing nsIProxyInfo::failoverTimeout to be
     *        assigned the default value.
     * @param aFailoverProxy
     *        Specifies the next proxy to try if this proxy fails.  This
     *        parameter may be null.
     */
  /* nsIProxyInfo newProxyInfo (in ACString aType, in AUTF8String aHost, in long aPort, in unsigned long aFlags, in unsigned long aFailoverTimeout, in nsIProxyInfo aFailoverProxy); */
  NS_IMETHOD NewProxyInfo(const nsACString & aType, const nsACString & aHost, PRInt32 aPort, PRUint32 aFlags, PRUint32 aFailoverTimeout, nsIProxyInfo *aFailoverProxy, nsIProxyInfo **_retval) = 0;

  /**
     * If the proxy identified by aProxyInfo is unavailable for some reason,
     * this method may be called to access an alternate proxy that may be used
     * instead.  As a side-effect, this method may affect future result values
     * from resolve/asyncResolve as well as from getFailoverForProxy.
     *
     * @param aProxyInfo
     *        The proxy that was unavailable.
     * @param aURI
     *        The URI that was originally passed to resolve/asyncResolve.
     * @param aReason
     *        The error code corresponding to the proxy failure.  This value
     *        may be used to tune the delay before this proxy is used again.
     *
     * @throw NS_ERROR_NOT_AVAILABLE if there is no alternate proxy available.
     */
  /* nsIProxyInfo getFailoverForProxy (in nsIProxyInfo aProxyInfo, in nsIURI aURI, in nsresult aReason); */
  NS_IMETHOD GetFailoverForProxy(nsIProxyInfo *aProxyInfo, nsIURI *aURI, nsresult aReason, nsIProxyInfo **_retval) = 0;

  /**
     * This method may be used to register a proxy filter instance.  Each proxy
     * filter is registered with an associated position that determines the
     * order in which the filters are applied (starting from position 0).  When
     * resolve/asyncResolve is called, it generates a list of proxies for the
     * given URI, and then it applies the proxy filters.  The filters have the
     * opportunity to modify the list of proxies.
     *
     * If two filters register for the same position, then the filters will be
     * visited in the order in which they were registered.
     *
     * If the filter is already registered, then its position will be updated.
     *
     * After filters have been run, any disabled or disallowed proxies will be
     * removed from the list.  A proxy is disabled if it had previously failed-
     * over to another proxy (see getFailoverForProxy).  A proxy is disallowed,
     * for example, if it is a HTTP proxy and the nsIProtocolHandler for the
     * queried URI does not permit proxying via HTTP.
     *
     * If a nsIProtocolHandler disallows all proxying, then filters will never
     * have a chance to intercept proxy requests for such URLs.
     *
     * @param aFilter
     *        The nsIProtocolProxyFilter instance to be registered.
     * @param aPosition
     *        The position of the filter.
     *
     * NOTE: It is possible to construct filters that compete with one another
     * in undesirable ways.  This API does not attempt to protect against such
     * problems.  It is recommended that any extensions that choose to call
     * this method make their position value configurable at runtime (perhaps
     * via the preferences service).
     */
  /* void registerFilter (in nsIProtocolProxyFilter aFilter, in unsigned long aPosition); */
  NS_IMETHOD RegisterFilter(nsIProtocolProxyFilter *aFilter, PRUint32 aPosition) = 0;

  /**
     * This method may be used to unregister a proxy filter instance.  All
     * filters will be automatically unregistered at XPCOM shutdown.
     *
     * @param aFilter
     *        The nsIProtocolProxyFilter instance to be unregistered.
     */
  /* void unregisterFilter (in nsIProtocolProxyFilter aFilter); */
  NS_IMETHOD UnregisterFilter(nsIProtocolProxyFilter *aFilter) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIPROTOCOLPROXYSERVICE \
  NS_IMETHOD Resolve(nsIURI *aURI, PRUint32 aFlags, nsIProxyInfo **_retval); \
  NS_IMETHOD AsyncResolve(nsIURI *aURI, PRUint32 aFlags, nsIProtocolProxyCallback *aCallback, nsICancelable **_retval); \
  NS_IMETHOD NewProxyInfo(const nsACString & aType, const nsACString & aHost, PRInt32 aPort, PRUint32 aFlags, PRUint32 aFailoverTimeout, nsIProxyInfo *aFailoverProxy, nsIProxyInfo **_retval); \
  NS_IMETHOD GetFailoverForProxy(nsIProxyInfo *aProxyInfo, nsIURI *aURI, nsresult aReason, nsIProxyInfo **_retval); \
  NS_IMETHOD RegisterFilter(nsIProtocolProxyFilter *aFilter, PRUint32 aPosition); \
  NS_IMETHOD UnregisterFilter(nsIProtocolProxyFilter *aFilter); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIPROTOCOLPROXYSERVICE(_to) \
  NS_IMETHOD Resolve(nsIURI *aURI, PRUint32 aFlags, nsIProxyInfo **_retval) { return _to Resolve(aURI, aFlags, _retval); } \
  NS_IMETHOD AsyncResolve(nsIURI *aURI, PRUint32 aFlags, nsIProtocolProxyCallback *aCallback, nsICancelable **_retval) { return _to AsyncResolve(aURI, aFlags, aCallback, _retval); } \
  NS_IMETHOD NewProxyInfo(const nsACString & aType, const nsACString & aHost, PRInt32 aPort, PRUint32 aFlags, PRUint32 aFailoverTimeout, nsIProxyInfo *aFailoverProxy, nsIProxyInfo **_retval) { return _to NewProxyInfo(aType, aHost, aPort, aFlags, aFailoverTimeout, aFailoverProxy, _retval); } \
  NS_IMETHOD GetFailoverForProxy(nsIProxyInfo *aProxyInfo, nsIURI *aURI, nsresult aReason, nsIProxyInfo **_retval) { return _to GetFailoverForProxy(aProxyInfo, aURI, aReason, _retval); } \
  NS_IMETHOD RegisterFilter(nsIProtocolProxyFilter *aFilter, PRUint32 aPosition) { return _to RegisterFilter(aFilter, aPosition); } \
  NS_IMETHOD UnregisterFilter(nsIProtocolProxyFilter *aFilter) { return _to UnregisterFilter(aFilter); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIPROTOCOLPROXYSERVICE(_to) \
  NS_IMETHOD Resolve(nsIURI *aURI, PRUint32 aFlags, nsIProxyInfo **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Resolve(aURI, aFlags, _retval); } \
  NS_IMETHOD AsyncResolve(nsIURI *aURI, PRUint32 aFlags, nsIProtocolProxyCallback *aCallback, nsICancelable **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->AsyncResolve(aURI, aFlags, aCallback, _retval); } \
  NS_IMETHOD NewProxyInfo(const nsACString & aType, const nsACString & aHost, PRInt32 aPort, PRUint32 aFlags, PRUint32 aFailoverTimeout, nsIProxyInfo *aFailoverProxy, nsIProxyInfo **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->NewProxyInfo(aType, aHost, aPort, aFlags, aFailoverTimeout, aFailoverProxy, _retval); } \
  NS_IMETHOD GetFailoverForProxy(nsIProxyInfo *aProxyInfo, nsIURI *aURI, nsresult aReason, nsIProxyInfo **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFailoverForProxy(aProxyInfo, aURI, aReason, _retval); } \
  NS_IMETHOD RegisterFilter(nsIProtocolProxyFilter *aFilter, PRUint32 aPosition) { return !_to ? NS_ERROR_NULL_POINTER : _to->RegisterFilter(aFilter, aPosition); } \
  NS_IMETHOD UnregisterFilter(nsIProtocolProxyFilter *aFilter) { return !_to ? NS_ERROR_NULL_POINTER : _to->UnregisterFilter(aFilter); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsProtocolProxyService : public nsIProtocolProxyService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPROTOCOLPROXYSERVICE

  nsProtocolProxyService();

private:
  ~nsProtocolProxyService();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsProtocolProxyService, nsIProtocolProxyService)

nsProtocolProxyService::nsProtocolProxyService()
{
  /* member initializers and constructor code */
}

nsProtocolProxyService::~nsProtocolProxyService()
{
  /* destructor code */
}

/* nsIProxyInfo resolve (in nsIURI aURI, in unsigned long aFlags); */
NS_IMETHODIMP nsProtocolProxyService::Resolve(nsIURI *aURI, PRUint32 aFlags, nsIProxyInfo **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsICancelable asyncResolve (in nsIURI aURI, in unsigned long aFlags, in nsIProtocolProxyCallback aCallback); */
NS_IMETHODIMP nsProtocolProxyService::AsyncResolve(nsIURI *aURI, PRUint32 aFlags, nsIProtocolProxyCallback *aCallback, nsICancelable **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIProxyInfo newProxyInfo (in ACString aType, in AUTF8String aHost, in long aPort, in unsigned long aFlags, in unsigned long aFailoverTimeout, in nsIProxyInfo aFailoverProxy); */
NS_IMETHODIMP nsProtocolProxyService::NewProxyInfo(const nsACString & aType, const nsACString & aHost, PRInt32 aPort, PRUint32 aFlags, PRUint32 aFailoverTimeout, nsIProxyInfo *aFailoverProxy, nsIProxyInfo **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIProxyInfo getFailoverForProxy (in nsIProxyInfo aProxyInfo, in nsIURI aURI, in nsresult aReason); */
NS_IMETHODIMP nsProtocolProxyService::GetFailoverForProxy(nsIProxyInfo *aProxyInfo, nsIURI *aURI, nsresult aReason, nsIProxyInfo **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void registerFilter (in nsIProtocolProxyFilter aFilter, in unsigned long aPosition); */
NS_IMETHODIMP nsProtocolProxyService::RegisterFilter(nsIProtocolProxyFilter *aFilter, PRUint32 aPosition)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void unregisterFilter (in nsIProtocolProxyFilter aFilter); */
NS_IMETHODIMP nsProtocolProxyService::UnregisterFilter(nsIProtocolProxyFilter *aFilter)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIProtocolProxyService_h__ */
