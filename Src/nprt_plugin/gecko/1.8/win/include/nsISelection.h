/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/content/base/public/nsISelection.idl
 */

#ifndef __gen_nsISelection_h__
#define __gen_nsISelection_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMNode; /* forward declaration */

class nsIDOMRange; /* forward declaration */


/* starting interface:    nsISelection */
#define NS_ISELECTION_IID_STR "b2c7ed59-8634-4352-9e37-5484c8b6e4e1"

#define NS_ISELECTION_IID \
  {0xb2c7ed59, 0x8634, 0x4352, \
    { 0x9e, 0x37, 0x54, 0x84, 0xc8, 0xb6, 0xe4, 0xe1 }}

/**
 * Interface for manipulating and querying the current selected range
 * of nodes within the document.
 *
 * @status FROZEN
 * @version 1.0
 */
class NS_NO_VTABLE nsISelection : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISELECTION_IID)

  /**
     * The node representing one end of the selection.
     */
  /* readonly attribute nsIDOMNode anchorNode; */
  NS_IMETHOD GetAnchorNode(nsIDOMNode * *aAnchorNode) = 0;

  /**
     * The offset within the (text) node where the selection begins.
     */
  /* readonly attribute long anchorOffset; */
  NS_IMETHOD GetAnchorOffset(PRInt32 *aAnchorOffset) = 0;

  /**
     * The node with keyboard focus.
     */
  /* readonly attribute nsIDOMNode focusNode; */
  NS_IMETHOD GetFocusNode(nsIDOMNode * *aFocusNode) = 0;

  /**
     * The offset within the (text) node where focus starts.
     */
  /* readonly attribute long focusOffset; */
  NS_IMETHOD GetFocusOffset(PRInt32 *aFocusOffset) = 0;

  /**
     * Indicates if the selection is collapsed or not.
     */
  /* readonly attribute boolean isCollapsed; */
  NS_IMETHOD GetIsCollapsed(PRBool *aIsCollapsed) = 0;

  /**
     * Returns the number of ranges in the selection.
     */
  /* readonly attribute long rangeCount; */
  NS_IMETHOD GetRangeCount(PRInt32 *aRangeCount) = 0;

  /**
     * Returns the range at the specified index.
     */
  /* nsIDOMRange getRangeAt (in long index); */
  NS_IMETHOD GetRangeAt(PRInt32 index, nsIDOMRange **_retval) = 0;

  /**
     * Collapses the selection to a single point, at the specified offset
     * in the given DOM node. When the selection is collapsed, and the content
     * is focused and editable, the caret will blink there.
     * @param parentNode      The given dom node where the selection will be set
     * @param offset          Where in given dom node to place the selection (the offset into the given node)
     */
  /* void collapse (in nsIDOMNode parentNode, in long offset); */
  NS_IMETHOD Collapse(nsIDOMNode *parentNode, PRInt32 offset) = 0;

  /**
     * Extends the selection by moving the focus to the specified node and offset,
     * preserving the anchor postion.  The new selection end result will always
     * be from the anchor to the new focus, regardless of direction.
     * @param parentNode      The node where the selection will be extended to
     * @param offset          Where in node to place the offset in the new focused node
     */
  /* void extend (in nsIDOMNode parentNode, in long offset); */
  NS_IMETHOD Extend(nsIDOMNode *parentNode, PRInt32 offset) = 0;

  /**
     * Collapses the whole selection to a single point at the start
     * of the current selection (irrespective of direction).  If content
     * is focused and editable, the caret will blink there.
     */
  /* void collapseToStart (); */
  NS_IMETHOD CollapseToStart(void) = 0;

  /**
     * Collapses the whole selection to a single point at the end
     * of the current selection (irrespective of direction).  If content
     * is focused and editable, the caret will blink there.
     */
  /* void collapseToEnd (); */
  NS_IMETHOD CollapseToEnd(void) = 0;

  /**
     * The value of entirelyContained determines the detail of the search to determine if
     * the selection contains the node.  If entirelyContained is set to PR_TRUE, t
     * or false if
     * @param node      The node where the selection will be extended to
     * @param entirelyContained Whether
     */
  /* boolean containsNode (in nsIDOMNode node, in boolean entirelyContained); */
  NS_IMETHOD ContainsNode(nsIDOMNode *node, PRBool entirelyContained, PRBool *_retval) = 0;

  /**
     * Adds all children of the specified node to the selection.
     * @param parentNode  the parent of the children to be added to the selection.
     */
  /* void selectAllChildren (in nsIDOMNode parentNode); */
  NS_IMETHOD SelectAllChildren(nsIDOMNode *parentNode) = 0;

  /**
     * Adds a range to the current selection.
     */
  /* void addRange (in nsIDOMRange range); */
  NS_IMETHOD AddRange(nsIDOMRange *range) = 0;

  /**
     * Removes a range from the current selection.
     */
  /* void removeRange (in nsIDOMRange range); */
  NS_IMETHOD RemoveRange(nsIDOMRange *range) = 0;

  /**
     * Removes all ranges from the current selection.
     */
  /* void removeAllRanges (); */
  NS_IMETHOD RemoveAllRanges(void) = 0;

  /**
     * Deletes this selection from document the nodes belong to.
     */
  /* void deleteFromDocument (); */
  NS_IMETHOD DeleteFromDocument(void) = 0;

  /**
     * Modifies the cursor Bidi level after a change in keyboard direction
     * @param langRTL is PR_TRUE if the new language is right-to-left or
     *                PR_FALSE if the new language is left-to-right.
     */
  /* void selectionLanguageChange (in boolean langRTL); */
  NS_IMETHOD SelectionLanguageChange(PRBool langRTL) = 0;

  /**
     * Returns the whole selection into a plain text string.
     */
  /* wstring toString (); */
  NS_IMETHOD ToString(PRUnichar **_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISELECTION \
  NS_IMETHOD GetAnchorNode(nsIDOMNode * *aAnchorNode); \
  NS_IMETHOD GetAnchorOffset(PRInt32 *aAnchorOffset); \
  NS_IMETHOD GetFocusNode(nsIDOMNode * *aFocusNode); \
  NS_IMETHOD GetFocusOffset(PRInt32 *aFocusOffset); \
  NS_IMETHOD GetIsCollapsed(PRBool *aIsCollapsed); \
  NS_IMETHOD GetRangeCount(PRInt32 *aRangeCount); \
  NS_IMETHOD GetRangeAt(PRInt32 index, nsIDOMRange **_retval); \
  NS_IMETHOD Collapse(nsIDOMNode *parentNode, PRInt32 offset); \
  NS_IMETHOD Extend(nsIDOMNode *parentNode, PRInt32 offset); \
  NS_IMETHOD CollapseToStart(void); \
  NS_IMETHOD CollapseToEnd(void); \
  NS_IMETHOD ContainsNode(nsIDOMNode *node, PRBool entirelyContained, PRBool *_retval); \
  NS_IMETHOD SelectAllChildren(nsIDOMNode *parentNode); \
  NS_IMETHOD AddRange(nsIDOMRange *range); \
  NS_IMETHOD RemoveRange(nsIDOMRange *range); \
  NS_IMETHOD RemoveAllRanges(void); \
  NS_IMETHOD DeleteFromDocument(void); \
  NS_IMETHOD SelectionLanguageChange(PRBool langRTL); \
  NS_IMETHOD ToString(PRUnichar **_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISELECTION(_to) \
  NS_IMETHOD GetAnchorNode(nsIDOMNode * *aAnchorNode) { return _to GetAnchorNode(aAnchorNode); } \
  NS_IMETHOD GetAnchorOffset(PRInt32 *aAnchorOffset) { return _to GetAnchorOffset(aAnchorOffset); } \
  NS_IMETHOD GetFocusNode(nsIDOMNode * *aFocusNode) { return _to GetFocusNode(aFocusNode); } \
  NS_IMETHOD GetFocusOffset(PRInt32 *aFocusOffset) { return _to GetFocusOffset(aFocusOffset); } \
  NS_IMETHOD GetIsCollapsed(PRBool *aIsCollapsed) { return _to GetIsCollapsed(aIsCollapsed); } \
  NS_IMETHOD GetRangeCount(PRInt32 *aRangeCount) { return _to GetRangeCount(aRangeCount); } \
  NS_IMETHOD GetRangeAt(PRInt32 index, nsIDOMRange **_retval) { return _to GetRangeAt(index, _retval); } \
  NS_IMETHOD Collapse(nsIDOMNode *parentNode, PRInt32 offset) { return _to Collapse(parentNode, offset); } \
  NS_IMETHOD Extend(nsIDOMNode *parentNode, PRInt32 offset) { return _to Extend(parentNode, offset); } \
  NS_IMETHOD CollapseToStart(void) { return _to CollapseToStart(); } \
  NS_IMETHOD CollapseToEnd(void) { return _to CollapseToEnd(); } \
  NS_IMETHOD ContainsNode(nsIDOMNode *node, PRBool entirelyContained, PRBool *_retval) { return _to ContainsNode(node, entirelyContained, _retval); } \
  NS_IMETHOD SelectAllChildren(nsIDOMNode *parentNode) { return _to SelectAllChildren(parentNode); } \
  NS_IMETHOD AddRange(nsIDOMRange *range) { return _to AddRange(range); } \
  NS_IMETHOD RemoveRange(nsIDOMRange *range) { return _to RemoveRange(range); } \
  NS_IMETHOD RemoveAllRanges(void) { return _to RemoveAllRanges(); } \
  NS_IMETHOD DeleteFromDocument(void) { return _to DeleteFromDocument(); } \
  NS_IMETHOD SelectionLanguageChange(PRBool langRTL) { return _to SelectionLanguageChange(langRTL); } \
  NS_IMETHOD ToString(PRUnichar **_retval) { return _to ToString(_retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISELECTION(_to) \
  NS_IMETHOD GetAnchorNode(nsIDOMNode * *aAnchorNode) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAnchorNode(aAnchorNode); } \
  NS_IMETHOD GetAnchorOffset(PRInt32 *aAnchorOffset) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAnchorOffset(aAnchorOffset); } \
  NS_IMETHOD GetFocusNode(nsIDOMNode * *aFocusNode) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFocusNode(aFocusNode); } \
  NS_IMETHOD GetFocusOffset(PRInt32 *aFocusOffset) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFocusOffset(aFocusOffset); } \
  NS_IMETHOD GetIsCollapsed(PRBool *aIsCollapsed) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetIsCollapsed(aIsCollapsed); } \
  NS_IMETHOD GetRangeCount(PRInt32 *aRangeCount) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRangeCount(aRangeCount); } \
  NS_IMETHOD GetRangeAt(PRInt32 index, nsIDOMRange **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRangeAt(index, _retval); } \
  NS_IMETHOD Collapse(nsIDOMNode *parentNode, PRInt32 offset) { return !_to ? NS_ERROR_NULL_POINTER : _to->Collapse(parentNode, offset); } \
  NS_IMETHOD Extend(nsIDOMNode *parentNode, PRInt32 offset) { return !_to ? NS_ERROR_NULL_POINTER : _to->Extend(parentNode, offset); } \
  NS_IMETHOD CollapseToStart(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->CollapseToStart(); } \
  NS_IMETHOD CollapseToEnd(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->CollapseToEnd(); } \
  NS_IMETHOD ContainsNode(nsIDOMNode *node, PRBool entirelyContained, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ContainsNode(node, entirelyContained, _retval); } \
  NS_IMETHOD SelectAllChildren(nsIDOMNode *parentNode) { return !_to ? NS_ERROR_NULL_POINTER : _to->SelectAllChildren(parentNode); } \
  NS_IMETHOD AddRange(nsIDOMRange *range) { return !_to ? NS_ERROR_NULL_POINTER : _to->AddRange(range); } \
  NS_IMETHOD RemoveRange(nsIDOMRange *range) { return !_to ? NS_ERROR_NULL_POINTER : _to->RemoveRange(range); } \
  NS_IMETHOD RemoveAllRanges(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->RemoveAllRanges(); } \
  NS_IMETHOD DeleteFromDocument(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->DeleteFromDocument(); } \
  NS_IMETHOD SelectionLanguageChange(PRBool langRTL) { return !_to ? NS_ERROR_NULL_POINTER : _to->SelectionLanguageChange(langRTL); } \
  NS_IMETHOD ToString(PRUnichar **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ToString(_retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSelection : public nsISelection
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISELECTION

  nsSelection();

private:
  ~nsSelection();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSelection, nsISelection)

nsSelection::nsSelection()
{
  /* member initializers and constructor code */
}

nsSelection::~nsSelection()
{
  /* destructor code */
}

/* readonly attribute nsIDOMNode anchorNode; */
NS_IMETHODIMP nsSelection::GetAnchorNode(nsIDOMNode * *aAnchorNode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long anchorOffset; */
NS_IMETHODIMP nsSelection::GetAnchorOffset(PRInt32 *aAnchorOffset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMNode focusNode; */
NS_IMETHODIMP nsSelection::GetFocusNode(nsIDOMNode * *aFocusNode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long focusOffset; */
NS_IMETHODIMP nsSelection::GetFocusOffset(PRInt32 *aFocusOffset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean isCollapsed; */
NS_IMETHODIMP nsSelection::GetIsCollapsed(PRBool *aIsCollapsed)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long rangeCount; */
NS_IMETHODIMP nsSelection::GetRangeCount(PRInt32 *aRangeCount)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMRange getRangeAt (in long index); */
NS_IMETHODIMP nsSelection::GetRangeAt(PRInt32 index, nsIDOMRange **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void collapse (in nsIDOMNode parentNode, in long offset); */
NS_IMETHODIMP nsSelection::Collapse(nsIDOMNode *parentNode, PRInt32 offset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void extend (in nsIDOMNode parentNode, in long offset); */
NS_IMETHODIMP nsSelection::Extend(nsIDOMNode *parentNode, PRInt32 offset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void collapseToStart (); */
NS_IMETHODIMP nsSelection::CollapseToStart()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void collapseToEnd (); */
NS_IMETHODIMP nsSelection::CollapseToEnd()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean containsNode (in nsIDOMNode node, in boolean entirelyContained); */
NS_IMETHODIMP nsSelection::ContainsNode(nsIDOMNode *node, PRBool entirelyContained, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void selectAllChildren (in nsIDOMNode parentNode); */
NS_IMETHODIMP nsSelection::SelectAllChildren(nsIDOMNode *parentNode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void addRange (in nsIDOMRange range); */
NS_IMETHODIMP nsSelection::AddRange(nsIDOMRange *range)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void removeRange (in nsIDOMRange range); */
NS_IMETHODIMP nsSelection::RemoveRange(nsIDOMRange *range)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void removeAllRanges (); */
NS_IMETHODIMP nsSelection::RemoveAllRanges()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void deleteFromDocument (); */
NS_IMETHODIMP nsSelection::DeleteFromDocument()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void selectionLanguageChange (in boolean langRTL); */
NS_IMETHODIMP nsSelection::SelectionLanguageChange(PRBool langRTL)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* wstring toString (); */
NS_IMETHODIMP nsSelection::ToString(PRUnichar **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISelection_h__ */
