#ifndef __ACCESSIBLE_H
#define __ACCESSIBLE_H

#include <bfc/common.h>

// Message sent to the OS window in order to retrieve an accessibility interface
#define WM_GETOBJECT            0x003D

// undef some stuff we're gonna #define ourselves
#ifdef OBJID_WINDOW
#undef OBJID_WINDOW
#endif
#ifdef OBJID_SYSMENU
#undef OBJID_SYSMENU   
#endif
#ifdef OBJID_TITLEBAR
#undef OBJID_TITLEBAR      
#endif
#ifdef OBJID_MENU
#undef OBJID_MENU          
#endif
#ifdef OBJID_CLIENT
#undef OBJID_CLIENT        
#endif
#ifdef OBJID_VSCROLL
#undef OBJID_VSCROLL       
#endif
#ifdef OBJID_HSCROLL
#undef OBJID_HSCROLL       
#endif
#ifdef OBJID_SIZEGRIP
#undef OBJID_SIZEGRIP      
#endif
#ifdef OBJID_CARET
#undef OBJID_CARET         
#endif
#ifdef OBJID_CURSOR
#undef OBJID_CURSOR
#endif
#ifdef OBJID_ALERT
#undef OBJID_ALERT
#endif
#ifdef OBJID_SOUND
#undef OBJID_SOUND         
#endif
#ifdef STATE_SYSTEM_VALID
#undef STATE_SYSTEM_VALID
#endif // STATE_SYSTEM_VALID

// Accessibility system object IDs
#define OBJID_WINDOW        0x00000000
#define OBJID_SYSMENU       0xFFFFFFFF
#define OBJID_TITLEBAR      0xFFFFFFFE
#define OBJID_MENU          0xFFFFFFFD
#define OBJID_CLIENT        0xFFFFFFFC
#define OBJID_VSCROLL       0xFFFFFFFB
#define OBJID_HSCROLL       0xFFFFFFFA
#define OBJID_SIZEGRIP      0xFFFFFFF9
#define OBJID_CARET         0xFFFFFFF8
#define OBJID_CURSOR        0xFFFFFFF7
#define OBJID_ALERT         0xFFFFFFF6
#define OBJID_SOUND         0xFFFFFFF5

