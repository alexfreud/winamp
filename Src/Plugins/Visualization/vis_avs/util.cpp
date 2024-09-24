/*
  LICENSE
  -------
Copyright 2005 Nullsoft, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer. 

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution. 

  * Neither the name of Nullsoft nor the names of its contributors may be used to 
    endorse or promote products derived from this software without specific prior written permission. 
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include <windows.h>
#include <commctrl.h>
#include "r_defs.h"
#include "resource.h"
#include "../Agave/Language/api_language.h"
#if 0//syntax highlighting
#include "compiler.h"
#include "richedit.h"
#endif

char* GetTextResource(UINT id){
	void* data = 0;
	HRSRC rsrc = FindResource(WASABI_API_LNG_HINST,MAKEINTRESOURCE(id),"TEXT");
	if(rsrc){
	HGLOBAL resourceHandle = LoadResource(WASABI_API_LNG_HINST,rsrc);
		data = LockResource(resourceHandle);
	}
	return (char*)data;
}

void loadComboBox(HWND dlg, char *ext, char *selectedName) 
{
	char path[MAX_PATH];
	int a;
	HANDLE ff;
	WIN32_FIND_DATA fd;

	wsprintf(path,"%s\\%s",g_path,ext);

	ff=FindFirstFile(path, &fd);
	if (ff == INVALID_HANDLE_VALUE) return;

	do
		{
		SendMessage(dlg, CB_ADDSTRING, 0, (LPARAM)(fd.cFileName));
		} while (FindNextFile(ff, &fd));
	FindClose(ff);

	a = SendMessage(dlg, CB_FINDSTRINGEXACT, 0, (LPARAM)(selectedName));
	if (a != CB_ERR) SendMessage(dlg, CB_SETCURSEL, (WPARAM) a, 0);
}

void GR_SelectColor(HWND hwnd, int *a)
{
	static COLORREF custcolors[16];
	CHOOSECOLOR cs;
	cs.lStructSize = sizeof(cs);
	cs.hwndOwner = hwnd;
	cs.hInstance = 0;
	cs.rgbResult=((*a>>16)&0xff)|(*a&0xff00)|((*a<<16)&0xff0000);
	cs.lpCustColors = custcolors;
	cs.Flags = CC_RGBINIT|CC_FULLOPEN;
	if (ChooseColor(&cs))
	{
		*a = ((cs.rgbResult>>16)&0xff)|(cs.rgbResult&0xff00)|((cs.rgbResult<<16)&0xff0000);
	}
}

void GR_DrawColoredButton(DRAWITEMSTRUCT *di, COLORREF color)
{
	color = ((color>>16)&0xff)|(color&0xff00)|((color<<16)&0xff0000);

	char wt[123];
	HPEN hPen,hOldPen;
	HBRUSH hBrush,hOldBrush;
	hPen = (HPEN)GetStockObject(BLACK_PEN);
	LOGBRUSH lb={BS_SOLID,color,0};
	hBrush = CreateBrushIndirect(&lb);
	hOldPen=(HPEN)SelectObject(di->hDC,hPen);
	hOldBrush=(HBRUSH)SelectObject(di->hDC,hBrush);

	Rectangle(di->hDC,di->rcItem.left,di->rcItem.top,di->rcItem.right,di->rcItem.bottom);

	GetWindowText(di->hwndItem,wt,sizeof(wt));
	SetBkColor(di->hDC,color);
	SetTextColor(di->hDC,~color & 0xffffff);
	DrawText(di->hDC,wt,-1,&di->rcItem,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
	
	DeleteObject(hBrush);
	SelectObject(di->hDC,hOldPen);
	SelectObject(di->hDC,hOldBrush);
}

static int m_help_lastpage=4;
static char *m_localtext;
static void _dosetsel(HWND hwndDlg)
{
  HWND tabwnd=GetDlgItem(hwndDlg,IDC_TAB1);
  int sel=TabCtrl_GetCurSel(tabwnd);
  char *text="";

  m_help_lastpage=sel;

  if (sel == 0)

/*
    text="Many AVS effects allow you to write simple expressions to control\r\n"
         "visualization. Here is a brief summary of how to write AVS code.\r\n"
         "\r\n"
         "Many aspects of AVS code are similar to C (including comments). "
         "\r\n"
         "You can create new variables just by using them, and you can read\r\n"
         "and write predefined variables (of which each effect has its own)\r\n"
         "to interact with the effect. Note that variables are all floating\r\n"
         "point numbers (no strings), and the maximum length of a variable's\r\n"
         "name is 8 characters (anything longer will be ignored.\r\n"
         "\r\n"
         "So, to create a variable, you can simply use it, for example:\r\n"
         "  x = 5;\r\n"
         "\r\n"
         "You can also use a variety of operators and math functions to\r\n"
         "modify variables, see the Operators and Functions tabs above.\r\n"
         "\r\n"
         "Code can include C and C++ style comments:\r\n"
         "\r\n"
         "You can combine operators and functions into expressions, such\r\n"
         "as:\r\n"
         "  x = 5 * cos(y) / 32.0; // this does some leetness right here\r\n"
         "\r\n"
         "You can use multiple expressions by seperating them with one or\r\n"
         "more semicolons, for example:\r\n"
         "  x = x * 17.0; x = x / 5; y = pow(x,3.0);\r\n"
         "\r\n"
         "It is worth noting that extra whitespace (spaces, newlines) is\r\n"
         "ignored, so if you need to space things out for clarity, you can.\r\n"
      ;
  else if (sel == 1)
    text="The following operators are available:\r\n"
      "=\r\n"
      "  assigns a value to a variable. \r\n"
      "  example:  var=5;\r\n"
      "\r\n"
      "+\r\n"
      "  adds two values, returns the sum. \r\n"
      "  example:  var=5+var2;\r\n"
      "\r\n"
      "-\r\n"
      "  subtracts two values, returns the difference. \r\n"
      "  example:  var=5-var2;\r\n"
      "\r\n"
      "*\r\n"
      "  multiplies two values, returns the product. \r\n"
      "  example:  var=5*var2;\r\n"
      "\r\n"
      "/\r\n"
      "  divides two values, returns the quotient. \r\n"
      "  example:  var=5/var2;\r\n"
      "\r\n"
      "%\r\n"
      "  converts two values to integer, performs division, returns remainder\r\n"
      "  example:  var=var2%5;\r\n"
      "\r\n"
      "|\r\n"
      "  converts two values to integer, returns bitwise OR of both values\r\n"
      "  example:  var=var2|31;\r\n"
      "\r\n"
      "&\r\n"
      "  converts two values to integer, returns bitwise AND of both values\r\n"
      "  example:  var=var2&31;\r\n"
      "\r\n"
    ;
  else if (sel == 2)
    text="Functions available from code:\r\n"
      "abs(value)\r\n"
      "  = returns the absolute value of 'value'\r\n"
      "\r\n"
      "sin(value)\r\n"
      "  = returns the sine of the radian angle 'value'\r\n"
      "\r\n"
      "cos(value)\r\n"
      "  = returns the cosine of the radian angle 'value'\r\n"
      "\r\n"
      "tan(value)\r\n"
      "  = returns the tangent of the radian angle 'value'\r\n"
      "\r\n"
      "asin(value)\r\n"
      "  = returns the arcsine (in radians) of 'value'\r\n"
      "\r\n"
      "acos(value)\r\n"
      "  = returns the arccosine (in radians) of 'value'\r\n"
      "\r\n"
      "atan(value)\r\n"
      "  = returns the arctangent (in radians) of 'value'\r\n"
      "\r\n"
      "atan2(value,value2)\r\n"
      "  = returns the arctangent (in radians) of 'value'/'value2'\r\n"
      "\r\n"
      "sqr(value)\r\n"
      "  = returns the square of 'value'\r\n"
      "\r\n"
      "sqrt(value)\r\n"
      "  = returns the square root of 'value'\r\n"
      "\r\n"
      "invsqrt(value)\r\n"
      "  = returns the reciprocal of the  square root of 'value' (1/sqrt(value))\r\n"
      "    (uses a fast approximation, may not always = 1/sqrt(value) :)\r\n"
      "\r\n"
      "pow(value,value2)\r\n"
      "  = returns 'value' to the power of 'value2'\r\n"
      "\r\n"
      "exp(value)\r\n"
      "  = returns e to the power of 'value'\r\n"
      "\r\n"
      "log(value)\r\n"
      "  = returns the log in base e of 'value'\r\n"
      "\r\n"
      "log10(value)\r\n"
      "  = returns the log in base 10 of 'value'\r\n"
      "\r\n"
      "floor(value)\r\n"
      "  = returns the largest integer less than or equal to 'value'\r\n"
      "\r\n"
      "ceil(value)\r\n"
      "  = returns the smallest integer greater than or equal to 'value'\r\n"
      "\r\n"
			"sign(value)\r\n"
      "  = returns the sign of 'value' (-1.0 or 1.0, or 0.0 or -0.0 for 0.0 or -0.0)\r\n"
      "\r\n"
			"min(value,value2)\r\n"
      "  = returns the smallest of 'value' and 'value2'\r\n"
      "\r\n"
			"max(var,var2)\r\n"
      "  = returns the greatest of 'value' and 'value2'\r\n"
      "\r\n"
			"sigmoid(value,value2)\r\n"
      "  = returns sigmoid function value of x='value' ('value2'=constraint)\r\n"
      "\r\n"
			"rand(value)\r\n"
      "  = returns a random integer between 0 and 'value'\r\n"
      "\r\n"
			"band(value,value2)\r\n"
      "  = returns a boolean AND of 'value' and 'value2'\r\n"
      "\r\n"
			"bor(value,value2)\r\n"
      "  = returns a boolean OR of 'value' and 'value2'\r\n"
      "\r\n"
			"bnot(value)\r\n"
      "  = returns a boolean NOT of 'value'\r\n"
      "\r\n"
			"if(condition,valtrue,valfalse)\r\n"
      "  = returns 'valtrue' if 'condition' is nonzero, returns 'valfalse' otherwise.\r\n"
      "    new in AVS 2.8+: only one of valtrue/valfalse is evaluated, depending on condition\r\n"
      "\r\n"
			"assign(dest, source)\r\n"
      "  = if 'dest' is a variable, assigns the value of 'source' to it. returns the value of 'source'.\r\n"
      "    a little trick: assign(if(v,a,b),1.0); is like if V is true, a=1.0, otherwise b=1.0. :)\r\n"
      "\r\n"
			"exec2(parm1, parm2)\r\n"
      "  = evaluates parm1, then parm2, and returns the value of parm2.\r\n"
      "\r\n"
			"equal(value,value2)\r\n"
      "  = returns 1.0 if 'value' is equal to 'value2', otherwise returns 0.0\r\n"
      "\r\n"
			"above(value,value2)\r\n"
      "  = returns 1.0 if 'value' is greater than 'value2', otherwise returns 0.0\r\n"
      "\r\n"
			"below(value,value2)\r\n"
      "  = returns 1.0 if 'value' is less than 'value2', otherwise returns 0.0\r\n"
      "\r\n"
			"getosc(band,width,channel)\r\n"
      "  = returns waveform data centered at 'band', (0..1), sampled 'width' (0..1) wide.\r\n"
      "    'channel' can be: 0=center, 1=left, 2=right. return value is (-1..1)\r\n"    
      "\r\n"
			"getspec(band,width,channel)\r\n"
      "  = returns spectrum data centered at 'band', (0..1), sampled 'width' (0..1) wide.\r\n"
      "    'channel' can be: 0=center, 1=left, 2=right. return value is (0..1)\r\n"    
      "\r\n"
      "gettime(start_time)\r\n"
      "  = returns time in seconds since start_time (start_time can be 0 for time since boot)\r\n"
      "    (start_time can be -1.0 for current play time in seconds\r\n"
      "    (start_time can be -2.0 for current play length in seconds\r\n"
      "\r\n"
      "getkbmouse(which_parm)\r\n"
      "  = returns information about the location and state of the keyboard or mouse\r\n"
      "    which_parm = 1: mouse X position (-1..1 is onscreen)\r\n"
      "    which_parm = 2: mouse Y position (-1..1 is onscreen)\r\n"
      "    which_parm = 3: mouse left button state (0 up, 1 down)\r\n"
      "    which_parm = 4: mouse right button state (0 up, 1 down)\r\n"
      "    which_parm = 5: mouse middle button state (0 up, 1 down)\r\n"
      "    which_parm > 5: (GetAsyncKeyState(which_parm)&0x8000)?1:0\r\n"
      "\r\n"
      "megabuf(index)\r\n"
      "  = can be used to get or set an item from the 1 million item temp buffer\r\n"
      "    to get, use:   val=megabuf(index);\r\n"
      "    to set, use:   assign(megabuf(index),val);\r\n"
      "gmegabuf(index)\r\n"
      "  = can be used to get or set an item from the global 1 million item buffer\r\n"
      "    to get, use:   val=gmegabuf(index);\r\n"
      "    to set, use:   assign(gmegabuf(index),val);\r\n"
      "\r\n"
      "loop(count, statement)\r\n"
      "  = executes <statement> <count> times. count is evaluated once and clamped\r\n"
      "    to 0..4096. best used with exec2() and exec3() and assign(). Note that\r\n"
      "    the return value of loop() is undefined and should not be used.\r\n"
      "\r\n"
    ;
  else if (sel == 3)
    text="Constants\r\n"
          "   '$PI' can be used in place of '3.14159'\r\n"
          "   '$E' can be used in place of '2.71828'\r\n"
          "   '$PHI' can be used in place of '1.618033'\r\n"
          "   Numbers can be specified as integers or as floating point\r\n"
          "     (i.e. '5' or '5.0' or '5.00001')\r\n"

    ;
*/

    text=GetTextResource(IDR_HELP_1);
	else if (sel == 1)
		text=GetTextResource(IDR_HELP_2);
	else if (sel == 2)
		text=GetTextResource(IDR_HELP_3);
	else if (sel == 3)
		text=GetTextResource(IDR_HELP_4);
	else if (sel == 4 && m_localtext)
		text=m_localtext;

  SetDlgItemText(hwndDlg,IDC_EDIT1,text);
}

