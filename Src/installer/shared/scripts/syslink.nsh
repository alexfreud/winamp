!ifndef NULLSOFT_NX_SYSLINK_NSIS_HEADER
!define NULLSOFT_NX_SYSLINK_NSIS_HEADER

!define WC_LINK    "SysLink"

!define INVALID_LINK_INDEX  -1
!define MAX_LINKID_TEXT     48
!define L_MAX_URL_LENGTH    2083

!define LWS_TRANSPARENT     0x0001
!define LWS_IGNORERETURN    0x0002
!define LWS_NOPREFIX        0x0004
!define LWS_USEVISUALSTYLE  0x0008
!define LWS_USECUSTOMTEXT   0x0010
!define LWS_RIGHT           0x0020

!define LIF_ITEMINDEX    0x00000001
!define LIF_STATE        0x00000002
!define LIF_ITEMID       0x00000004
!define LIF_URL          0x00000008

!define LIS_FOCUSED         0x00000001
!define LIS_ENABLED         0x00000002
!define LIS_VISITED         0x00000004
!define LIS_HOTTRACK        0x00000008
!define LIS_DEFAULTCOLORS   0x00000010

;typedef struct tagLITEM {
;  UINT  mask;
;  int   iLink;
;  UINT  state;
;  UINT  stateMask;
;  WCHAR szID[MAX_LINKID_TEXT];
;  WCHAR szUrl[L_MAX_URL_LENGTH];
;} LITEM, *PLITEM;
!define stLITEM '(i, i, i, i, &w${MAX_LINKID_TEXT}, &w${L_MAX_URL_LENGTH}) i'

;typedef struct tagNMLINK
;{
;    NMHDR       hdr;
;    LITEM     item ;
;} NMLINK,  *PNMLINK;
!define stNMLINK '(i, i, i, i, i, i, i, &w${MAX_LINKID_TEXT}, &w${L_MAX_URL_LENGTH}) i'

;  SysLink notifications
;  NM_CLICK    ;wParam: control ID, lParam: PNMLINK, ret: ignored.

;  LinkWindow messages
!define /math LM_HITTEST	${WM_USER} + 0x300  ;wParam: n/a, lparam: PLHITTESTINFO, ret: BOOL
!define /math LM_GETIDEALHEIGHT	${WM_USER} + 0x301 ;wParam: cxMaxWidth, lparam: n/a, ret: cy
!define /math LM_SETITEM	${WM_USER} + 0x302 ;wParam: n/a, lparam: LITEM*, ret: BOOL
!define /math LM_GETITEM	${WM_USER} + 0x303 ;wParam: n/a, lparam: LITEM*, ret: BOOL
!define LM_GETIDEALSIZE	${LM_GETIDEALHEIGHT} ;wParam: cxMaxWidth, lparam: SIZE*, ret: cy

!endif ; defined(NULLSOFT_NX_SYSLINK_NSIS_HEADER)