#if 0
#define ROLE_SYSTEM_TITLEBAR            0x00000001
#define ROLE_SYSTEM_MENUBAR             0x00000002
#define ROLE_SYSTEM_SCROLLBAR           0x00000003
#define ROLE_SYSTEM_GRIP                0x00000004
#define ROLE_SYSTEM_SOUND               0x00000005
#define ROLE_SYSTEM_CURSOR              0x00000006
#define ROLE_SYSTEM_CARET               0x00000007
#define ROLE_SYSTEM_ALERT               0x00000008
#define ROLE_SYSTEM_WINDOW              0x00000009
#define ROLE_SYSTEM_CLIENT              0x0000000A
#define ROLE_SYSTEM_MENUPOPUP           0x0000000B
#define ROLE_SYSTEM_MENUITEM            0x0000000C
#define ROLE_SYSTEM_TOOLTIP             0x0000000D
#define ROLE_SYSTEM_APPLICATION         0x0000000E
#define ROLE_SYSTEM_DOCUMENT            0x0000000F
#define ROLE_SYSTEM_PANE                0x00000010
#define ROLE_SYSTEM_CHART               0x00000011
#define ROLE_SYSTEM_DIALOG              0x00000012
#define ROLE_SYSTEM_BORDER              0x00000013
#define ROLE_SYSTEM_GROUPING            0x00000014
#define ROLE_SYSTEM_SEPARATOR           0x00000015
#define ROLE_SYSTEM_TOOLBAR             0x00000016
#define ROLE_SYSTEM_STATUSBAR           0x00000017
#define ROLE_SYSTEM_TABLE               0x00000018
#define ROLE_SYSTEM_COLUMNHEADER        0x00000019
#define ROLE_SYSTEM_ROWHEADER           0x0000001A
#define ROLE_SYSTEM_COLUMN              0x0000001B
#define ROLE_SYSTEM_ROW                 0x0000001C
#define ROLE_SYSTEM_CELL                0x0000001D
#define ROLE_SYSTEM_LINK                0x0000001E
#define ROLE_SYSTEM_HELPBALLOON         0x0000001F
#define ROLE_SYSTEM_CHARACTER           0x00000020
#define ROLE_SYSTEM_LIST                0x00000021
#define ROLE_SYSTEM_LISTITEM            0x00000022
#define ROLE_SYSTEM_OUTLINE             0x00000023
#define ROLE_SYSTEM_OUTLINEITEM         0x00000024
#define ROLE_SYSTEM_PAGETAB             0x00000025
#define ROLE_SYSTEM_PROPERTYPAGE        0x00000026
#define ROLE_SYSTEM_INDICATOR           0x00000027
#define ROLE_SYSTEM_GRAPHIC             0x00000028
#define ROLE_SYSTEM_STATICTEXT          0x00000029
#define ROLE_SYSTEM_TEXT                0x0000002A  // Editable, selectable, etc.
#define ROLE_SYSTEM_PUSHBUTTON          0x0000002B
#define ROLE_SYSTEM_CHECKBUTTON         0x0000002C
#define ROLE_SYSTEM_RADIOBUTTON         0x0000002D
#define ROLE_SYSTEM_COMBOBOX            0x0000002E
#define ROLE_SYSTEM_DROPLIST            0x0000002F
#define ROLE_SYSTEM_PROGRESSBAR         0x00000030
#define ROLE_SYSTEM_DIAL                0x00000031
#define ROLE_SYSTEM_HOTKEYFIELD         0x00000032
#define ROLE_SYSTEM_SLIDER              0x00000033
#define ROLE_SYSTEM_SPINBUTTON          0x00000034
#define ROLE_SYSTEM_DIAGRAM             0x00000035
#define ROLE_SYSTEM_ANIMATION           0x00000036
#define ROLE_SYSTEM_EQUATION            0x00000037
#define ROLE_SYSTEM_BUTTONDROPDOWN      0x00000038
#define ROLE_SYSTEM_BUTTONMENU          0x00000039
#define ROLE_SYSTEM_BUTTONDROPDOWNGRID  0x0000003A
#define ROLE_SYSTEM_WHITESPACE          0x0000003B
#define ROLE_SYSTEM_PAGETABLIST         0x0000003C
#define ROLE_SYSTEM_CLOCK               0x0000003D
#endif
#define STATE_SYSTEM_UNAVAILABLE        0x00000001  // Disabled
#define STATE_SYSTEM_SELECTED           0x00000002
#define STATE_SYSTEM_FOCUSED            0x00000004
#define STATE_SYSTEM_PRESSED            0x00000008
#define STATE_SYSTEM_CHECKED            0x00000010
#define STATE_SYSTEM_MIXED              0x00000020  // 3-state checkbox or toolbar button
#define STATE_SYSTEM_READONLY           0x00000040
#define STATE_SYSTEM_HOTTRACKED         0x00000080
#define STATE_SYSTEM_DEFAULT            0x00000100
#define STATE_SYSTEM_EXPANDED           0x00000200
#define STATE_SYSTEM_COLLAPSED          0x00000400
#define STATE_SYSTEM_BUSY               0x00000800
#define STATE_SYSTEM_FLOATING           0x00001000  // Children "owned" not "contained" by parent
#define STATE_SYSTEM_MARQUEED           0x00002000
#define STATE_SYSTEM_ANIMATED           0x00004000
#define STATE_SYSTEM_INVISIBLE          0x00008000
#define STATE_SYSTEM_OFFSCREEN          0x00010000
#define STATE_SYSTEM_SIZEABLE           0x00020000
#define STATE_SYSTEM_MOVEABLE           0x00040000
#define STATE_SYSTEM_SELFVOICING        0x00080000
#define STATE_SYSTEM_FOCUSABLE          0x00100000
#define STATE_SYSTEM_SELECTABLE         0x00200000
#define STATE_SYSTEM_LINKED             0x00400000
#define STATE_SYSTEM_TRAVERSED          0x00800000
#define STATE_SYSTEM_MULTISELECTABLE    0x01000000  // Supports multiple selection
#define STATE_SYSTEM_EXTSELECTABLE      0x02000000  // Supports extended selection
#define STATE_SYSTEM_ALERT_LOW          0x04000000  // This information is of low priority
#define STATE_SYSTEM_ALERT_MEDIUM       0x08000000  // This information is of medium priority
#define STATE_SYSTEM_ALERT_HIGH         0x10000000  // This information is of high priority