static BOOL CALLBACK evalHelpDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      {
        TCITEM item;
        HWND tabwnd=GetDlgItem(hwndDlg,IDC_TAB1);
        item.mask=TCIF_TEXT;
        item.pszText=WASABI_API_LNGSTRING(IDS_GENERAL);
        TabCtrl_InsertItem(tabwnd,0,&item);
        item.pszText=WASABI_API_LNGSTRING(IDS_OPERATORS);
        TabCtrl_InsertItem(tabwnd,1,&item);
        item.pszText=WASABI_API_LNGSTRING(IDS_FUNCTIONS);
        TabCtrl_InsertItem(tabwnd,2,&item);
        item.pszText=WASABI_API_LNGSTRING(IDS_CONSTANTS);
        TabCtrl_InsertItem(tabwnd,3,&item);
        // fucko: context specific stuff
        m_localtext=0;
        if (lParam)
        {
          item.pszText=(char *)lParam;
          m_localtext=item.pszText + strlen(item.pszText)+1;
          TabCtrl_InsertItem(tabwnd,4,&item);
        }
        else if (m_help_lastpage > 3) m_help_lastpage=0;

        TabCtrl_SetCurSel(tabwnd,m_help_lastpage);
        _dosetsel(hwndDlg);
      }
    return 0;
    case WM_COMMAND:
      if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        EndDialog(hwndDlg,1);
    return 0;
    case WM_NOTIFY:
      {
        LPNMHDR p=(LPNMHDR) lParam;
        if (p->idFrom == IDC_TAB1 && p->code == TCN_SELCHANGE) _dosetsel(hwndDlg);
      }
    return 0;
  }
  return 0;
}

