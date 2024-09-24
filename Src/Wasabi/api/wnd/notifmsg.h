#ifndef _NOTIFMSG_H
#define _NOTIFMSG_H

//!## Messages for BaseWnd::childNotify

// these are in their own header file to keep from having to muck with
// basewnd.h all the time to add a new global message

namespace ChildNotify {
enum {
//
// Common messages any control may need to send
  NOP = 0, // not used.  do not use.

  // When child window is in its destructor
  DELETED = 100, // no params

  // Focus stuff
  GOTFOCUS = 200,	// no params
  KILLFOCUS = 210, // no params
  NAMECHANGED = 220, // no params

  // for transient child windows
  RETURN_CODE = 300, // param1 = the code
	// NOTE: Currently only used by editwnd.cpp, but open for custom use by developers

  // child wants parent to hide/show it
  HIDEYHIDEY = 400, // no params
  UNHIDEYHIDEY = 410, // no params
  
// class-specific stuff

  // ButtonWnd, buttwnd.h
  BUTTON_LEFTPUSH = 10000, // no params
  BUTTON_RIGHTPUSH = 10010, // no params
  BUTTON_LEFTDOUBLECLICK = 10020, // no params
  BUTTON_RIGHTDOUBLECLICK = 10030, // no params

  CLICKWND_LEFTDOWN = 10100, // x/y
  CLICKWND_LEFTUP = 10101, // x/y
  CLICKWND_RIGHTDOWN = 10102, // x/y
  CLICKWND_RIGHTUP = 10103, // x/y

  // EditWnd, editwnd.h
  EDITWND_DATA_MODIFIED = 11000, // user edit - no params
  EDITWND_DATA_MODIFIED_ONIDLE = 11005, // user edit - no params (on idle)
  EDITWND_CANCEL_PRESSED = 11010, // esc pressed - no params
  EDITWND_ENTER_PRESSED = 11020, // enter pressed	- no params
  EDITWND_KEY_PRESSED = 11030, // any key pressed - param1 = the key

  // DropList, droplist.h
	DROPLIST_SEL_CHANGED = 12000, // param1 = new pos

  // ListWnd, listwnd.h
  LISTWND_SELCHANGED = 13000, // sent on any change - no params
  LISTWND_ITEMSELCHANGED = 13010, // sent for each item - param1 = the item index, param2 = its selected state
  LISTWND_DBLCLK = 13100, // param1 = the item index
  LISTWND_POPUPMENU = 13200, // param1 = the x coord, param2 = the y coord

  // FrameWnd, framewnd.h
  FRAMEWND_QUERY_SLIDE_MODE = 14000, // no params - return slide mode
  FRAMEWND_SET_SLIDE_MODE = 14010, // param1 = the new slide mode
  FRAMEWND_WINDOWSHADE_CAPABLE = 14020, // no params - return width of shade
  FRAMEWND_WINDOWSHADE_ENABLE = 14030, // param1 = new enabled state
// Cut?  this is unused.
//  FRAMEWND_WINDOWSHADE_DISABLE = 14040,
  FRAMEWND_SETTITLEWIDTH = 14050,	// param1 = the pullbar position

	// ServiceWnd, servicewnd.h
  SVCWND_LBUTTONDOWN = 15000,	// param1 = the x coord, param2 = the y coord
  SVCWND_RBUTTONDOWN = 15010,	// param1 = the x coord, param2 = the y coord
  SVCWND_LBUTTONUP = 15020,	// param1 = the x coord, param2 = the y coord
  SVCWND_RBUTTONUP = 15030,	// param1 = the x coord, param2 = the y coord
  SVCWND_LBUTTONDBLCLK = 15040,	// param1 = the x coord, param2 = the y coord
  SVCWND_RBUTTONDBLCLK = 15050,	// param1 = the x coord, param2 = the y coord
  SVCWND_MOUSEMOVE = 15060,	// param1 = the x coord, param2 = the y coord

  // Slider, slider.h
  SLIDER_INTERIM_POSITION = 16000, // param1 = the slider position
  SLIDER_FINAL_POSITION = 16010, // param1 = the slider position

  // BucketItem, bucketitem.h
  COMPONENTBUCKET_SETTEXT = 17000, // param1 = pointer to the text

	// CheckWnd, checkwnd.h
  CHECKWND_CLICK = 18000, // param1 = the new checkstate

  // ScrollBar, scrollbar.h
  SCROLLBAR_SETPOSITION = 19000, // param1 = the new position
  SCROLLBAR_SETFINALPOSITION = 19010, // called when scrolling is completed - no params

  // RadioGroup, radiogroup.h
  RADIOGROUP_STATECHANGE =  20000, // param1 = the object selected, param2 = the object deselected

  // grouptogglebutton
  GROUPCLICKTGBUTTON_TOGGLE = 20100, // when the button is toggled, param1 = status
  GROUPCLICKTGBUTTON_CLICKED = 20101, // when the button is clicked regardless of toggling

  // misc
  AUTOWHCHANGED = 20200, // sent by an object whose resource's auto w/h has changed (ie Text changed text, Layer changed image, etc)
  GROUPRELOAD = 20300, // sent by abstractwndholder when its content is reloaded

  // popup
  POPUP_SUBMENUCLOSE = 21000, // sent by a submenu to its parent popup menu when it should close (esc/left key pressed)
};

};

#endif
