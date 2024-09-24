/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/netwerk/base/public/nsIProtocolHandler.idl
 */

#ifndef __gen_nsIProtocolHandler_h__
#define __gen_nsIProtocolHandler_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIURI; /* forward declaration */

class nsIChannel; /* forward declaration */


/* starting interface:    nsIProtocolHandler */
#define NS_IPROTOCOLHANDLER_IID_STR "15fd6940-8ea7-11d3-93ad-00104ba0fd40"

#define NS_IPROTOCOLHANDLER_IID \
  {0x15fd6940, 0x8ea7, 0x11d3, \
    { 0x93, 0xad, 0x00, 0x10, 0x4b, 0xa0, 0xfd, 0x40 }}

/**
 * nsIProtocolHandler
 *
 * @status FROZEN
 */
class NS_NO_VTABLE nsIProtocolHandler : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IPROTOCOLHANDLER_IID)

  /**
     * The scheme of this protocol (e.g., "file").
     */
  /* readonly attribute ACString scheme; */
  NS_IMETHOD GetScheme(nsACString & aScheme) = 0;

  /** 
     * The default port is the port that this protocol normally uses.
     * If a port does not make sense for the protocol (e.g., "about:")
     * then -1 will be returned.
     */
  /* readonly attribute long defaultPort; */
  NS_IMETHOD GetDefaultPort(PRInt32 *aDefaultPort) = 0;

  /**
     * Returns the protocol specific flags (see flag definitions below).  
     */
  /* readonly attribute unsigned long protocolFlags; */
  NS_IMETHOD GetProtocolFlags(PRUint32 *aProtocolFlags) = 0;

  /**
     * Makes a URI object that is suitable for loading by this protocol,
     * where the URI string is given as an UTF-8 string.  The caller may
     * provide the charset from which the URI string originated, so that
     * the URI string can be translated back to that charset (if necessary)
     * before communicating with, for example, the origin server of the URI
     * string.  (Many servers do not support UTF-8 IRIs at the present time,
     * so we must be careful about tracking the native charset of the origin
     * server.)
     *
     * @param aSpec          - the URI string in UTF-8 encoding. depending
     *                         on the protocol implementation, unicode character
     *                         sequences may or may not be %xx escaped.
     * @param aOriginCharset - the charset of the document from which this URI
     *                         string originated.  this corresponds to the
     *                         charset that should be used when communicating
     *                         this URI to an origin server, for example.  if
     *                         null, then UTF-8 encoding is assumed (i.e.,
     *                         no charset transformation from aSpec).
     * @param aBaseURI       - if null, aSpec must specify an absolute URI.
     *                         otherwise, aSpec may be resolved relative
     *                         to aBaseURI, depending on the protocol. 
     *                         If the protocol has no concept of relative 
     *                         URI aBaseURI will simply be ignored.
     */
  /* nsIURI newURI (in AUTF8String aSpec, in string aOriginCharset, in nsIURI aBaseURI); */
  NS_IMETHOD NewURI(const nsACString & aSpec, const char *aOriginCharset, nsIURI *aBaseURI, nsIURI **_retval) = 0;

  /**
     * Constructs a new channel from the given URI for this protocol handler. 
     */
  /* nsIChannel newChannel (in nsIURI aURI); */
  NS_IMETHOD NewChannel(nsIURI *aURI, nsIChannel **_retval) = 0;

  /**
     * Allows a protocol to override blacklisted ports.
     *
     * This method will be called when there is an attempt to connect to a port 
     * that is blacklisted.  For example, for most protocols, port 25 (Simple Mail
     * Transfer) is banned.  When a URI containing this "known-to-do-bad-things" 
     * port number is encountered, this function will be called to ask if the 
     * protocol handler wants to override the ban.  
     */
  /* boolean allowPort (in long port, in string scheme); */
  NS_IMETHOD AllowPort(PRInt32 port, const char *scheme, PRBool *_retval) = 0;

  /**************************************************************************
     * Constants for the protocol flags (the first is the default mask, the
     * others are deviations):
     *
     * NOTE: Implementation must ignore any flags they do not understand.
     */