#define STATE_SYSTEM_VALID              0x1FFFFFFF

/*
 * Reserved IDs for system objects
 */
#define     OBJID_WINDOW        0x00000000
#define     OBJID_SYSMENU       0xFFFFFFFF
#define     OBJID_TITLEBAR      0xFFFFFFFE
#define     OBJID_MENU          0xFFFFFFFD
#define     OBJID_CLIENT        0xFFFFFFFC
#define     OBJID_VSCROLL       0xFFFFFFFB
#define     OBJID_HSCROLL       0xFFFFFFFA
#define     OBJID_SIZEGRIP      0xFFFFFFF9
#define     OBJID_CARET         0xFFFFFFF8
#define     OBJID_CURSOR        0xFFFFFFF7
#define     OBJID_ALERT         0xFFFFFFF6
#define     OBJID_SOUND         0xFFFFFFF5

/*
 * EVENT DEFINITION
 */
#define EVENT_MIN           0x00000001
#define EVENT_MAX           0x7FFFFFFF


/*
 *  EVENT_SYSTEM_SOUND
 *  Sent when a sound is played.  Currently nothing is generating this, we
 *  this event when a system sound (for menus, etc) is played.  Apps
 *  generate this, if accessible, when a private sound is played.  For
 *  example, if Mail plays a "New Mail" sound.
 *
 *  System Sounds:
 *  (Generated by PlaySoundEvent in USER itself)
 *      hwnd            is NULL
 *      idObject        is OBJID_SOUND
 *      idChild         is sound child ID if one
 *  App Sounds:
 *  (PlaySoundEvent won't generate notification; up to app)
 *      hwnd + idObject gets interface pointer to Sound object
 *      idChild identifies the sound in question
 *  are going to be cleaning up the SOUNDSENTRY feature in the control panel
 *  and will use this at that time.  Applications implementing WinEvents
 *  are perfectly welcome to use it.  Clients of IAccessible* will simply
 *  turn around and get back a non-visual object that describes the sound.
 */
#define EVENT_SYSTEM_SOUND              0x0001

/*
 * EVENT_SYSTEM_ALERT
 * System Alerts:
 * (Generated by MessageBox() calls for example)
 *      hwnd            is hwndMessageBox
 *      idObject        is OBJID_ALERT
 * App Alerts:
 * (Generated whenever)
 *      hwnd+idObject gets interface pointer to Alert
 */
#define EVENT_SYSTEM_ALERT              0x0002

/*
 * EVENT_SYSTEM_FOREGROUND
 * Sent when the foreground (active) window changes, even if it is changing
 * to another window in the same thread as the previous one.
 *      hwnd            is hwndNewForeground
 *      idObject        is OBJID_WINDOW
 *      idChild    is INDEXID_OBJECT
 */
#define EVENT_SYSTEM_FOREGROUND         0x0003

/*
 * Menu
 *      hwnd            is window (top level window or popup menu window)
 *      idObject        is ID of control (OBJID_MENU, OBJID_SYSMENU, OBJID_SELF for popup)
 *      idChild         is CHILDID_SELF
 *
 * EVENT_SYSTEM_MENUSTART
 * EVENT_SYSTEM_MENUEND
 * For MENUSTART, hwnd+idObject+idChild refers to the control with the menu bar,
 *  or the control bringing up the context menu.
 *
 * Sent when entering into and leaving from menu mode (system, app bar, and
 * track popups).
 */
#define EVENT_SYSTEM_MENUSTART          0x0004
#define EVENT_SYSTEM_MENUEND            0x0005

