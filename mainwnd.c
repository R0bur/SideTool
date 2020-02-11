#include <windows.h>
#include <windowsx.h>
#include "mainwnd.h"
#include "iacalc.h"
#include "iacalend.h"
#include "iactrl.h"
#include "debug.h"
#define WMU_APPBARNOTIFY WM_USER + 0
#define IDTOOLS 10
/*---------------------------*/
/* �������� �� ������������. */
/*---------------------------*/
#define NABTOOLS 3
enum EToolPlace {TOOLPLACE_TOP, TOOLPLACE_BOTTOM};
static struct SABTool {
	int w, h;		/* ������ � ������ ���� ����������� */
	HWND hWnd;		/* DEBUG: ����������� ����. */
	enum EToolPlace place;	/* ���������� ����������� �� ������ */
	HBRUSH hbrBackground;	/* ����� ��� ���� ���� */
	WNDPROC lpfnWndProc;	/* ������� ��������� */
} abTools[NABTOOLS];
/*====================================================*/
/* ���������� ��������� �� ���������� �� �����������. */
/* �����: pABTool - ��������� �� ���������,           */
/*        w, h - ������ � ������ ���� �����������,    */
/*        place - ���������� ����������� �� ������,   */
/*        hbrBackground - ����� ��� ���� ����,        */
/*        lpfnWndProc - ������� �������.              */
/*====================================================*/
void SetABTool (struct SABTool* pABTool, int w, int h, enum EToolPlace place, HBRUSH hbrBackground, WNDPROC lpfnWndProc)
{
	pABTool->w = w;
	pABTool->h = h;
	pABTool->place = place;
	pABTool->hbrBackground = hbrBackground;
	pABTool->lpfnWndProc = lpfnWndProc;
}
BOOL CALLBACK PlaceToolWindow (HWND hWnd, LPARAM lParam);
/*---------------------------------------------------*/
/* ���������� ������� ���������� ������� ����������. */
/*---------------------------------------------------*/
static BOOL AppBarRegister (HWND hWnd);
static BOOL AppBarUnregister (HWND hWnd);
static void AppBarPosChanged (HWND hWnd, UINT uEdge);
/*-------------------------------------------------------*/
/* ���������� ������� ��������� ��������� �������� ����. */
/*-------------------------------------------------------*/
static LRESULT WMUAppBarNotify (HWND hWnd, UINT uNotifyMsg, LPARAM lParam);
static LRESULT WMCreate (HWND hWnd, LPCREATESTRUCT lpCs);
static LRESULT WMActivate (HWND hWnd, WORD uAct, HWND hWndCo);
static LRESULT WMWindowPosChanged (HWND hWnd, LPWINDOWPOS lpWp);
static LRESULT WMHotKey (HWND hWnd, WORD hkid, WORD modk);
static LRESULT WMDestroy (HWND hWnd);
/*-------------------------------------*/
/* ���������� ��������������� �������. */
/*-------------------------------------*/
BOOL MainWndChildrenCreate (HINSTANCE hInstance, HWND hWndParent);
/*========================================*/
/* ������� �������� ���� ����������.      */
/* �����: hWnd - ����������� ����,        */
/*        uMsg - ������������� ���������, */
/*        wParam - ������ ��������,       */
/*        lParam - ������ ��������.       */
/*========================================*/
LRESULT CALLBACK MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes;
	lRes = 0;
	switch (uMsg) {
		case WM_CREATE:		/* �������� �������� ���� ����������. */
			lRes = WMCreate (hWnd, (LPCREATESTRUCT)lParam);
			break;
		case WM_ACTIVATE:	/* ��������� ���� ���������� ����. */
			lRes = WMActivate (hWnd, (WORD)LOWORD (wParam), (HWND)lParam);
			break;
		case WM_WINDOWPOSCHANGED:	/* ���������� �������������� ����. */
			lRes = WMWindowPosChanged (hWnd, (LPWINDOWPOS)lParam);
			break;
		case WM_HOTKEY:		/* ������ ������������������ ��������� ������. */
			lRes = WMHotKey (hWnd, (WORD)wParam, (WORD)LOWORD (lParam));
			break;
		case WMU_APPBARNOTIFY:	/* ��������� ��� ������ ����������. */
			lRes = WMUAppBarNotify (hWnd, wParam, lParam);
			break;
		case WM_DESTROY:	/* ����������� �������� ���� ����������. */
			lRes = WMDestroy (hWnd);
			break;
		default:		/* �������� �� ������������� ���������. */
			lRes = DefWindowProc (hWnd, uMsg, wParam, lParam);
	}
	return lRes;
}
/*===================================================*/
/* ����������� ������ ����������.                    */
/* �����: hWnd - ����������� ���� ������ ����������. */
/* �������: TRUE - ������ ����������������,          */
/*          FALSE - ������ ����������� ������.       */
/*===================================================*/
static BOOL AppBarRegister (HWND hWnd)
{
	APPBARDATA abd;
	abd.cbSize = sizeof (APPBARDATA);
	abd.hWnd = hWnd;
	abd.uCallbackMessage = WMU_APPBARNOTIFY;
	return SHAppBarMessage (ABM_NEW, &abd);
}
/*======================================================*/
/* ������ � ����������� ������ ����������.              */
/* �����: hWnd - ����������� ���� ������ ����������.    */
/* �������: TRUE - ������ � ����������� ������ �������, */
/*          FALSE - ������ ������ � �����������.        */
/*======================================================*/
static BOOL AppBarUnregister (HWND hWnd)
{
	APPBARDATA abd;
	abd.cbSize = sizeof (APPBARDATA);
	abd.hWnd = hWnd;
	return SHAppBarMessage (ABM_REMOVE, &abd);
}
/*==================================================================*/
/* ��������� �������� � �������������� ������ ����������.           */
/* �����: hWnd - ����������� ���� ������ ����������,                */
/*        uEdge - ��� ������� ������, � ������� ����������� ������. */
/*==================================================================*/
static void AppBarPosChanged (HWND hWnd, UINT uEdge)
{
	APPBARDATA abd;
	RECT rc;
	int nWidth;
	/* ��������� ���������� � �������� ������ ����������. */
	GetWindowRect (hWnd, &rc);
	nWidth = rc.right - rc.left;
	/* ���������� �������� � ������ ����������. */
	abd.cbSize = sizeof (APPBARDATA);
	abd.hWnd = hWnd;
	abd.uEdge = uEdge;
	/* ����������� �������� ������� ������ ��� ������ ����������. */
	abd.rc.top = 0;
	abd.rc.bottom = GetSystemMetrics (SM_CYSCREEN);
	switch (uEdge) {
		case ABE_LEFT:
			abd.rc.left = 0;
			abd.rc.right = abd.rc.left + nWidth;
			break;
		case ABE_RIGHT:
			abd.rc.right = GetSystemMetrics (SM_CXSCREEN);
			abd.rc.left = abd.rc.right - nWidth;
	}
	/* ������ �� ��������� �������� ������ ����������. */
	SHAppBarMessage (ABM_QUERYPOS, &abd);
	/* ������������� ������������ �������� ������ ����������. */
	switch (uEdge) {
		case ABE_LEFT:
			abd.rc.right = abd.rc.left + nWidth;
			break;
		case ABE_RIGHT:
			abd.rc.left = abd.rc.right - nWidth;
			break;
	}
	/* ��������� ����� �������� ������ ����������. */
	SHAppBarMessage (ABM_SETPOS, &abd);
	MoveWindow (hWnd, abd.rc.left, abd.rc.top, abd.rc.right - abd.rc.left, abd.rc.bottom - abd.rc.top, TRUE);
}
/*=========================================================*/
/* ��������� �����������, ������������� ������ ����������. */
/* �����: hWnd - ������������� ���� ������ ����������,     */
/*        uNotifyMsg - ��� �����������,                    */
/*        lParam - �������������� ��������.                */
/* �������: ��� ��������� �����������.                     */
/*=========================================================*/
static LRESULT WMUAppBarNotify (HWND hWnd, UINT uNotifyMsg, LPARAM lParam)
{
	LRESULT lRes = 0;
	switch (uNotifyMsg) {
		case ABN_POSCHANGED:	/* �������� ����������� �������� �������������� ��� ������� ������ */
			AppBarPosChanged (hWnd, ABE_RIGHT);
			break;
		case ABN_STATECHANGE:	/* ��������� ���� "auto hide" ��� "always on top" */
			MessageBox (hWnd, TEXT ("ABN_STATECHANGE"), TEXT ("WMUAppBarNotify"), MB_OK);
			break;
		case ABN_FULLSCREENAPP:	/* �������� ��� ������� ������������� ���������� */
			SetWindowPos(hWnd, (BOOL)lParam? HWND_BOTTOM: HWND_TOPMOST, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE); 
			break;
		case ABN_WINDOWARRANGE:	/* ������� ������� ������������ ���� */
			ShowWindow (hWnd, (BOOL)lParam? SW_HIDE: SW_SHOW);
			break;
		default:
			MessageBox (hWnd, TEXT ("Unknown notification message"), TEXT ("WMUAppBarNotify"), MB_OK);
	}
	return lRes;
}
/*==============================================*/
/* ��������� ��������� WM_CREATE.               */
/* �����: hWnd - ����������� ������������ ����. */
/* �������: ��� ��������� ���������.            */
/*==============================================*/
static LRESULT WMCreate (HWND hWnd, LPCREATESTRUCT lpCs)
{
	LRESULT lRes;
	lRes = 0;
	/* �������� �������� ���� - ���� ������������. */
	MainWndChildrenCreate (GetModuleHandle (NULL), hWnd);
	/* ����������� ������ ���������� � ���������������� � � ���������� ������� ������. */
	if (AppBarRegister (hWnd)) {
		AppBarPosChanged (hWnd, ABE_RIGHT);
		/* ����������� ��������� ������ ��� ������ ���������� �� ������ ��������� �������. */
		RegisterHotKey (hWnd, HKACTIVATE, MOD_ALT | MOD_CONTROL, 67);
	}
	else {
		MessageBox (hWnd, TEXT ("Cannot register Application bar."), TEXT ("Fatal error"), MB_OK | MB_ICONHAND);
		lRes = -1;
	}
	return lRes;
}
/*============================================*/
/* ��������� ��������� WM_ACTIVATE.           */
/* �����: hWnd - ����������� ���� ����������, */
/*        uAct - �������� � ����� ����������, */
/*        hWndCo - ����-������������� (�����- */
/*               ��� ��� ���������� �����).   */
/* �������: ��� ��������� ���������.          */
/*============================================*/
static LRESULT WMActivate (HWND hWnd, WORD uAct, HWND hWndCo)
{
	APPBARDATA abd;
	abd.cbSize = sizeof (APPBARDATA);
	abd.hWnd = hWnd;
	SHAppBarMessage (ABM_ACTIVATE, &abd); /* ������ ���������� � ����� �� WM_ACTIVATE �� ������������ */
	/* �������� ������ ��������� ���� ������������. */
	/* TODO: ����������� �������� ������ ����� ������ ������������. */
	if (uAct != WA_INACTIVE) {
		/* ���� ���������� ������������. */
		SetFocus (abTools[0].hWnd);
		IACalcActive (1);
	}
	else
		IACalcActive (0);
	return 0;
}
/*==============================================*/
/* ��������� ��������� WM_WINDOWPOSCHANGED.     */
/* �����: hWnd - ����������� ���� ����������.   */
/*        lpWp - ��������� �� �������� �� ����. */
/* �������: ��� ��������� ���������.            */
/*==============================================*/
static LRESULT WMWindowPosChanged (HWND hWnd, LPWINDOWPOS lpWp)
{
	APPBARDATA abd;
	RECT rc;
	/* ������������� �������� �������� ���� (���� ������������). */
	GetClientRect (hWnd, &rc);
	EnumChildWindows (hWnd, PlaceToolWindow, (LPARAM) &rc);
	/* ����������� ������������ ������� �� ��������� �������������� ��� �������� ������ ����������. */
	abd.cbSize = sizeof (APPBARDATA);
	abd.hWnd = hWnd;
	SHAppBarMessage (ABM_WINDOWPOSCHANGED, &abd); /* ������ ���������� � ����� �� WM_WINDOWPOSCHANGED �� ������������ */
	return 0;
}
/*===============================================*/
/* ��������� ��������� WM_HOTKEY.                */
/* �����: hWnd - ����������� ���� ����������,    */
/*        hkid - ������������� ��������� ������, */
/*        modk - ����� ������-�������������.     */
/* �������: ��� ��������� ���������.             */
/*===============================================*/
static LRESULT WMHotKey (HWND hWnd, WORD hkid, WORD modk)
{
	static HWND hWndBack = NULL;
	HWND hWndCur;
	hWndCur = GetForegroundWindow ();
	if (hWndCur != hWnd) {
		hWndBack = hWndCur;
		SetForegroundWindow (hWnd);
	}
	else if (hWndBack != NULL)
		SetForegroundWindow (hWndBack);
	return 0;
}
/*=========================================================*/
/* ���������������� ���� ����������� �� ������ ����������. */
/* �����: hWnd - ����������� ���� �����������,             */
/*        lParam - ��������� �� ������� RECT, ���������    */
/*                 ��� ���������� ���� �����������.        */
/* �������� ������: �� ������� RECT, �� ������� ���������  */
/*        lParam, ����������� ������� ����� �����������    */
/*        ������������.                                    */
/*=========================================================*/
BOOL CALLBACK PlaceToolWindow (HWND hWnd, LPARAM lParam)
{
	LPRECT lprc;
	int i, x, y, w, h;
	/* ����������� ������ �����������, ��� �������� ��������� ����������� ����. */
	i = GetWindowLong (hWnd, GWL_ID) - IDTOOLS;
	/* ���������� ���� ������������ �� ���������������. */
	if (i >= 0 && i < NABTOOLS) {
		lprc = (LPRECT) lParam;
		if (abTools[i].place == TOOLPLACE_TOP) {
			y = lprc->top;
			/* �������� ���� ������� ������� ��������� ������� */
			lprc->top += abTools[i].h;
		}
		else {
			y = lprc->bottom - abTools[i].h;
			/* �������� ����� ������ ������� ��������� ������� */
			lprc->bottom -= abTools[i].h;
		}
		x = (lprc->right - abTools[i].w) / 2; 
		MoveWindow (hWnd, x, y, abTools[i].w, abTools[i].h, TRUE);
	}
	return TRUE;
}
/*==============================================*/
/* ��������� ��������� WM_DESTROY.              */
/* �����: hWnd - ����������� ������������ ����. */
/* �������: ��� ��������� ���������.            */
/*==============================================*/
static LRESULT WMDestroy (HWND hWnd)
{
	AppBarUnregister (hWnd);
	UnregisterHotKey (hWnd, HKACTIVATE);
	PostQuitMessage (0);
	return 0;
}
/*=======================================================*/
/* ����������� ������ �������� ���� ����������.          */
/* �����: hInstance - ����������� ���������� ����������. */
/* �������: ���� ������ �������� ���� ����������.        */
/*=======================================================*/
ATOM MainWndRegisterClass (HINSTANCE hInstance)
{
	WNDCLASSEX wcx;
	ATOM awcx;
	HBITMAP hBackground;
	hBackground = LoadBitmap (hInstance, "BACKGROUND");
	wcx.cbSize = sizeof (wcx);
	wcx.lpfnWndProc = MainWndProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = hInstance;
	wcx.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	wcx.hCursor = LoadCursor (NULL, IDC_ARROW);
	wcx.hbrBackground = CreatePatternBrush (hBackground);
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = "MainWindowClass";
	wcx.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
	awcx = RegisterClassEx (&wcx);
	DeleteObject (hBackground);
	return awcx;
}
/*=======================================================*/
/* �������� �������� ���� ����������.                    */
/* �����: hInstance - ����������� ���������� ����������, */
/*        wcx - ���� ������ �������� ����,               */
/*        lpTitle - ��������� �� ������ ��������� ����.  */
/* �������: ����������� �������� ���� ����������.        */
/*=======================================================*/
HWND MainWndCreate (HINSTANCE hInstance, ATOM wcx, LPCTSTR lpTitle)
{
	HWND hWnd;
	RECT rc;
	int i;
	/* ������������ ���������� �� ������������. */
	/* SetABTool (abTools + 0, 240, 300, TOOLPLACE_BOTTOM, IACalcWindowProc); */
	SetABTool (abTools + 0, 240, 300, TOOLPLACE_BOTTOM, IACalcHbrBackground (), IACalcWindowProc);
	SetABTool (abTools + 1, 240, 32, TOOLPLACE_TOP, IACtrlHbrBackground (), IACtrlWindowProc);
	SetABTool (abTools + 2, 240, 240, TOOLPLACE_TOP, IACalendHbrBackground (), IACalendWindowProc);
	/* ���������� �������� �������, ��������� �������� ���� ������ �����������. */
	rc.left = 0;
	rc.top = 0;
	rc.right = 0;
	rc.bottom = 0;
	for (i = 0; i < NABTOOLS; i++) {
		if (abTools[i].w > rc.right)
			rc.right = abTools[i].w;
		if (abTools[i].h > rc.bottom)
			rc.bottom = abTools[i].h;
	}
	/* �������� ����. */
	AdjustWindowRectEx (&rc, WS_POPUP, FALSE, WS_EX_TOOLWINDOW);
	/* ���� �� ������� ����� WS_EX_TOOLWINDOW, �� ���� ����� �������� ����� ���������� � �������� ���������. */
	hWnd = CreateWindowEx (WS_EX_TOPMOST | WS_EX_TOOLWINDOW, (LPCTSTR) MAKELONG (wcx, 0), lpTitle,
		WS_POPUP | WS_VISIBLE, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL
	);
	return hWnd;
}
/*=====================================================*/
/* �������� �������� ���� - ���� ������������.         */
/* �����: hInstance - ����������� ����������,          */
/*        hParentWnd - ����������� ������������� ����. */
/* �������: TRUE - �������� ���� �������,              */
/*          FALSE - ������ �������� �������� ����.     */
/*=====================================================*/
BOOL MainWndChildrenCreate (HINSTANCE hInstance, HWND hWndParent)
{
	BOOL bRes = FALSE;
	WNDCLASSEX wcx;
	ATOM awcx;
	HWND hWnd;
	TCHAR lpszClassName[32];
	int i;
	/* ������� ������������. */
	wcx.cbSize = sizeof (wcx);
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = hInstance;
	wcx.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	wcx.hCursor = LoadCursor (NULL, IDC_ARROW);
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = lpszClassName;
	wcx.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
	bRes = TRUE;
	for (int i = 0; bRes && i < NABTOOLS; i++) {
		/* ��������� ��������, ����������� ��� �����������. */
		wsprintf (lpszClassName, "ToolWindowClass%d", IDTOOLS + i);
		wcx.hbrBackground = abTools[i].hbrBackground;
		wcx.lpfnWndProc = abTools[i].lpfnWndProc;
		/* ����������� ������ ���� �����������. */
		awcx = RegisterClassEx (&wcx);
		/* �������� ���� �����������. */
		if (awcx) {
			hWnd = CreateWindowEx (0, (LPCTSTR) MAKELONG (awcx, 0), (LPCTSTR) NULL,
				WS_CHILD | WS_BORDER | WS_VISIBLE, 0, 0, abTools[i].w, abTools[i].h,
				hWndParent, (HMENU)(int)(IDTOOLS + i), hInstance, NULL
			);
			abTools[i].hWnd = hWnd;
			if (!hWnd) {
				bRes = FALSE;
				MessageBox (hWndParent, TEXT ("Child window create error"), TEXT ("Debug"), MB_OK);
			}
		}
		else
			bRes = FALSE;
	}
	return bRes;
}