/**
     * standard full URI with authority component and concept of relative
     * URIs (http, ftp, ...)
     */
  enum { URI_STD = 0U };

  /**
     * no concept of relative URIs (about, javascript, finger, ...)
     */
  enum { URI_NORELATIVE = 1U };

  /**
     * no authority component (file, ...)
     */
  enum { URI_NOAUTH = 2U };

  /**
     * This protocol handler can be proxied via a proxy (socks or http)
     * (e.g., irc, smtp, http, etc.).  If the protocol supports transparent
     * proxying, the handler should implement nsIProxiedProtocolHandler.
     *
     * If it supports only HTTP proxying, then it need not support
     * nsIProxiedProtocolHandler, but should instead set the ALLOWS_PROXY_HTTP
     * flag (see below).
     *
     * @see nsIProxiedProtocolHandler
     */
  enum { ALLOWS_PROXY = 4U };

  /**
     * This protocol handler can be proxied using a http proxy (e.g., http,
     * ftp, etc.).  nsIIOService::newChannelFromURI will feed URIs from this
     * protocol handler to the HTTP protocol handler instead.  This flag is
     * ignored if ALLOWS_PROXY is not set.
     */
  enum { ALLOWS_PROXY_HTTP = 8U };

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIPROTOCOLHANDLER \
  NS_IMETHOD GetScheme(nsACString & aScheme); \
  NS_IMETHOD GetDefaultPort(PRInt32 *aDefaultPort); \
  NS_IMETHOD GetProtocolFlags(PRUint32 *aProtocolFlags); \
  NS_IMETHOD NewURI(const nsACString & aSpec, const char *aOriginCharset, nsIURI *aBaseURI, nsIURI **_retval); \
  NS_IMETHOD NewChannel(nsIURI *aURI, nsIChannel **_retval); \
  NS_IMETHOD AllowPort(PRInt32 port, const char *scheme, PRBool *_retval); \

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIPROTOCOLHANDLER(_to) \
  NS_IMETHOD GetScheme(nsACString & aScheme) { return _to GetScheme(aScheme); } \
  NS_IMETHOD GetDefaultPort(PRInt32 *aDefaultPort) { return _to GetDefaultPort(aDefaultPort); } \
  NS_IMETHOD GetProtocolFlags(PRUint32 *aProtocolFlags) { return _to GetProtocolFlags(aProtocolFlags); } \
  NS_IMETHOD NewURI(const nsACString & aSpec, const char *aOriginCharset, nsIURI *aBaseURI, nsIURI **_retval) { return _to NewURI(aSpec, aOriginCharset, aBaseURI, _retval); } \
  NS_IMETHOD NewChannel(nsIURI *aURI, nsIChannel **_retval) { return _to NewChannel(aURI, _retval); } \
  NS_IMETHOD AllowPort(PRInt32 port, const char *scheme, PRBool *_retval) { return _to AllowPort(port, scheme, _retval); } \

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIPROTOCOLHANDLER(_to) \
  NS_IMETHOD GetScheme(nsACString & aScheme) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetScheme(aScheme); } \
  NS_IMETHOD GetDefaultPort(PRInt32 *aDefaultPort) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDefaultPort(aDefaultPort); } \
  NS_IMETHOD GetProtocolFlags(PRUint32 *aProtocolFlags) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetProtocolFlags(aProtocolFlags); } \
  NS_IMETHOD NewURI(const nsACString & aSpec, const char *aOriginCharset, nsIURI *aBaseURI, nsIURI **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->NewURI(aSpec, aOriginCharset, aBaseURI, _retval); } \
  NS_IMETHOD NewChannel(nsIURI *aURI, nsIChannel **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->NewChannel(aURI, _retval); } \
  NS_IMETHOD AllowPort(PRInt32 port, const char *scheme, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->AllowPort(port, scheme, _retval); } \

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsProtocolHandler : public nsIProtocolHandler
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPROTOCOLHANDLER

  nsProtocolHandler();

private:
  ~nsProtocolHandler();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsProtocolHandler, nsIProtocolHandler)

nsProtocolHandler::nsProtocolHandler()
{
  /* member initializers and constructor code */
}

nsProtocolHandler::~nsProtocolHandler()
{
  /* destructor code */
}

/* readonly attribute ACString scheme; */
NS_IMETHODIMP nsProtocolHandler::GetScheme(nsACString & aScheme)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long defaultPort; */
NS_IMETHODIMP nsProtocolHandler::GetDefaultPort(PRInt32 *aDefaultPort)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long protocolFlags; */
NS_IMETHODIMP nsProtocolHandler::GetProtocolFlags(PRUint32 *aProtocolFlags)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIURI newURI (in AUTF8String aSpec, in string aOriginCharset, in nsIURI aBaseURI); */
NS_IMETHODIMP nsProtocolHandler::NewURI(const nsACString & aSpec, const char *aOriginCharset, nsIURI *aBaseURI, nsIURI **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIChannel newChannel (in nsIURI aURI); */
NS_IMETHODIMP nsProtocolHandler::NewChannel(nsIURI *aURI, nsIChannel **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean allowPort (in long port, in string scheme); */
NS_IMETHODIMP nsProtocolHandler::AllowPort(PRInt32 port, const char *scheme, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

/**
 * Protocol handlers are registered with XPCOM under the following CONTRACTID prefix:
 */
#define NS_NETWORK_PROTOCOL_CONTRACTID_PREFIX "@mozilla.org/network/protocol;1?name="
/**
 * For example, "@mozilla.org/network/protocol;1?name=http"
 */

#endif /* __gen_nsIProtocolHandler_h__ */