/*
 * EVENT_SYSTEM_MENUPOPUPSTART
 * EVENT_SYSTEM_MENUPOPUPEND
 * Sent when a menu popup comes up and just before it is taken down.  Note
 * that for a call to TrackPopupMenu(), a client will see EVENT_SYSTEM_MENUSTART
 * followed almost immediately by EVENT_SYSTEM_MENUPOPUPSTART for the popup
 * being shown.
 *
 * For MENUPOPUP, hwnd+idObject+idChild refers to the NEW popup coming up, not the
 * parent item which is hierarchical.  You can get the parent menu/popup by
 * asking for the accParent object.
 */
#define EVENT_SYSTEM_MENUPOPUPSTART     0x0006
#define EVENT_SYSTEM_MENUPOPUPEND       0x0007


/*
 * EVENT_SYSTEM_CAPTURESTART
 * EVENT_SYSTEM_CAPTUREEND
 * Sent when a window takes the capture and releases the capture.
 */
#define EVENT_SYSTEM_CAPTURESTART       0x0008
#define EVENT_SYSTEM_CAPTUREEND         0x0009

/*
 * Move Size
 * EVENT_SYSTEM_MOVESIZESTART
 * EVENT_SYSTEM_MOVESIZEEND
 * Sent when a window enters and leaves move-size dragging mode.
 */
#define EVENT_SYSTEM_MOVESIZESTART      0x000A
#define EVENT_SYSTEM_MOVESIZEEND        0x000B

/*
 * Context Help
 * EVENT_SYSTEM_CONTEXTHELPSTART
 * EVENT_SYSTEM_CONTEXTHELPEND
 * Sent when a window enters and leaves context sensitive help mode.
 */
#define EVENT_SYSTEM_CONTEXTHELPSTART   0x000C
#define EVENT_SYSTEM_CONTEXTHELPEND     0x000D

/*
 * Drag & Drop
 * EVENT_SYSTEM_DRAGDROPSTART
 * EVENT_SYSTEM_DRAGDROPEND
 * Send the START notification just before going into drag&drop loop.  Send
 * the END notification just after canceling out.
 * Note that it is up to apps and OLE to generate this, since the system
 * doesn't know.  Like EVENT_SYSTEM_SOUND, it will be a while before this
 * is prevalent.
 */
#define EVENT_SYSTEM_DRAGDROPSTART      0x000E
#define EVENT_SYSTEM_DRAGDROPEND        0x000F

/*
 * Dialog
 * Send the START notification right after the dialog is completely
 *  initialized and visible.  Send the END right before the dialog
 *  is hidden and goes away.
 * EVENT_SYSTEM_DIALOGSTART
 * EVENT_SYSTEM_DIALOGEND
 */
#define EVENT_SYSTEM_DIALOGSTART        0x0010
#define EVENT_SYSTEM_DIALOGEND          0x0011

/*
 * EVENT_SYSTEM_SCROLLING
 * EVENT_SYSTEM_SCROLLINGSTART
 * EVENT_SYSTEM_SCROLLINGEND
 * Sent when beginning and ending the tracking of a scrollbar in a window,
 * and also for scrollbar controls.
 */
#define EVENT_SYSTEM_SCROLLINGSTART     0x0012
#define EVENT_SYSTEM_SCROLLINGEND       0x0013

/*
 * Alt-Tab Window
 * Send the START notification right after the switch window is initialized
 * and visible.  Send the END right before it is hidden and goes away.
 * EVENT_SYSTEM_SWITCHSTART
 * EVENT_SYSTEM_SWITCHEND
 */
#define EVENT_SYSTEM_SWITCHSTART        0x0014
#define EVENT_SYSTEM_SWITCHEND          0x0015

/*
 * EVENT_SYSTEM_MINIMIZESTART
 * EVENT_SYSTEM_MINIMIZEEND
 * Sent when a window minimizes and just before it restores.
 */
#define EVENT_SYSTEM_MINIMIZESTART      0x0016
#define EVENT_SYSTEM_MINIMIZEEND        0x0017



