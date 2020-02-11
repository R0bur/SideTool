/*==================================*/
/* ��������� ��� ������ ����������. */
/*==================================*/
#include <windows.h>
#include <commctrl.h>
#include "iacalend.h"
static LRESULT WMPaint (HWND hWnd);
static LRESULT WMCreate (HWND hWnd);
static void iaCalendSelectFont (HWND hWnd, int mw, int mh);
static HBRUSH hbrBackground;
/*===========================================*/
/* ��������� ����� ��� ���������� ���� ����. */
/* �������: ����������� �����.               */
/*===========================================*/
HBRUSH IACalendHbrBackground (void)
{
	if (!hbrBackground)
		hbrBackground = GetStockObject (DKGRAY_BRUSH);
	return hbrBackground;
}
/*===========================================*/
/* ������� ������� ���� ���������.           */
/* �����: hWnd - ����������� ���� ���������, */
/*        uMsg - ������������� ���������,    */
/*        wParam - ������ ��������,          */
/*        lParam - ������ ��������.          */
/*===========================================*/
LRESULT CALLBACK IACalendWindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	switch (uMsg) {
		case WM_CREATE:	/* �������� ����. */
			lRes = WMCreate (hWnd);
			break;
/*		case WM_PAINT: */ /* ��������� ���������� ������� ����. */
/*			lRes = WMPaint (hWnd);
			break;*/
		default:
			lRes = DefWindowProc (hWnd, uMsg, wParam, lParam);
	}
	return lRes;
}
/*==============================================*/
/* ��������� ��������� WM_CREATE.               */
/* �����: hWnd - ����������� ����.              */
/* �������: ��� ���������� ��������� ���������. */
/*==============================================*/
static LRESULT WMCreate (HWND hWnd)
{
	LRESULT lRes = 0;
	RECT rc;
	HWND hCldr;
	int mw, mh;
	HINSTANCE hInstance = (HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE);
	hCldr = CreateWindowEx (0, MONTHCAL_CLASS, "", WS_BORDER | WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		hWnd, NULL, hInstance, NULL);
	GetClientRect (hWnd, &rc);
	mw = rc.right - rc.left;
	mh = rc.bottom - rc.top;
	iaCalendSelectFont (hCldr, mw, mh);
	MoveWindow (hCldr, 0, 0, mw, mh, FALSE);
	ShowWindow (hCldr, SW_SHOW);
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
		TextOut (hdc, 0, 0, TEXT ("Calendar"), 8);
		EndPaint (hWnd, &ps);
	}
	return lRes;
}
/*======================================================*/
/* ������ ������ ��� �������� ���������� "���������",   */
/* ����� ��� ����������� ����������� �� ���������� ���  */
/* ���� ��������.                                       */
/* �����: hWnd - ����������� �������� ����������,       */
/*        mw - ������������ ������ �������� ����������, */
/*        mh - ������������ ������ �������� ����������. */
/*======================================================*/
static void iaCalendSelectFont (HWND hWnd, int mw, int mh)
{
	TCHAR DebugBuf[512];
	int dh;
	int fw = 1;
	int fh = 1;
	LOGFONT lf;
	HFONT hf0, hf1;
	RECT rc;
	BOOL cont;
	hf0 = (HFONT)SendMessage (hWnd, WM_GETFONT, 0, 0);
	GetObject (hf0, sizeof (lf), &lf);
/*
	sprintf (DebugBuf, "lfHeight = %d\nlfWidth = %d\nlfFace = %s", lf.lfHeight, lf.lfWidth, lf.lfFaceName);
	MessageBox (hWnd, DebugBuf, TEXT ("DEBUG: Current font"), MB_OK | MB_ICONINFORMATION);
*/
	dh = lf.lfHeight < 0? -1: 1;
	do {
		/* ��������� �������� ������� ������. */
		lf.lfHeight += dh;
		hf1 = CreateFontIndirect (&lf);
		SendMessage (hWnd, WM_SETFONT, (WPARAM)hf1, FALSE);
		/* ������ �������. */
		cont = MonthCal_GetMinReqRect (hWnd, &rc);
		/* ������� � ��������� ������. */
		SendMessage (hWnd, WM_SETFONT, (WPARAM)hf0, FALSE);
		DeleteObject (hf1);
		/* ������ ������ �������. */
		cont = cont && (rc.right - rc.left < mw) && (rc.bottom - rc.top < mh);
	} while (cont);
	/* ��������� ���������� ����������� ������� ������. */
	lf.lfHeight -= dh;
	hf1 = CreateFontIndirect (&lf);
	SendMessage (hWnd, WM_SETFONT, (WPARAM)hf1, TRUE);
}
