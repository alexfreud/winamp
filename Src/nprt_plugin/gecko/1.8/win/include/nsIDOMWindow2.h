/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/base/nsIDOMWindow2.idl
 */

#ifndef __gen_nsIDOMWindow2_h__
#define __gen_nsIDOMWindow2_h__


#ifndef __gen_nsIDOMWindow_h__
#include "nsIDOMWindow.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMWindow2 */
#define NS_IDOMWINDOW2_IID_STR "65455132-b96a-40ec-adea-52fa22b1028c"

#define NS_IDOMWINDOW2_IID \
  {0x65455132, 0xb96a, 0x40ec, \
    { 0xad, 0xea, 0x52, 0xfa, 0x22, 0xb1, 0x02, 0x8c }}

class NS_NO_VTABLE nsIDOMWindow2 : public nsIDOMWindow {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMWINDOW2_IID)

  /**
   * Get the window root for this window. This is useful for hooking
   * up event listeners to this window and every other window nested
   * in the window root.
   */
  /* [noscript] readonly attribute nsIDOMEventTarget windowRoot; */
  NS_IMETHOD GetWindowRoot(nsIDOMEventTarget * *aWindowRoot) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMWINDOW2 \
  NS_IMETHOD GetWindowRoot(nsIDOMEventTarget * *aWindowRoot); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMWINDOW2(_to) \
  NS_IMETHOD GetWindowRoot(nsIDOMEventTarget * *aWindowRoot) { return _to GetWindowRoot(aWindowRoot); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMWINDOW2(_to) \
  NS_IMETHOD GetWindowRoot(nsIDOMEventTarget * *aWindowRoot) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetWindowRoot(aWindowRoot); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMWindow2 : public nsIDOMWindow2
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMWINDOW2

  nsDOMWindow2();

private:
  ~nsDOMWindow2();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMWindow2, nsIDOMWindow2)

nsDOMWindow2::nsDOMWindow2()
{
  /* member initializers and constructor code */
}

nsDOMWindow2::~nsDOMWindow2()
{
  /* destructor code */
}

/* [noscript] readonly attribute nsIDOMEventTarget windowRoot; */
NS_IMETHODIMP nsDOMWindow2::GetWindowRoot(nsIDOMEventTarget * *aWindowRoot)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMWindow2_h__ */