/*
 * Object events
 *
 * The system AND apps generate these.  The system generates these for
 * real windows.  Apps generate these for objects within their window which
 * act like a separate control, e.g. an item in a list view.
 *
 * When the system generate them, dwParam2 is always WMOBJID_SELF.  When
 * apps generate them, apps put the has-meaning-to-the-app-only ID value
 * in dwParam2.
 * For all events, if you want detailed accessibility information, callers
 * should
 *      * Call AccessibleObjectFromWindow() with the hwnd, idObject parameters
 *          of the event, and IID_IAccessible as the REFIID, to get back an
 *          IAccessible* to talk to
 *      * Initialize and fill in a VARIANT as VT_I4 with lVal the idChild
 *          parameter of the event.
 *      * If idChild isn't zero, call get_accChild() in the container to see
 *          if the child is an object in its own right.  If so, you will get
 *          back an IDispatch* object for the child.  You should release the
 *          parent, and call QueryInterface() on the child object to get its
 *          IAccessible*.  Then you talk directly to the child.  Otherwise,
 *          if get_accChild() returns you nothing, you should continue to
 *          use the child VARIANT.  You will ask the container for the properties
 *          of the child identified by the VARIANT.  In other words, the
 *          child in this case is accessible but not a full-blown object.
 *          Like a button on a titlebar which is 'small' and has no children.
 */

/*
 * For all EVENT_OBJECT events,
 *      hwnd is the dude to Send the WM_GETOBJECT message to (unless NULL,
 *          see above for system things)
 *      idObject is the ID of the object that can resolve any queries a
 *          client might have.  It's a way to deal with windowless controls,
 *          controls that are just drawn on the screen in some larger parent
 *          window (like SDM), or standard frame elements of a window.
 *      idChild is the piece inside of the object that is affected.  This
 *          allows clients to access things that are too small to have full
 *          blown objects in their own right.  Like the thumb of a scrollbar.
 *          The hwnd/idObject pair gets you to the container, the dude you
 *          probably want to talk to most of the time anyway.  The idChild
 *          can then be passed into the acc properties to get the name/value
 *          of it as needed.
 *
 * Example #1:
 *      System propagating a listbox selection change
 *      EVENT_OBJECT_SELECTION
 *          hwnd == listbox hwnd
 *          idObject == OBJID_WINDOW
 *          idChild == new selected item, or CHILDID_SELF if
 *              nothing now selected within container.
 *      Word '97 propagating a listbox selection change
 *          hwnd == SDM window
 *          idObject == SDM ID to get at listbox 'control'
 *          idChild == new selected item, or CHILDID_SELF if
 *              nothing
 *
 * Example #2:
 *      System propagating a menu item selection on the menu bar
 *      EVENT_OBJECT_SELECTION
 *          hwnd == top level window
 *          idObject == OBJID_MENU
 *          idChild == ID of child menu bar item selected
 *
 * Example #3:
 *      System propagating a dropdown coming off of said menu bar item
 *      EVENT_OBJECT_CREATE
 *          hwnd == popup item
 *          idObject == OBJID_WINDOW
 *          idChild == CHILDID_SELF
 *
 * Example #4:
 *
 * For EVENT_OBJECT_REORDER, the object referred to by hwnd/idObject is the
 * PARENT container in which the zorder is occurring.  This is because if
 * one child is zordering, all of them are changing their relative zorder.
 */
#define EVENT_OBJECT_CREATE                 0x8000  // hwnd + ID + idChild is created item
#define EVENT_OBJECT_DESTROY                0x8001  // hwnd + ID + idChild is destroyed item
#define EVENT_OBJECT_SHOW                   0x8002  // hwnd + ID + idChild is shown item
#define EVENT_OBJECT_HIDE                   0x8003  // hwnd + ID + idChild is hidden item
#define EVENT_OBJECT_REORDER                0x8004  // hwnd + ID + idChild is parent of zordering children
/*
 * NOTE:
 * Minimize the number of notifications!
 *
 * When you are hiding a parent object, obviously all child objects are no
 * longer visible on screen.  They still have the same "visible" status,
 * but are not truly visible.  Hence do not send HIDE notifications for the
 * children also.  One implies all.  The same goes for SHOW.
 */


