/*=======================*/
/* ��������� ������������. */
/*=======================*/
#include <windows.h>
#include "iadisp.h"
#include "iastring.h"
/*-------------------------*/
/* �������� �� ����������. */
/*-------------------------*/
static void DisplayUpdate (void);
#define ZERO "0,"
static struct {
	TCHAR value[CAPACITY];	/* ������, ������������ �� ���������� */
	int nCap;	/* ����������� ������������ (������ �����) */
	int n;		/* ������� ���������� (������ �����) */
	int m;		/* ����� �������, ����� �������� ����� ���������� ����������� */
	HWND hWnd;	/* ����������� �������� ���������� */
	HFONT hFont, hFontOld;	/* ������� ����� � ������ ����� �������� ���������� */
} display;
/*==============================================================*/
/* ��������� "������������� ����".                              */
/* �������: ��������� �� ������ � ������� ��������� ����������. */
/*==============================================================*/
LPCTSTR DisplayGetZero (void)
{
	return ZERO;
}
/*=============================================================*/
/* �������� ����������.                                        */
/* �����: hWnd - ������������ ����,                            */
/*        lpRc - ��������� �� �������, ��������� ��� �������. */
/* �������: ����������� ���� �������.                          */
/*=============================================================*/
HWND DisplayCreate (HWND hWnd, LPRECT lpRc)
{
	BOOL lRes;
	HWND hWndTmp;
	LOGFONT lf;
	RECT rc;
	/* �������� �����. */
	rc = *lpRc;
	hWndTmp = CreateWindow (TEXT ("STATIC"), NULL, WS_CHILD | WS_VISIBLE | SS_GRAYRECT,
		rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
		hWnd, NULL, (HINSTANCE) GetWindowLong (hWnd, GWL_HINSTANCE), NULL);
	/* �������� �������. */
	rc.left += 2;
	rc.top += 2;
	rc.right -= 2;
	rc.bottom -= 2;
	hWndTmp = CreateWindow (TEXT ("STATIC"), TEXT (ZERO), WS_CHILD | WS_VISIBLE | SS_RIGHT,
		rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
		hWnd, (HMENU) IDDISPLAY, (HINSTANCE) GetWindowLong (hWnd, GWL_HINSTANCE), NULL);
	/* ��������� ������ ������� ������������. */
	/* TODO: ����������� ���������� ������� ������ � ������������ � ��������� ��������������� ������� �����������. */
	GetObject (GetStockObject (ANSI_FIXED_FONT), sizeof (LOGFONT), &lf);
	lf.lfWeight = FW_BOLD;
	lf.lfHeight = 36;
	lf.lfWidth = 16;
	display.hFont = CreateFont (lf.lfHeight, lf.lfWidth, lf.lfEscapement, lf.lfOrientation,
		lf.lfWeight, lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet,
		lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality, lf.lfPitchAndFamily, lf.lfFaceName);
	display.hFontOld = (HFONT) SendMessage (hWndTmp, WM_GETFONT, 0, 0);
	SendMessage (hWndTmp, WM_SETFONT, (WPARAM) display.hFont, MAKELONG (0, 0));
	/* ������������� �������������. */
	display.nCap = CAPACITY - 3;
	StrCpy (display.value, ZERO);
	display.n = 1;
	display.m = 0;
	display.hWnd = hWndTmp;
	return hWndTmp;
}
/*===========================================================*/
/* ���������� ��������, ������������� �� ���������� �������. */
/*===========================================================*/
static void DisplayUpdate (void)
{
	SendMessage (display.hWnd, WM_SETTEXT, 0, (LPARAM) TEXT (display.value));
}
/*==================================*/
/* ������� ���������� ������������. */
/*==================================*/
void DisplayClearAll (void)
{
	/* MessageBox (NULL, TEXT ("DisplayClearAll ()"), TEXT ("Debug"), MB_OK | MB_ICONINFORMATION); */
	StrCpy (display.value, ZERO);
	display.n = 1;
	display.m = 0;
	DisplayUpdate ();
}
/*======================================================*/
/* �������� ��������� ����� �� ���������� ������������. */
/*======================================================*/
void DisplayClearLastDigit (void)
{
	int p, q;
	/* MessageBox (NULL, TEXT ("DisplayClearLastDigit ()"), TEXT ("Debug"), MB_OK | MB_ICONINFORMATION); */
	if (display.m == display.n) {
		/* ����� ������� ���������� �����. */
		display.m = 0;
	}
	else {
		q = display.value[0] == (TCHAR)'-'? 1: 0;	/* ������� ������� ��� ���� */
		if (display.n > 1) {
			/* �������� ��������� ����� �� �������. */
			p = display.n + q;
			if (display.m > 0) {
				/* �������� ����� �� ������� ����� �����. */
				display.value[p] = display.value[p + 1];	/* ����� �������� ����� ������ ����� */
				display.n--;	/* ���� �������� ����� */
			}
			else {
				/* �������� ����� �� ����� ����� �����. */
				display.value[p - 1] = display.value[p];	/* ����� ����������� ����������� �����. */
				display.value[p] = display.value[p + 1];	/* ����� �������� ����� ������ �����. */
				display.n--;	/* ���� �������� ����� */
			}	
		}
		else
			/* �������� ������������ ����� �� �������. */
			display.value[q] = (TCHAR)'0';
		/* ���������� ��������� ����������. */
		DisplayUpdate ();
	}
}
/*=============================*/
/* ��������� ���������� �����. */
/*=============================*/
void DisplayDecimalPoint (void)
{
	/* MessageBox (NULL, TEXT ("DisplayDecimalPoint ()"), TEXT ("Debug"), MB_OK | MB_ICONINFORMATION); */
	if (display.m == 0)
		display.m = display.n;
}
/*========================*/
/* ��������� ����� �����. */
/*========================*/
void DisplaySign (void)
{
	int i, n;
	n = display.n;
	if (display.value[0] == (TCHAR)'-') {
		/* ����� ������������� */
		for (i = 0; i <= n; i++)
			display.value[i] = display.value[i + 1];	/* ����� ���� � ����������� ����������� ����� */
		display.value[i] = display.value[i + 1];		/* ����� �������� ����� ������ ����� */
	}
	else {
		/* ����� �� ������������� */
		display.value[n + 2] = display.value[n + 1];		/* ����� �������� ����� ������ ������ */
		for (i = n + 1; i > 0; i--)
			display.value[i] = display.value[i - 1];	/* ����� ���� � ����������� ����������� ������ */
		display.value[0] = (TCHAR)'-';
	}
	/* ���������� ��������� ����������. */
	DisplayUpdate ();
}
/*=============================================*/
/* ���������� ����� �� ��������� ������������. */
/* �����: digit - �����.                       */
/*=============================================*/
void DisplayConcatDigit (TCHAR digit)
{
	/* MessageBox (NULL, TEXT ("DisplayConcatDigit ()"), TEXT ("Debug"), MB_OK | MB_ICONINFORMATION); */
	int p, q;
	if (display.n < display.nCap) {
		/* ���� ��������� ������� ��� ����. */
		q = display.value[0] == (TCHAR)'-'? 1: 0;	/* ������� ������� ��� ���� */
		p = display.n + q;				/* ���������� �������� � ������ ����� */
		if (display.m == 0) {
			/* �� ������� - ����� �����. */
			if (display.n == 1 && display.value[q] == (TCHAR)'0')
				/* �� ������� ������������ ����. */
				display.value[q] = digit;
			else {
				display.value[p + 2] = display.value[p + 1];	/* ����� �������� ����� ������ ������ */
				display.value[p + 1] = display.value[p];	/* ����� ����������� ����������� ������ */
				display.value[p] = digit;			/* ������� �������� ����� */
				display.n++;	/* ���� ������������ ������� */
			}
		}
		else {
			/* �� ������� - ������� �����. */
			display.value[p + 2] = display.value[p + 1];	/* ����� �������� ����� ������ ������ */
			display.value[p + 1] = digit;			/* ������� �������� ����� */
			display.n++;	/* ���� ������������ ������� */
		}
	}
	DisplayUpdate ();	
}
/*================================================================*/
/* ��������� ��������, ������������� �� ����������.               */
/* �������: ����������� ��������� �� ������, ���������� ��������. */
/*================================================================*/
LPCTSTR DisplayGetValue (void)
{
	return (LPCTSTR)display.value;
}
/*=============================================================*/
/* ��������� �������� ��� ����������� �� ����������.           */
/* �����: v - ����������� ��������� �� ������ ��� �����������. */
/*=============================================================*/
void DisplaySetValue (LPCTSTR v)
{
	int i, n, m, dp;
	/* ������ �������� � ��������������� ��������. */
	dp = 1;
	n = 0;
	m = 0;	/* ����� �������, ����� �������� ����� ���������� �����������, ���� ���� ������� ����� */
	for (i = 0; v[i] != (TCHAR)'\0'; i++)
		if ((TCHAR)'0' <= v[i] && v[i] <= (TCHAR)'9')
			n++;
		else if (v[i] == (TCHAR)'.' || v[i] == (TCHAR)',')
			m = n;
	display.n = n;
	display.m = n > m? m: 0;
	/* ����������� �������� �� ����������. */
	StrNCpy (display.value, v, CAPACITY);
	(display.value)[CAPACITY - 1] = (TCHAR) '\0';
	DisplayUpdate ();
}
/*=========================*/
/* ����������� ����������. */
/*=========================*/
void DisplayDestroy (void)
{
}