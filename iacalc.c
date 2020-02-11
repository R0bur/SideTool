/*====================================*/
/* ����������� ��� ������ ����������. */
/*====================================*/
#include <windows.h>
#include <windowsx.h>
#include "iacalc.h"
#include "iadisp.h"
#include "iaarithm.h"
#include "iastring.h"
#include "debug.h"
/*----------------------------------*/
/* ���������� ������� ������������. */
/*----------------------------------*/
static LRESULT WMCtlColorStatic (HWND hWnd, HDC hDC, HWND hCtl);
static LRESULT WMCreate (HWND hWnd, LPARAM lParam);
static LRESULT WMChar (HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT WMContextMenu (HWND hWnd, int x, int y);
static LRESULT WMCommand (HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT WMPaint (HWND hWnd);
static void CreateKeyboard (HINSTANCE hInstance, HWND hWnd, int cw, int ch);
static void InitKey (struct SKey* key, int row, int col, enum EKeySize size, LPCTSTR title);
static void InitKeyboard (void);
static void ClipboardCopy ();
static int ClipboardPasteTextToNumber (LPCTSTR txt, LPSTR buf, int sz);
static void ClipboardPaste ();
static void HandleKey (HWND hWnd, enum EKeys key);
static void InitCalculator (LPCTSTR zero, BOOL mc);
static enum EOpRes BiOperation (LPTSTR a, LPCTSTR b, enum EKeys op);
static enum EModes MemoryAction (enum EKeys key);
static void MemoryEnable (BOOL en);
static void CalculatorError (BOOL er);
static void DebugCalculatorState (void);
/*-------------------------------*/
/* ����� ��������� ������������. */
/*-------------------------------*/
static HBRUSH hbrBackground = NULL;
static HBRUSH hbrDisplay = NULL;
static DWORD dwDisplayBg = 0;
static DWORD dwDisplayFg = 0;
/*---------------------------------------*/
/* �������� ������� ������ ������������. */
/*---------------------------------------*/
static enum EModes {
	NUMB,	/* ���� ����� */
	OPER,	/* ���� �������� */
	ERR,	/* ������ */
	CALC,	/* ���������� (�� ������� "=") */
	UNOP	/* ������� �������� */
};
static enum EModes SwitchCalculatorMode (enum EModes m1, enum EModes m2);
/*-------------------------------*/
/* �������� ������ ������������. */
/*-------------------------------*/
#define IDKEYS 100
static enum EKeys {
	KEY0, KEY1, KEY2, KEY3, KEY4, KEY5, KEY6, KEY7, KEY8, KEY9, KEYDP,
	KEYEQ, KEYAD, KEYSB, KEYML, KEYDV, /* KEYSQ, KEYPR, */ KEYSG,
	KEYON, KEYCE, KEYBS, KEYMC, KEYMR, KEYMS, KEYMA, 
	NKEYS,
	KEYST, KEYQT, KEYCC, KEYCV
};
/*------------------------------*/
/* ������� ������ ������������. */
/*------------------------------*/
static enum EKeySize {SZNORM, SZDBLW, SZDBLH, SZHALFH};
/*----------------------------------*/
/* �������� � ������� ������������. */
/*----------------------------------*/
static struct SKey {
	int row, col;		/* ������������ ������� �� ���������� ������������ */
	enum EKeySize size;	/* ������ ������� */
	LPCTSTR title;		/* ������� �� ������� */
};
static struct SKey keyboard[NKEYS];
static WNDPROC lpfnOldKeysWndProc;	/* ������������ ������� ������� ������ */
/*-------------------------------------------------------------*/
/* ��������� ������������.                                     */
/* �������� CAPACITY ���������� � ������������ ����� iadisp.h. */
/*-------------------------------------------------------------*/
static struct {
	enum EModes mode;	/* ����� ������ */
	TCHAR regx[CAPACITY];	/* ������� X (���������) */
	TCHAR regy[CAPACITY];	/* ������� Y (������ �������) */
	TCHAR regc[CAPACITY];	/* ��������� */
	TCHAR regm[CAPACITY];	/* ������ */
	enum EKeys op;		/* ��������� �������� */
	int cm;			/* ����� ���������� � ���������� */
} calculator;
static HWND hWndCalc;		/* ����������� ���� ������������ */
static HWND hWndDisp;		/* ����������� ���� ���������� */
static RECT rcDisp;		/* ������� ���� ���������� � ���� ������������ */
/*========================================================*/
/* ��������� ����� ��� ���������� ���� ���� ������������. */
/* �������: ����������� �����.                            */
/*========================================================*/
HBRUSH IACalcHbrBackground (void)
{
	if (!hbrBackground)
		hbrBackground = GetStockObject (WHITE_BRUSH);
	return hbrBackground;
}
/*======================================*/
/* ��������� �� ��������� ������������. */
/* �����: act - ������� ����������.     */
/*======================================*/
void IACalcActive (int act)
{
	dwDisplayFg = act? RGB (0, 0, 0): RGB (127, 127, 127);
	RedrawWindow (hWndDisp, NULL, NULL, RDW_INVALIDATE);
}
/*======================================*/
/* ������� ������� ������ ������������. */
/*======================================*/
static LRESULT CALLBACK KeysWindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lres;
	if (uMsg == WM_CHAR)
		lres = WMChar (hWnd, wParam, lParam);
	else
		lres = CallWindowProc (lpfnOldKeysWndProc, hWnd, uMsg, wParam, lParam);
	return lres;
}
/*==============================================*/
/* ������� ������� ���� ������������.           */
/* �����: hWnd - ����������� ���� ������������, */
/*        uMsg - ������������� ���������,       */
/*        wParam - ������ ��������,             */
/*        lParam - ������ ��������.             */
/*==============================================*/
LRESULT CALLBACK IACalcWindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	switch (uMsg) {
		case WM_CREATE:	/* �������� ����. */
			lRes = WMCreate (hWnd, lParam);
			break;
		case WM_CHAR:	/* ������ ������� �� ����������. */
			lRes = WMChar (hWnd, wParam, lParam);
			break;
		case WM_CONTEXTMENU: /* �������� ����� ������������ ����. */
			lRes = WMContextMenu (hWnd, GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
			break;
		case WM_COMMAND: /* ��������� �������. */
			lRes = WMCommand (hWnd, wParam, lParam);
			break;
		case WM_CTLCOLORSTATIC:	/* ������������� ���� �������� ����������. */
			lRes = WMCtlColorStatic (hWnd, (HDC)wParam, (HWND)lParam);
			break; 
		/*case WM_PAINT:*/ /* ��������� ���������� ������� ����. */
			/* lRes = WMPaint (hWnd);
			break; */
		default:
			lRes = DefWindowProc (hWnd, uMsg, wParam, lParam);
	}
	return lRes;
}
/*===================================================================*/
/* ��������� ��������� WM_CTLCOLORSTATIC.                            */
/* �����: hWnd - ����������� ���� ������������,                      */
/*        hDC - �������� ���������� ����������� �������� ����������, */
/*        hCtl - ����������� �������� ����������.                    */
/* �������: ��� ���������� ��������� ��������� (��. ������������).   */
/*===================================================================*/
static LRESULT WMCtlColorStatic (HWND hWnd, HDC hDC, HWND hCtl)
{
	/*MessageBox (hWnd, TEXT ("WMCtlColorStatic"), TEXT ("iacalc.c - WMCtlColorStatic"), MB_OK);*/
	LRESULT lRes;
	if (hCtl == hWndDisp) {
		SetTextColor (hDC, dwDisplayFg);
		SetBkColor (hDC, dwDisplayBg);
		lRes = (LRESULT)hbrDisplay;
	}
	else
		lRes = 0;
	return lRes;
}
/*==============================================*/
/* ��������� ��������� WM_PAINT.                */
/* �����: hWnd - ����������� ����.              */
/* �������: ��� ���������� ��������� ���������. */
/*==============================================*/
static LRESULT WMPaint (HWND hWnd)
{
	RECT rc;
	PAINTSTRUCT ps;
	HDC hdc;
	LRESULT lRes = 0;
	if (GetUpdateRect (hWnd, &rc, TRUE)) {
		hdc = BeginPaint (hWnd, &ps);
		TextOut (hdc, 0, 0, TEXT ("Calculator"), 10);
		EndPaint (hWnd, &ps);
	}
	return lRes;
}
/*============================================================*/
/* ��������� ��������� WM_CHAR.                               */
/* �����: hWnd - ����������� ����,                            */
/*        wParam - ��� �������, ���������������� �������,     */
/*        lParam - �������������� �������� � ������� �������. */
/* �������: ��� ���������� ��������� ���������.               */
/*============================================================*/
static LRESULT WMChar (HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	static enum EKeys ekp = NKEYS;
	TCHAR buf[128];
	LRESULT lRes = 0;
	TCHAR ck;
	enum EKeys ek = NKEYS;	/* ���������� ��� ������� ������� */
	ck = (TCHAR) wParam;	/* ��� ������� ������� ������� */
	if ((TCHAR) '0' <= ck && ck <= (TCHAR) '9')
		/* ������ �������� �������. */
		ek = KEY0 + (ck - (TCHAR) '0');
	else
		switch (ck) {
			case VK_ESCAPE:		/* ������� ���������� / ������������� ������������ */
				ek = (ekp != KEYCE)? KEYCE: KEYON;
				break;
			case VK_BACK:		/* �������� ��������� ��������� ����� */
				ek = KEYBS;
				break;
			case (TCHAR)',':	/* ������� */
			case (TCHAR)'.':	/* ����� */
				ek = KEYDP;
				break;
			case (TCHAR)'-':	/* ����� */
				ek = KEYSB;
				break;
			case (TCHAR)'+':	/* ���� */
				ek = KEYAD;
				break;
			case (TCHAR)'*':	/* �������� */
				ek = KEYML;
				break;
			case (TCHAR)'/':	/* ��������� */
				ek = KEYDV;
				break;
			case (TCHAR)'`':		/* �������� ���� ����� */
			case (TCHAR)'~':
				ek = KEYSG;
				break;
			case (VK_RETURN):	/* ���� / ����� */
			case (TCHAR)'=':
				ek = KEYEQ;
				break;
			case (TCHAR)4:		/* [CTRL]+[D] - ����������� ����������� ��������� ������������ */
				ek = KEYST;
				break;
			case (TCHAR)17:		/* [CTRL]+[Q] - ���������� ������ � ������������� */
				ek = KEYQT;
				break;
			case (TCHAR)3:		/* [CTRL]+[C] - ����������� ����������� ���������� */
				ek = KEYCC;
				break;
			case (TCHAR)22:		/* [CTRL]+[V] - ������� ����� � ��������� */
				ek = KEYCV;
				break;
			default:
				ek = NKEYS;
/*				wsprintf (buf, TEXT ("WM_CHAR wParam = %u\nLOWORD(lParam) = %u\nHIWORD(lParam) = %u"), wParam, LOWORD (lParam), HIWORD (lParam));
				MessageBox (hWnd, buf, TEXT ("iacalc.c - WMChar()"), MB_OK);*/
		}
	/* ��������� ������ �� ������, ��������������� ������� �������. */
	if (ek != NKEYS)
		SetFocus (GetDlgItem (hWndCalc, IDKEYS + ek));
	/* ��������� ������� �������. */
	HandleKey (hWnd, ek);
	/* ����������� ������������ �������. */
	ekp = ek;
	return lRes;
}
/*==============================================*/
/* ��������� ��������� WM_CONTEXTMENU.          */
/* �����: hWnd - ����������� ����,              */
/*        x, y - �������������� ��������� ����. */
/* �������: ��� ���������� ��������� ���������. */
/*==============================================*/
static LRESULT WMContextMenu (HWND hWnd, int x, int y)
{
	HMENU hMenu;
	BOOL mRes;
	LANGID lang;
	int klang;
	LPCTSTR menuItems[] = {
		TEXT ("&���������� \tCtrl+C"),
		TEXT ("&Copy \tCtrl+C"),
		TEXT ("���&����� \tCtrl+V"),
		TEXT ("P&aste \tCtrl+V")
	};
	RECT rc;
	POINT pt;
	LRESULT lRes = 0;
	/* ����������� �������� ��������� ��� �������� ������ �� ���������� ������������. */
	GetWindowRect (hWndDisp, &rc);
	pt.x = x > 0? x: rc.left;
	pt.y = y > 0? y: rc.top;
	if (PtInRect (&rc, pt)) {
		/* �������� ������������ ����. */
		lang = GetUserDefaultUILanguage ();
		klang = lang == 0x0419 || lang == 0x0422 || lang == 0x0423? 0: 1;
		hMenu = CreatePopupMenu ();
		AppendMenu (hMenu, MF_STRING, IDM_CLIPBOARD_COPY, menuItems[0 + klang]);
		AppendMenu (hMenu, MF_SEPARATOR, 0, NULL);
		AppendMenu (hMenu, MF_STRING, IDM_CLIPBOARD_PASTE, menuItems[2 + klang]);
		/* ����������� ���� � ������. */
		mRes = TrackPopupMenuEx (hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, hWnd, NULL);
		/* ���������� ������ � ����. */
		DestroyMenu (hMenu);
	}
	return lRes;
}
/*==================================================*/
/* ��������� ��������� WM_COMMAND.                  */
/* �����: hWnd - ����������� ����,                  */
/*        wParam - ������ �������� ���������,       */
/*        lParam - ����������� �������� ����������. */
/* �������: ��� ���������� ��������� ���������.     */
/*==================================================*/
static LRESULT WMCommand (HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	if (lParam == 0)
		/* ��������� �� ������������ ����. */
		switch (LOWORD (wParam)) {
			case IDM_CLIPBOARD_COPY:	/* ����������� ���������� � ����� ������ */
				HandleKey (hWnd, KEYCC);
				break;
			case IDM_CLIPBOARD_PASTE:	/* ������� �� ������ ������ � ��������� */
				HandleKey (hWnd, KEYCV);
				break;
			default:
				MessageBox (hWnd, TEXT ("Unknown context menu command."), TEXT ("iacalc.c: IACalcWindowProc"), MB_OK | MB_ICONEXCLAMATION);
		}
	else if (HIWORD (wParam) == BN_CLICKED)
		/* ��������� �� ������ ������������. */
		HandleKey (hWnd, LOWORD (wParam) - IDKEYS);
	return lRes;
}
/*==============================================*/
/* ��������� ��������� WM_CREATE.               */
/* �����: hWnd - ����������� ����,              */
/*        lParam - ��������� �� CREATESTRUCT.   */
/* �������: ��� ���������� ��������� ���������. */
/*==============================================*/
static LRESULT WMCreate (HWND hWnd, LPARAM lParam)
{
	LRESULT lRes = 0;
	HINSTANCE hInstance = (HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE);
	hWndCalc = hWnd;
	InitKeyboard ();
	CreateKeyboard (hInstance, hWnd, 42, 42);
	InitCalculator (DisplayGetZero (), TRUE);
	return lRes;
}
/*==============================================*/
/* �������� ���������� ������������.            */
/* �����: hInstance - ����������� ����������,   */
/*        hWnd - ����������� ���� ������������, */
/*        cw - ������ ������� ��� �������,      */
/*        ch - ������ ������� ��� �������.      */
/*==============================================*/
static void CreateKeyboard (HINSTANCE hInstance, HWND hWnd, int cw, int ch)
{
	RECT rc;
	DWORD ws;
	int kw0, kh0, x0, y0, x, y, w, h;
	int rows, cols;
	enum EKeys key;
	HWND hWndTmp;
	/* ����������� �������� ����� ��� ����������. */
	rows = cols = 0;
	for (key = KEY0; key < NKEYS; key++) {
		if (keyboard[key].col > cols)
			cols = keyboard[key].col;
		if (keyboard[key].row > rows)
			rows = keyboard[key].row;
	}
	rows += 1;
	cols += 1;
	/* ����������� �������������� ���������� �� ������������. */
	GetClientRect (hWnd, &rc);
	x0 = rc.left + (rc.right - rc.left - cols * cw) / 2;
	y0 = rc.bottom - 4 - ch;
	/* ����� ���� ������. */
	ws = WS_CHILD | BS_PUSHBUTTON | BS_FLAT | WS_VISIBLE;
	/* ����������� �������� ������������� ��� ���������� �������� �������. */
	kw0 = cw / 2 - 1;
	kh0 = ch / 2 - 1;
	/* ������� ���� ������ ���������� ������������. */
	for (key = KEY0; key < NKEYS; key++) {
		/* ���������� ������� ������� �� ���� ������������. */
		x = x0 + keyboard[key].col * cw;
		y = y0 - keyboard[key].row * ch;
		/* ���������� ������� �������. */
		switch (keyboard[key].size) {
			case SZDBLW:	/* ������� ������� ������ */
				w = cw + 2 * kw0;
				h = 2 * kh0;
				break;
			case SZDBLH:	/* ������� ������� ������ */
				y -= ch;
				w = 2 * kw0;
				h = ch + 2 * kh0;
				break;
			case SZHALFH:	/* ������� ���������� ������ */
				y += ch / 3;
				w = 2 * kw0;
				h = kh0;
				break;
			default:	/* ������� ����������� ������� */
				w = 2 * kw0;
				h = 2 * kh0;
		}
		/* �������� ���� �������. */
		hWndTmp = CreateWindow ("BUTTON", keyboard[key].title, ws, x, y, w, h, hWnd, (HMENU)(int)(IDKEYS + key), hInstance, NULL);
		lpfnOldKeysWndProc = (WNDPROC)SetWindowLong (hWndTmp, GWL_WNDPROC, (DWORD)KeysWindowProc);
	}
	/* ���������� ����� ������������. */
	/* TODO: ������� �� ��������� ����� � ������� �������� ����������? */
	rcDisp.left = x0;
	rcDisp.top = 6;
	rcDisp.right = rcDisp.left + cols * cw;
	rcDisp.bottom = rcDisp.top + ch;
	hbrDisplay = GetStockObject (WHITE_BRUSH);
	dwDisplayBg = RGB (255, 255, 255);
	dwDisplayFg = RGB (0, 0, 0);
	hWndDisp = DisplayCreate (hWnd, &rcDisp);
}
/*===============================================*/
/* ������������ ������� ���������� ������������. */
/* �����: key - ��������� �� �������� � �������, */
/*        row - ������ ������� (����� �����),    */
/*        col - ������� ������� (����� �������), */
/*        size - ������ �������,                 */
/*        title - ������� �� �������.            */
/*===============================================*/
static void InitKey (struct SKey* key, int row, int col, enum EKeySize size, LPCTSTR title)
{
	key->row = row;
	key->col = col;
	key->size = size;
	key->title = title;
}
/*=======================================*/
/* ������������ ���������� ������������. */
/*=======================================*/
static void InitKeyboard (void)
{
	InitKey (keyboard + KEY0, 0, 0, SZDBLW, TEXT ("0"));
	InitKey (keyboard + KEYDP, 0, 2, SZNORM, TEXT ("."));
	InitKey (keyboard + KEYAD, 0, 3, SZNORM, TEXT ("+"));
	InitKey (keyboard + KEYEQ, 0, 4, SZDBLH, TEXT ("="));
	InitKey (keyboard + KEYON, 1, 0, SZNORM, TEXT ("ON/C"));
	InitKey (keyboard + KEY1, 1, 1, SZNORM, TEXT ("1"));
	InitKey (keyboard + KEY2, 1, 2, SZNORM, TEXT ("2"));
	InitKey (keyboard + KEY3, 1, 3, SZNORM, TEXT ("3"));
	InitKey (keyboard + KEYCE, 2, 0, SZNORM, TEXT ("CE"));
	InitKey (keyboard + KEY4, 2, 1, SZNORM, TEXT ("4"));
	InitKey (keyboard + KEY5, 2, 2, SZNORM, TEXT ("5"));
	InitKey (keyboard + KEY6, 2, 3, SZNORM, TEXT ("6"));
	InitKey (keyboard + KEYSB, 2, 4, SZNORM, TEXT ("-"));
	InitKey (keyboard + KEYBS, 3, 0, SZNORM, TEXT ("DEL"));
	InitKey (keyboard + KEY7, 3, 1, SZNORM, TEXT ("7"));
	InitKey (keyboard + KEY8, 3, 2, SZNORM, TEXT ("8"));
	InitKey (keyboard + KEY9, 3, 3, SZNORM, TEXT ("9"));
	InitKey (keyboard + KEYML, 3, 4, SZNORM, TEXT ("*"));
	InitKey (keyboard + KEYMC, 4, 0, SZNORM, TEXT ("MC"));
	InitKey (keyboard + KEYMR, 4, 1, SZNORM, TEXT ("MR"));
	InitKey (keyboard + KEYMS, 4, 2, SZNORM, TEXT ("M-"));
	InitKey (keyboard + KEYMA, 4, 3, SZNORM, TEXT ("M+"));
	InitKey (keyboard + KEYDV, 4, 4, SZNORM, TEXT ("/"));
/*	InitKey (keyboard + KEYSQ, 5, 2, SZHALFH, TEXT ("SQ"));
	InitKey (keyboard + KEYPR, 5, 3, SZHALFH, TEXT ("%"));*/
	InitKey (keyboard + KEYSG, 5, 4, SZHALFH, TEXT ("+/-"));
}
/*=======================================================================*/
/* ���������� � ������ ��������������� ����.                             */
/* �����: zero - ������� ��������, ������������ �� ������� ������������, */
/*        mc - ���������� ������� �������� ������.                       */
/*=======================================================================*/
static void InitCalculator (LPCTSTR zero, BOOL mc)
{
	StrCpy (calculator.regx, zero);
	StrCpy (calculator.regy, zero);
	StrCpy (calculator.regc, zero);
	if (mc) {
		StrCpy (calculator.regm, zero);
		MemoryEnable (FALSE);
	}
	calculator.op = KEYEQ;		/* ������� ��������� �������� */
	calculator.mode = NUMB;		/* ����� ����� ����� */
	calculator.cm = 0;		/* ��������� �� ������ */
}
/*===================================================*/
/* ���������� �������� �������� b <= a op b.         */
/* �����: b - ������ ������� � ����� ��� ����������, */
/*        a - ������ �������,                        */
/*        op - ��� ��������.                         */
/* �������: ��������� ���������� ��������.           */
/*===================================================*/
static enum EOpRes BiOperation (LPTSTR b, LPCTSTR a, enum EKeys op)
{
	enum EOpRes opr;
	switch (op) {
		case KEYAD:	/* �������� */
			opr = OpAdd (b, a);
			break;
		case KEYSB:	/* ��������� */
			opr = OpSub (b, a);
			break;
		case KEYML:	/* ��������� */
			opr = OpMul (b, a);
			break;
		case KEYDV:	/* ������� */
			opr = OpDiv (b, a);
			break;
		case KEYEQ:	/* ��������� ��������, �� ��������� ���������� */
			opr = OPOK;
			break;
		default:	/* ����������� �������� */
			MessageBox (NULL, TEXT ("BiOperation (...)\nop - unknown operation"), TEXT ("DEBUG: iacalc.c"), MB_ICONEXCLAMATION | MB_OK);
			opr = OPERR;
	}
	return opr;
}
/*==========================================*/
/* ������������ ������ ������ ������������. */
/* �����: m1 - ������� ����� ������,        */
/*        m2 - ������������� ����� ������.  */
/* �������: ����� ����� ������.             */
/*==========================================*/
static enum EModes SwitchCalculatorMode (enum EModes m1, enum EModes m2)
{
	TCHAR msg[128];
	enum EModes res = m1;	/* �� ��������� ������������ ��������� */
	enum EOpRes opr;	/* ��������� ���������� �������� */
	opr = OPOK;	/* ��������������� ���������� ���������� �������� */
	if (m1 == NUMB && m2 == OPER) {
		/* �� ������ ����� ����� � ����� ������ ��������. */
		StrCpy (calculator.regx, DisplayGetValue ());
		if (!calculator.cm) {
			/* ���������� Y [op] X � ������� ���������� � X. */
			opr = BiOperation (calculator.regx, calculator.regy, calculator.op);
			/* ����������� ���������� �� ����������. */
			DisplaySetValue (calculator.regx);
		}
		else
			/* ����������� ���������� � ����������. */
			calculator.cm = 0;
		res = m2;
	}
	else if (m1 == NUMB && m2 == UNOP) {
		/* �� ������ ����� ����� � ����� ���������� ������� ��������. */
		res = m2;
	}
	else if (m1 == UNOP && m2 == OPER) {
		/* �� ������ ������� �������� � ����� ������ ��������. */
		StrCpy (calculator.regx, DisplayGetValue ());
		if (!calculator.cm) {
			/* ���������� Y [op] X � ������� ���������� � X. */
			opr = BiOperation (calculator.regx, calculator.regy, calculator.op);
			/* ����������� ���������� �� ����������. */
			DisplaySetValue (calculator.regx);
		}
		else
			/* ����������� ���������� � ����������. */
			calculator.cm = 0;
		res = m2;
	}
	else if (m1 == OPER && m2 == NUMB) {
		/* �� ������ ������ �������� � ����� ����� �����. */
		/* DebugCalculatorState (); */
		/* ��������� X � Y. */
		StrCpy (calculator.regy, calculator.regx);
		/* ������� ����������. */
		DisplayClearAll ();
		res = m2;
	}
	else if (m1 == OPER && m2 == UNOP) {
		/* �� ������ ������ �������� � ����� ������� ��������. */
		/* ��������� X � Y. */
		StrCpy (calculator.regy, calculator.regx);
		res = m2;
	}
	else if (m1 == NUMB && m2 == CALC) {
		/* �� ������ ����� ����� � ����� ���������� � ����������. */
		if (!calculator.cm) {
			/* ��������� ��������� � ������� X. */
			StrCpy (calculator.regx, DisplayGetValue ());
			/* ���������� ��������� �������� ��������. */
			StrCpy (calculator.regc, calculator.regx);
			opr = BiOperation (calculator.regc, calculator.regy, calculator.op);
			/* ����������� ���������� �������� �� ����������. */
			DisplaySetValue (calculator.regc);
			calculator.cm = 1;
		}
		else {
			/* ������ ���������� � ����������, ����������� � �������� X. */
			StrCpy (calculator.regy, DisplayGetValue ());
			StrCpy (calculator.regc, calculator.regx);
			opr = BiOperation (calculator.regc, calculator.regy, calculator.op);
			/* ����������� ���������� �������� �� ����������. */
			DisplaySetValue (calculator.regc);
		}
		res = m2;
	}
	else if (m1 == CALC && m2 == CALC) {
		/* ��������� ���������� � ����������, ����������� � �������� X. */
		StrCpy (calculator.regy, DisplayGetValue ());
		StrCpy (calculator.regc, calculator.regx);
		opr = BiOperation (calculator.regc, calculator.regy, calculator.op);
		/* ����������� ���������� �������� �� ����������. */
		DisplaySetValue (calculator.regc);
		res = m2;
	}
	else if (m1 == CALC && m2 == NUMB) {
		/* �� ������ ���������� � ���������� � ����� ����� �����. */
		/* ������ ������� ����������. */
		DisplayClearAll ();
		res = m2;		
	}
	else if (m1 == CALC && m2 == OPER) {
		/* �� ������ ���������� � ���������� � ����� ������ ��������. */
		StrCpy (calculator.regx, DisplayGetValue ());
		/* ����������� ���������� � ����������. */
		calculator.cm = 0;
		res = m2;
	}
	else if (m1 == CALC && m2 == UNOP) {
		/* �� ������ ���������� � ���������� � ����� ������� ��������. */
		res = m2;
	}
	else if (m1 == UNOP && m2 == CALC) {
		/* �� ������ ������� �������� � ����� ���������� � ����������. */
		if (calculator.cm) {
			/* ������� ������� � ����������� ���������. */
			StrCpy (calculator.regy, DisplayGetValue ());
			StrCpy (calculator.regc, calculator.regx);
			opr = BiOperation (calculator.regc, calculator.regy, calculator.op);
			/* ����������� ���������� �������� �� ����������. */
			DisplaySetValue (calculator.regc);
			res = m2;
		}
		else {
			/* ��������� ��������� � ������� X. */
			StrCpy (calculator.regx, DisplayGetValue ());
			/* ���������� ��������� �������� ��������. */
			StrCpy (calculator.regc, calculator.regx);
			opr = BiOperation (calculator.regc, calculator.regy, calculator.op);
			/* ����������� ���������� �������� �� ����������. */
			DisplaySetValue (calculator.regc);
			calculator.cm = 1;
		}
	}
	else if (m1 == UNOP && m2 == NUMB) {
		/* �� ������ ������� �������� � ����� ����� �����. */
		/* ������� ����������. */
		DisplayClearAll ();
		res = m2;
	}
	else if (m1 == ERR) {
		/* ����� ��������� ������ �������������� ��������� [CE] � [ON/C]. */
		res = ERR;	/* �������������� ��������� ������ */
	}
	else if (m1 != m2) {
		/* ������������ ���������. */
		wsprintf (msg, TEXT ("Forbidden mode switch from %d to %d"), m1, m2);
		MessageBox (NULL, msg, TEXT ("iacalc.c - SwitchCalculatorMode ()"), MB_OK | MB_ICONEXCLAMATION);
	}
	/* �������� ���������� ������ ��� ���������� ����������. */
	if (opr != OPOK) {
		CalculatorError (TRUE);
		res = ERR;
	}
	return res;
}
/*================================================================*/
/* ���������� �������� � ������� ������.                          */
/* �����: key - ��� �������, ��������� �������� � ������� ������. */
/* �������: ����� ����� ������ ������������ (UNOP ��� ERR).       */
/*================================================================*/
static enum EModes MemoryAction (enum EKeys key)
{
	enum EModes res;
	enum EOpRes opr;
	opr = OPOK;	/* ��������������� ���������� ���������� �������� */
	switch (key) {
		case KEYMA:	/* ����������� ��������, ������������� �� ����������, � ������ ������. */
		case KEYMS:	/* ��������� ��������, ������������� �� ����������, �� ������ ������. */
			/* ������� ��������, ������������� �� ����������, �� ��������������� �������. */
			StrCpy (calculator.regc, DisplayGetValue ());
			/* ���������� �������� �������� ��� ���������. */
			opr = BiOperation (calculator.regc, calculator.regm, key == KEYMA? KEYAD: key == KEYMS? KEYSB: NKEYS);
			/* ������ ���������� � ������ ������. */
			StrCpy (calculator.regm, calculator.regc);
			/* ��������� ������ ���������� ������� ������. */
			MemoryEnable (TRUE);
			break;
		case KEYMR:	/* ���������� �������� �� ������ ������ �� ���������. */
			DisplaySetValue (calculator.regm);
			break;
		case KEYMC:	/* ������� ������ ������. */
			StrCpy (calculator.regm, DisplayGetZero ());
			MemoryEnable (FALSE);
			break;
		default:
			MessageBox (NULL, "Unknown action in MemoryAction ()", "Debug: iacalc.c", MB_OK | MB_ICONEXCLAMATION);
	}
	/* �������� ���������� ������ ��� ���������� ����������. */
	if (opr != OPOK) {
		CalculatorError (TRUE);
		res = ERR;
	}
	else
		res = UNOP;
	return res;
}
/*==================================================================*/
/* ��������� ��������� ������ ������ ������������.                  */
/* �����: en - ������� ������� �������� ���������� � ������ ������. */
/*==================================================================*/
static void MemoryEnable (BOOL en)
{
	EnableWindow (GetDlgItem (hWndCalc, IDKEYS + KEYMR), en);
	EnableWindow (GetDlgItem (hWndCalc, IDKEYS + KEYMC), en);
	if (!en)
		SetFocus (hWndCalc);
}
/*==========================================*/
/* ��������� ��������� ������ ������������. */
/* �����: er - ������� ������� ������.      */
/*==========================================*/
static void CalculatorError (BOOL er)
{
	register int key;
	er = !er;
	for (key = KEY0; key < NKEYS; key++)
		if (key != KEYON && key != KEYCE && key != KEYMC && key != KEYMR)
			EnableWindow (GetDlgItem (hWndCalc, IDKEYS + key), er);
	if (!er)
		SetFocus (hWndCalc);
}
/*===============================================*/
/* ���������� ���������� �� ������� ������� "=". */
/*===============================================*/
static void HandleEqKey ()
{
	if (!calculator.cm) {
		/* ������� �������� �� ���������� � ������� X ������������. */
		StrCpy (calculator.regx, DisplayGetValue ());
		/* ���������� �������� ���������. */
		StrCpy (calculator.regc, calculator.regx);
		calculator.cm = 1;
		/* ���������� �������� � ��������� Y. */
		BiOperation (calculator.regx, calculator.regy, calculator.op);
	}
	else {
		/* ���������� �������� � ����������. */
		StrCpy (calculator.regy, DisplayGetValue ());
		StrCpy (calculator.regx, calculator.regc);
		BiOperation (calculator.regx, calculator.regy, calculator.op);
	}
	/* ����������� ���������� �� ����������. */
	DisplaySetValue (calculator.regx);
}
/*====================================================*/
/* ����������� ����������� ���������� � ����� ������. */
/*====================================================*/
static void ClipboardCopy ()
{
	TCHAR const* v;		/* ��������� �� ���������� ���������� */
	HGLOBAL hgValue;	/* ����������� ������� ���������� ������ */
	LPTSTR lpValue;		/* ��������� �� ���������� ������� ������ */
	int n;
	/* ��������� ������� � ������ ������. */
	if (OpenClipboard (NULL)) {
		/* ������� ����������� ������ ������. */
		EmptyClipboard ();
		/* ��������� �������� � ���������� ���������� */
		v = DisplayGetValue ();
		n = StrLen (v);
		hgValue = GlobalAlloc (GMEM_MOVEABLE, (n + 1) * sizeof (TCHAR));
		if (hgValue) {
			lpValue = GlobalLock (hgValue);
			memcpy (lpValue, v, n * sizeof (TCHAR));
			lpValue[n] = (TCHAR)'\0';
			GlobalUnlock (lpValue);
		}
		SetClipboardData (CF_TEXT, hgValue);
		/* ���������� ������ � ������� ������. */
		CloseClipboard ();
	}
}
/*==============================================================*/
/* ���������� ����� � ������� ������������ �� ��������� ������. */
/* �����: txt - ��������� ������,                               */
/*        buf - ����� ��� ������ �����,                         */
/*        sz  - ������ ������.                                  */
/* �������: ���������� �������� ����� � ������� ������������,   */
/*          ���������� � ����� buf, �� ������ ������� '\0'.     */
/*==============================================================*/
static int ClipboardPasteTextToNumber (LPCTSTR txt, LPSTR buf, int sz)
{
	int i, j, dp;
	int nd, mnd;	/* ������������ ���������� ���� � ����� */
	TCHAR s[CAPACITY];
	TCHAR c;
	int res;
	if (sz > 2) {
		mnd = sz - 3;	/* ����������� ����, ���������� ����������� � ������������ */
		dp = 0;	/* ������� ������� ������� ����� */
		nd = 0;	/* ������� ���������� ���� � ����� */
		i = 0;	/* ������ ��������� */
		j = 0;	/* ������ �������� */
		/* ������� ��������, �� ���������� ������� � ������� �����. */
		c = txt[i];
		while (c && !((TCHAR)'0' <= c && c <= (TCHAR)'9') && c != (TCHAR)'-')
			c = txt[++i];
		/* ����������� ����� �����. */
		if (c == (TCHAR)'-') {
			s[j++] = (TCHAR)'-';
			i++;
		}
		/* ���������� ����� ����� �����. */
		c = txt[i++];
		/* ������� ���������� ����� ����� �����. */
		while (c == (TCHAR)'0') 
			c = txt[i++];
		/* ����������� ��������� ���� ����� �����. */
		while ((TCHAR)'0' <= c && c <= (TCHAR)'9' && j < sz - 2 && nd < mnd) {
			s[j++] = c;
			c = txt[i++];
			nd++;
		}
		/* ���������� ����������� �����������. */
		if ((c == (TCHAR)'.' || c == (TCHAR)',') && j < sz - 2) {
			/* ��� ������������� ����� ����� ������� ����. */
			if (!j || j == 1 && s[0] == '-') {
				s[j++] = (TCHAR)'0';
				nd++;
			}
			/* ������� ����������� �����������. */
			s[j++] = (TCHAR)',';
			dp = 1;
		}
		else
			i--;	/* ������� ��������� � ������������� �������. */
		/* ���������� ���� ������� �����. */
		c = txt[i++];
		while ((TCHAR)'0' <= c && c <= (TCHAR)'9' && j < sz - 1 && nd < mnd) {
			s[j++] = c;
			c = txt[i++];
			nd++;
		}
		/* ������������ ���������� ����� ������� �����. */
		if (dp) {
			j--;
			while (s[j] == (TCHAR)'0')
				j--;
		}
		else
			s[j] = (TCHAR)',';
		s[++j] = (TCHAR)'\0';
		/* �������� ������� ��������� �������� � ������������ ��������� ������. */
		if (s[0] != (TCHAR)',' || s[1] != (TCHAR)'\0') {
			StrCpy (buf, s);
			res = j;
		}
		else
			res = 0;
	}
	else
		res = 0;
	return res;
}
/*================================================*/
/* ������� ����������� ������ ������ � ���������. */
/*================================================*/
static void ClipboardPaste ()
{
	TCHAR buf[CAPACITY];
	HGLOBAL hgValue;	/* ����������� ������� ���������� ������ */
	LPTSTR lpValue;		/* ��������� �� ���������� ������� ������ */
	/* ��������� ������� � ������ ������. */
	if (IsClipboardFormatAvailable (CF_TEXT) && OpenClipboard (NULL)) {
		hgValue = GetClipboardData (CF_TEXT);
		if (hgValue) {
			lpValue = GlobalLock (hgValue);
			if (lpValue) {
				if (ClipboardPasteTextToNumber (lpValue, buf, CAPACITY))
					DisplaySetValue (buf);
				GlobalUnlock (hgValue);
			}
		}
		/* ���������� ������ � ������� ������. */
		CloseClipboard ();
		SetFocus (hWndCalc);
	}
}
/*==============================================*/
/* ��������� ������� ������� ������������.      */
/* �����: hWnd - ����������� ���� ������������, */
/*        key - ���������� ��� ������� �������. */
/*==============================================*/
static void HandleKey (HWND hWnd, enum EKeys key)
{
	TCHAR msg[64];
	if (KEY0 <= key && key <= KEY9) {
		/* �������� �������. */
		/* ������� � ����� ����� �����. */
		calculator.mode = SwitchCalculatorMode (calculator.mode, NUMB);
		if (calculator.mode == NUMB)
			DisplayConcatDigit ((TCHAR) '0' + key);
	}
	else
		switch (key) {
			case KEYON:	/* ��������� ������������ */
				InitCalculator (DisplayGetZero (), FALSE);
				DisplayClearAll ();
				CalculatorError (FALSE);
				break;
			case KEYCE:	/* ����� ������ ��� ������� ���������� */
				if (calculator.mode == ERR) {
					/* ����� ������. */
					calculator.mode = UNOP;
					CalculatorError (FALSE);
				}
				else {
					/* ������� ����������. */
					calculator.mode = SwitchCalculatorMode (calculator.mode, NUMB);
					if (calculator.mode == NUMB)
						DisplayClearAll ();
				}
				break;
			case KEYBS:	/* �������� ��������� ����� */
				if (calculator.mode == NUMB)
					DisplayClearLastDigit ();
				break;
			case KEYDP:	/* ���������� ����� */
				/* ������� � ����� ����� �����. */
				calculator.mode = SwitchCalculatorMode (calculator.mode, NUMB);
				if (calculator.mode == NUMB)
					DisplayDecimalPoint ();
				break;
			case KEYSG:	/* ��������� ����� ����� */
				if (calculator.mode == NUMB)
					DisplaySign ();
				break;
			case KEYSB:	/* ��������� */
			case KEYAD:	/* �������� */
			case KEYML:	/* ��������� */
			case KEYDV:	/* ������� */
				/* ������� � ����� ������ ��������. */
				calculator.mode = SwitchCalculatorMode (calculator.mode, OPER);
				if (calculator.mode == OPER)
					calculator.op = key;	/* �������� �������� */
				break;
			case KEYEQ:	/* ����� */
				/* ������� � ����� ���������� � ����������. */
				if (calculator.mode != OPER)	/* � ������ ������ �������� ������� "=" ������������ */
					calculator.mode = SwitchCalculatorMode (calculator.mode, CALC);
					if (calculator.mode == CALC) {
					}
				break;
			case KEYMA:	/* ����������� � ������ ������ */
			case KEYMS:	/* ��������� �� ������ ������ */
			case KEYMR:	/* ��������� �� ������ ������ */
			case KEYMC:	/* ������� ������ ������ */
				calculator.mode = SwitchCalculatorMode (calculator.mode, UNOP);
				if (calculator.mode == UNOP)
					calculator.mode = MemoryAction (key);
				break;
			case KEYST:	/* ����������� ����������� ��������� ������������ */
				DebugCalculatorState ();
				break;
			case KEYQT:	/* ���������� ������ � ������������� */
				SendMessage (GetParent (hWndCalc), WM_CLOSE, 0, 0);
				break;
			case KEYCC:	/* ����������� ����������� ���������� � ����� ������ */
				ClipboardCopy ();
				/*MessageBox (hWnd, TEXT ("KEYCC"), TEXT ("iacalc.c - HandleKey ()"), MB_OK | MB_ICONINFORMATION);*/
				break;
			case KEYCV:	/* ������� ����������� ������ ������ � ��������� */
				calculator.mode = SwitchCalculatorMode (calculator.mode, NUMB);
				if (calculator.mode == NUMB)
					ClipboardPaste ();
				/* MessageBox (hWnd, TEXT ("KEYCV"), TEXT ("iacalc.c - HandleKey ()"), MB_OK | MB_ICONINFORMATION);*/
				break;
			default:	/* ����������� ������� */
				break;
				/*
				wsprintf (msg, TEXT ("The pressed key id code is %d"), key);
				MessageBox (hWnd, msg, TEXT ("iacalc.HandleKey ()"), MB_OK | MB_ICONINFORMATION);*/
		}
}
/*=================================================*/
/* ����������� ��������� ������������ ��� �������. */
/*=================================================*/
static void DebugCalculatorState (void)
{
	TCHAR msg[512];
	enum EKeys op = calculator.op;
	wsprintf (msg, TEXT ("regx = [%s]\nregy = [%s]\nop = [%c]\nregm = [%s]\nregc = [%s]"),
		calculator.regx, calculator.regy,
		op == KEYAD? (TCHAR)'+': op == KEYSB? (TCHAR)'-': op == KEYML? (TCHAR)'*': op == KEYDV? (TCHAR)'/': op == KEYEQ? (TCHAR)'=' : (TCHAR)'?', calculator.regm, calculator.regc);
	MessageBox (NULL, msg, TEXT ("DebugCalculatorState ()"), MB_OK | MB_ICONINFORMATION);
}