#define EVENT_OBJECT_FOCUS                  0x8005  // hwnd + ID + idChild is focused item
#define EVENT_OBJECT_SELECTION              0x8006  // hwnd + ID + idChild is selected item (if only one), or idChild is OBJID_WINDOW if complex
#define EVENT_OBJECT_SELECTIONADD           0x8007  // hwnd + ID + idChild is item added
#define EVENT_OBJECT_SELECTIONREMOVE        0x8008  // hwnd + ID + idChild is item removed
#define EVENT_OBJECT_SELECTIONWITHIN        0x8009  // hwnd + ID + idChild is parent of changed selected items

/*
 * NOTES:
 * There is only one "focused" child item in a parent.  This is the place
 * keystrokes are going at a given moment.  Hence only send a notification
 * about where the NEW focus is going.  A NEW item getting the focus already
 * implies that the OLD item is losing it.
 *
 * SELECTION however can be multiple.  Hence the different SELECTION
 * notifications.  Here's when to use each:
 *
 * (1) Send a SELECTION notification in the simple single selection
 *     case (like the focus) when the item with the selection is
 *     merely moving to a different item within a container.  hwnd + ID
 *     is the container control, idChildItem is the new child with the
 *     selection.
 *
 * (2) Send a SELECTIONADD notification when a new item has simply been added
 *     to the selection within a container.  This is appropriate when the
 *     number of newly selected items is very small.  hwnd + ID is the
 *     container control, idChildItem is the new child added to the selection.
 *
 * (3) Send a SELECTIONREMOVE notification when a new item has simply been
 *     removed from the selection within a container.  This is appropriate
 *     when the number of newly selected items is very small, just like
 *     SELECTIONADD.  hwnd + ID is the container control, idChildItem is the
 *     new child removed from the selection.
 *
 * (4) Send a SELECTIONWITHIN notification when the selected items within a
 *     control have changed substantially.  Rather than propagate a large
 *     number of changes to reflect removal for some items, addition of
 *     others, just tell somebody who cares that a lot happened.  It will
 *     be faster an easier for somebody watching to just turn around and
 *     query the container control what the new bunch of selected items
 *     are.
 */

#define EVENT_OBJECT_STATECHANGE            0x800A  // hwnd + ID + idChild is item w/ state change
/*
 * Examples of when to send an EVENT_OBJECT_STATECHANGE include
 *      * It is being enabled/disabled (USER does for windows)
 *      * It is being pressed/released (USER does for buttons)
 *      * It is being checked/unchecked (USER does for radio/check buttons)
 */
#define EVENT_OBJECT_LOCATIONCHANGE         0x800B  // hwnd + ID + idChild is moved/sized item

/*
 * Note:
 * A LOCATIONCHANGE is not sent for every child object when the parent
 * changes shape/moves.  Send one notification for the topmost object
 * that is changing.  For example, if the user resizes a top level window,
 * USER will generate a LOCATIONCHANGE for it, but not for the menu bar,
 * title bar, scrollbars, etc.  that are also changing shape/moving.
 *
 * In other words, it only generates LOCATIONCHANGE notifications for
 * real windows that are moving/sizing.  It will not generate a LOCATIONCHANGE
 * for every non-floating child window when the parent moves (the children are
 * logically moving also on screen, but not relative to the parent).
 *
 * Now, if the app itself resizes child windows as a result of being
 * sized, USER will generate LOCATIONCHANGEs for those dudes also because
 * it doesn't know better.
 *
 * Note also that USER will generate LOCATIONCHANGE notifications for two
 * non-window sys objects:
 *      (1) System caret
 *      (2) Cursor
 */

