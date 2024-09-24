/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/stylesheets/nsIDOMNSDocumentStyle.idl
 */

#ifndef __gen_nsIDOMNSDocumentStyle_h__
#define __gen_nsIDOMNSDocumentStyle_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

#ifndef __gen_nsIDOMDocumentStyle_h__
#include "nsIDOMDocumentStyle.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMNSDocumentStyle */
#define NS_IDOMNSDOCUMENTSTYLE_IID_STR "4ecdf254-a21e-47b0-8d72-55da8208299f"

#define NS_IDOMNSDOCUMENTSTYLE_IID \
  {0x4ecdf254, 0xa21e, 0x47b0, \
    { 0x8d, 0x72, 0x55, 0xda, 0x82, 0x08, 0x29, 0x9f }}

/**
 * The nsIDOMNSDocumentStyle interface is an extension to the
 * nsIDOMDocumentStyle interface.  This interface exposes more ways to interact
 * with style sheets in the Document Object Model.  This interface is currently
 * very much experimental.
 */
class NS_NO_VTABLE nsIDOMNSDocumentStyle : public nsIDOMDocumentStyle {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMNSDOCUMENTSTYLE_IID)

  /**
   * This attribute indicates the preferredStylesheetSet as set by the
   * author. It is determined from the order of stylesheet declarations and the
   * Default-Style HTTP headers. See [[HTML4]]. If there is no preferred
   * stylesheet set, this attribute returns the empty string. The case of this
   * attribute must exactly match the case given by the author where the
   * preferred stylesheet is specified or implied.
   */
  /* readonly attribute DOMString preferredStylesheetSet; */
  NS_IMETHOD GetPreferredStylesheetSet(nsAString & aPreferredStylesheetSet) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMNSDOCUMENTSTYLE \
  NS_IMETHOD GetPreferredStylesheetSet(nsAString & aPreferredStylesheetSet); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMNSDOCUMENTSTYLE(_to) \
  NS_IMETHOD GetPreferredStylesheetSet(nsAString & aPreferredStylesheetSet) { return _to GetPreferredStylesheetSet(aPreferredStylesheetSet); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMNSDOCUMENTSTYLE(_to) \
  NS_IMETHOD GetPreferredStylesheetSet(nsAString & aPreferredStylesheetSet) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPreferredStylesheetSet(aPreferredStylesheetSet); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMNSDocumentStyle : public nsIDOMNSDocumentStyle
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMNSDOCUMENTSTYLE

  nsDOMNSDocumentStyle();

private:
  ~nsDOMNSDocumentStyle();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMNSDocumentStyle, nsIDOMNSDocumentStyle)

nsDOMNSDocumentStyle::nsDOMNSDocumentStyle()
{
  /* member initializers and constructor code */
}

nsDOMNSDocumentStyle::~nsDOMNSDocumentStyle()
{
  /* destructor code */
}

/* readonly attribute DOMString preferredStylesheetSet; */
NS_IMETHODIMP nsDOMNSDocumentStyle::GetPreferredStylesheetSet(nsAString & aPreferredStylesheetSet)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMNSDocumentStyle_h__ */