void compilerfunctionlist(HWND hwndDlg, char *localinfo)
{
  WASABI_API_DIALOGBOXPARAM(IDD_EVAL_HELP,hwndDlg,evalHelpDlgProc, (LONG)localinfo);
}



#if 0//syntax highlighting
// If you include richedit boxes, you need to load the richlib at the beginning:
// HANDLE hRichLib = LoadLibrary("RICHED32.DLL");
// When quitting:
// FreeLibrary(hRichLib);


#define M_WORD 1
#define M_NUM  2
#define M_COMM 3

#define is_alpha(a)  ((((a) >= 'A') && ((a) <= 'Z')) || (((a) >= 'a') && ((a) <= 'z')))
#define is_num(a)  (((a) >= '0') && ((a) <= '9'))

#define is_op(a) (((a) == '=') || ((a) == '+') || ((a) == '-') || ((a) == '*') || ((a) == '/') || ((a) == '%'))


// Colors for bracket pairs (loops around, ugly colors for now), can be any number of colors
static int bcol[] = { RGB(192, 0, 0), RGB(64, 128, 128), RGB(128, 0, 255), RGB(128, 128, 255) };

#define COLOR_COMMENT RGB(0, 128, 0)
#define COLOR_FUNC RGB(0, 0, 192)
#define COLOR_VAR RGB(96, 96, 96)
#define COLOR_OP RGB(0, 0, 0)
#define COLOR_NUMBER RGB(0, 0, 128)