#define EVENT_OBJECT_NAMECHANGE             0x800C  // hwnd + ID + idChild is item w/ name change
#define EVENT_OBJECT_DESCRIPTIONCHANGE      0x800D  // hwnd + ID + idChild is item w/ desc change
#define EVENT_OBJECT_VALUECHANGE            0x800E  // hwnd + ID + idChild is item w/ value change
#define EVENT_OBJECT_PARENTCHANGE           0x800F  // hwnd + ID + idChild is item w/ new parent
#define EVENT_OBJECT_HELPCHANGE             0x8010  // hwnd + ID + idChild is item w/ help change
#define EVENT_OBJECT_DEFACTIONCHANGE        0x8011  // hwnd + ID + idChild is item w/ def action change
#define EVENT_OBJECT_ACCELERATORCHANGE      0x8012  // hwnd + ID + idChild is item w/ keybd accel change

#ifdef WIN32
WINUSERAPI VOID WINAPI
NotifyWinEvent(
    DWORD event,
    HWND  hwnd,
    LONG  idObject,
    LONG  idChild);
#endif

#include <bfc/dispatch.h>

struct IAccessible;

class Accessible : public Dispatchable {
  public:
    IAccessible *getIAccessible();
#ifdef _WIN32
    HRESULT getOSHandle(int p);
#endif
    void release();
    void addRef();
    int getNumRefs();
    void onGetFocus(int idx=-1);
    void onStateChange(int idx=-1);
    void onSetName(const wchar_t *newname, int idx=-1);
    OSWINDOWHANDLE getOSWnd();
    int flattenContent(OSWINDOWHANDLE *w);

  enum {
    ACCESSIBLE_GETIACCESSIBLE=10,
    ACCESSIBLE_GETOSHANDLE=20,
    ACCESSIBLE_ADDREF=30,
    ACCESSIBLE_RELEASE=40,
    ACCESSIBLE_GETNUMREFS=50,
    ACCESSIBLE_ONGETFOCUS=60,
    ACCESSIBLE_ONSETNAME=70,
    ACCESSIBLE_GETOSWND=80,
    ACCESSIBLE_ONSTATECHANGE=90,
    ACCESSIBLE_FLATTENCONTENT=100,
  };
};

inline IAccessible *Accessible::getIAccessible() {
  return _call(ACCESSIBLE_GETIACCESSIBLE, (IAccessible *)NULL);
}

#ifdef _WIN32
inline HRESULT Accessible::getOSHandle(int p) {
  return _call(ACCESSIBLE_GETOSHANDLE, (HRESULT)NULL, p);
}
#endif

inline void Accessible::addRef() {
  _voidcall(ACCESSIBLE_ADDREF);
}

inline void Accessible::release() {
  _voidcall(ACCESSIBLE_RELEASE);
}

inline int Accessible::getNumRefs() {
  return _call(ACCESSIBLE_GETNUMREFS, 0);
}

inline void Accessible::onGetFocus(int idx/* =-1 */) {
  _voidcall(ACCESSIBLE_ONGETFOCUS, idx);
}

inline void Accessible::onSetName(const wchar_t *name, int idx) {
  _voidcall(ACCESSIBLE_ONSETNAME, name, idx);
}

inline OSWINDOWHANDLE Accessible::getOSWnd() {
  return _call(ACCESSIBLE_GETOSWND, (OSWINDOWHANDLE)NULL);
}

inline void Accessible::onStateChange(int idx/* =-1 */) {
  _voidcall(ACCESSIBLE_ONSTATECHANGE, idx);
}

inline int Accessible::flattenContent(OSWINDOWHANDLE *w) {
  return _call(ACCESSIBLE_FLATTENCONTENT, 0, w);
}

class AccessibleI : public Accessible {
  public:
    AccessibleI() {}
    virtual ~AccessibleI() {}

    virtual IAccessible *getIAccessible()=0;
#ifdef _WIN32
    virtual HRESULT getOSHandle(int p)=0;
#endif
    virtual void release()=0;
    virtual void addRef()=0;
    virtual int getNumRefs()=0;
    virtual void onGetFocus(int idx=-1)=0;
    virtual void onSetName(const wchar_t *name, int idx)=0;
    virtual OSWINDOWHANDLE getOSWnd()=0;
    virtual void onStateChange(int idx=-1)=0;
    virtual int flattenContent(OSWINDOWHANDLE *w)=0;

  protected:
    RECVS_DISPATCH;
};

#endif
