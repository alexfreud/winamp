/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/core/nsIDOMDocumentType.idl
 */

#ifndef __gen_nsIDOMDocumentType_h__
#define __gen_nsIDOMDocumentType_h__


#ifndef __gen_nsIDOMNode_h__
#include "nsIDOMNode.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMDocumentType */
#define NS_IDOMDOCUMENTTYPE_IID_STR "a6cf9077-15b3-11d2-932e-00805f8add32"

#define NS_IDOMDOCUMENTTYPE_IID \
  {0xa6cf9077, 0x15b3, 0x11d2, \
    { 0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32 }}

class NS_NO_VTABLE nsIDOMDocumentType : public nsIDOMNode {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMDOCUMENTTYPE_IID)

  /**
 * Each Document has a doctype attribute whose value is either null 
 * or a DocumentType object. 
 * The nsIDOMDocumentType interface in the DOM Core provides an 
 * interface to the list of entities that are defined for the document.
 *
 * For more information on this interface please see 
 * http://www.w3.org/TR/DOM-Level-2-Core/
 *
 * @status FROZEN
 */
  /* readonly attribute DOMString name; */
  NS_IMETHOD GetName(nsAString & aName) = 0;

  /* readonly attribute nsIDOMNamedNodeMap entities; */
  NS_IMETHOD GetEntities(nsIDOMNamedNodeMap * *aEntities) = 0;

  /* readonly attribute nsIDOMNamedNodeMap notations; */
  NS_IMETHOD GetNotations(nsIDOMNamedNodeMap * *aNotations) = 0;

  /* readonly attribute DOMString publicId; */
  NS_IMETHOD GetPublicId(nsAString & aPublicId) = 0;

  /* readonly attribute DOMString systemId; */
  NS_IMETHOD GetSystemId(nsAString & aSystemId) = 0;

  /* readonly attribute DOMString internalSubset; */
  NS_IMETHOD GetInternalSubset(nsAString & aInternalSubset) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMDOCUMENTTYPE \
  NS_IMETHOD GetName(nsAString & aName); \
  NS_IMETHOD GetEntities(nsIDOMNamedNodeMap * *aEntities); \
  NS_IMETHOD GetNotations(nsIDOMNamedNodeMap * *aNotations); \
  NS_IMETHOD GetPublicId(nsAString & aPublicId); \
  NS_IMETHOD GetSystemId(nsAString & aSystemId); \
  NS_IMETHOD GetInternalSubset(nsAString & aInternalSubset); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMDOCUMENTTYPE(_to) \
  NS_IMETHOD GetName(nsAString & aName) { return _to GetName(aName); } \
  NS_IMETHOD GetEntities(nsIDOMNamedNodeMap * *aEntities) { return _to GetEntities(aEntities); } \
  NS_IMETHOD GetNotations(nsIDOMNamedNodeMap * *aNotations) { return _to GetNotations(aNotations); } \
  NS_IMETHOD GetPublicId(nsAString & aPublicId) { return _to GetPublicId(aPublicId); } \
  NS_IMETHOD GetSystemId(nsAString & aSystemId) { return _to GetSystemId(aSystemId); } \
  NS_IMETHOD GetInternalSubset(nsAString & aInternalSubset) { return _to GetInternalSubset(aInternalSubset); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMDOCUMENTTYPE(_to) \
  NS_IMETHOD GetName(nsAString & aName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetName(aName); } \
  NS_IMETHOD GetEntities(nsIDOMNamedNodeMap * *aEntities) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetEntities(aEntities); } \
  NS_IMETHOD GetNotations(nsIDOMNamedNodeMap * *aNotations) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetNotations(aNotations); } \
  NS_IMETHOD GetPublicId(nsAString & aPublicId) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPublicId(aPublicId); } \
  NS_IMETHOD GetSystemId(nsAString & aSystemId) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSystemId(aSystemId); } \
  NS_IMETHOD GetInternalSubset(nsAString & aInternalSubset) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetInternalSubset(aInternalSubset); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMDocumentType : public nsIDOMDocumentType
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMDOCUMENTTYPE

  nsDOMDocumentType();

private:
  ~nsDOMDocumentType();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMDocumentType, nsIDOMDocumentType)

nsDOMDocumentType::nsDOMDocumentType()
{
  /* member initializers and constructor code */
}

nsDOMDocumentType::~nsDOMDocumentType()
{
  /* destructor code */
}

/* readonly attribute DOMString name; */
NS_IMETHODIMP nsDOMDocumentType::GetName(nsAString & aName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMNamedNodeMap entities; */
NS_IMETHODIMP nsDOMDocumentType::GetEntities(nsIDOMNamedNodeMap * *aEntities)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMNamedNodeMap notations; */
NS_IMETHODIMP nsDOMDocumentType::GetNotations(nsIDOMNamedNodeMap * *aNotations)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString publicId; */
NS_IMETHODIMP nsDOMDocumentType::GetPublicId(nsAString & aPublicId)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString systemId; */
NS_IMETHODIMP nsDOMDocumentType::GetSystemId(nsAString & aSystemId)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString internalSubset; */
NS_IMETHODIMP nsDOMDocumentType::GetInternalSubset(nsAString & aInternalSubset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMDocumentType_h__ */