// Actual syntax highlighting
// 'hwnd' is a richedit box, 'data' is the code of 'size' characters
void doAVSEvalHighLight(HWND hwndDlg, UINT sub, char *data) {
  int size=strlen(data);
  HWND hwnd=GetDlgItem(hwndDlg,sub);
	CHARRANGE cr, cro;
	
	CHARFORMAT cf;
	cf.cbSize    = sizeof(CHARFORMAT);
	cf.dwMask    = CFM_COLOR;
	cf.dwEffects = 0;

	SendMessage(hwnd, WM_SETREDRAW, false, 0);
	SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&cro);
	int mode = 0;
	int pos = 0;
	int brackets = 0;
	for (int i = 0; i < size; ++i) {
		if (mode == M_COMM) {
			// We're inside a comment, check for its end
			if ((data[i] == ';') || ((data[i] == '*') && ((i+1) < size) && (data[++i] == '/'))) {
				mode = 0;
				cf.crTextColor = COLOR_COMMENT;
				cr.cpMin = pos;
				cr.cpMax = i+1;
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
				SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			}
		} else if (is_alpha(data[i]) || ((mode == M_WORD) && is_num(data[i]))) {
			// Letter tokens (func calls, vars)
			if (mode != M_WORD) {
				// Enter word-mode if we haven't yet
				mode = M_WORD;
				pos = i;
			}
			// Stop condition
			bool valid = (i != (size-1)); // Check if this isn't the last character
			if (valid) {
				valid = is_num(data[i+1]) || is_alpha(data[i+1]);
			}
			if (!valid) {
				// We have reached the end of this word
				cr.cpMin = pos;
				cr.cpMax = i+1;
				// Check if its a function
				bool func = false;
				for (int j = 0; j < (sizeof(fnTable) / sizeof(fnTable[0])); ++j) {
					if ((i - pos + 1) == (signed)strlen(fnTable[j].name)) {
						if (strnicmp(fnTable[j].name, &data[pos], strlen(fnTable[j].name)) == 0) {
							func = true;
							break;
						}
					}
				}
				cf.crTextColor = func ? COLOR_FUNC : COLOR_VAR;
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
				SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			}
		} else if (is_op(data[i])) {
			// This is an operator
			mode = 0;
			cf.crTextColor = COLOR_OP;
			cr.cpMin = i;
			cr.cpMax = i+1;
			SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
			SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		} else if ((data[i] & 0x80) || ((data[i] == '/') && ((i+1) < size) && (data[++i] == '*'))) {
			// This is a comment marker, enter comment mode
			mode = M_COMM;
			pos = i;
		} else if ((data[i] == '(') || (data[i] == ')') || (data[i] == ',')) {
			// Reached brackets: we count them and color them in pairs
			mode = 0;
			if (data[i] == '(') ++brackets;
			cf.crTextColor = bcol[brackets % (sizeof(bcol) / sizeof(bcol[0]))];
			if (data[i] == ')') --brackets;
			cr.cpMin = i;
			cr.cpMax = i+1;
			SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
			SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		} else if (is_num(data[i]) || (data[i] == '.')) {
			// constants
			if (mode != M_NUM) {
				pos = i;
				mode = M_NUM;
			}
			// Stop condition
			bool valid = (i != (size-1));
			if (valid) {
				valid = is_num(data[i+1]) || (data[i+1] == '.');
			}
			if (!valid) {
				cf.crTextColor = COLOR_NUMBER;
				cr.cpMin = pos;
				cr.cpMax = i+1;
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
				SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			}
		} else if (data[i] == ';') {
			// Reset bracket count and mode for every statement.
			mode = 0;
			brackets = 0;
		}
	}
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cro);
	SendMessage(hwnd, WM_SETREDRAW, true, 0);
	InvalidateRect(hwnd, 0, true);
}
#endif