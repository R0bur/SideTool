/*==============================*/
/* ���� ���������� �����������. */
/*==============================*/
#include <windows.h>
#include <shellapi.h>
#include "iactrl.h"
#define CMD_QUIT 100
#define CMD_HELP 101
#define CMD_WEB 102
static HBRUSH hbrBackground = 0;
static LRESULT WMCreate (HWND hWnd);
static LRESULT WMCommand (HWND hWnd, WORD wCode, WORD wId, HWND hWndCtl);
static void CmdQuit (HWND hWnd);
static void CmdHelp (HWND hWnd);
static void CmdWeb (HWND hWnd);
/*===========================================*/
/* ��������� ����� ��� ���������� ���� ����. */
/* �������: ����������� �����.               */
/*===========================================*/
HBRUSH IACtrlHbrBackground (void)
{
	if (!hbrBackground)
		hbrBackground = GetStockObject (DKGRAY_BRUSH);
	return hbrBackground;
}
/*===========================================*/
/* ������� ������� ����� ����������.         */
/* �����: hWnd - ����������� ���� ���������, */
/*        uMsg - ������������� ���������,    */
/*        wParam - ������ ��������,          */
/*        lParam - ������ ��������.          */
/*===========================================*/
LRESULT CALLBACK IACtrlWindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes;
	lRes = 0;
	switch (uMsg) {
		case WM_CREATE:	/* �������� ����. */
			lRes = WMCreate (hWnd);
			break;
		case WM_COMMAND: /* ��������� ��������� �� ������. */
			lRes = WMCommand (hWnd, HIWORD (wParam), LOWORD (wParam), (HWND) lParam);
			break;
		default:	/* �� ������������ ���������. */
			lRes = DefWindowProc (hWnd, uMsg, wParam, lParam);
	}
	return lRes;
}
/*===================================*/
/* ��������� ��������� WM_CREATE.    */
/* �����: hWnd - ����������� ����.   */
/* �������: ��� ��������� ���������. */
/*===================================*/
static LRESULT WMCreate (HWND hWnd)
{
	RECT rc;	/* ���������� ������� ������� ���� */
	const int wb = 24;	/* ������ ������ */
	const int hb = 24;	/* ������ ������ */
	int x, y;
	HINSTANCE hInstance = (HINSTANCE) GetWindowLong (hWnd, GWL_HINSTANCE);
	GetClientRect (hWnd, &rc);
	x = rc.right - wb - 1;
	y = rc.top + (rc.bottom - rc.top - hb) / 2;
	CreateWindow (TEXT ("BUTTON"), TEXT ("X"), WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		x, y, wb, hb, hWnd, (HMENU) CMD_QUIT, hInstance, NULL);
	x -= (wb + 1);
	CreateWindow (TEXT ("BUTTON"), TEXT ("?"), WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		x, y, wb, hb, hWnd, (HMENU) CMD_HELP, hInstance, NULL);
	x = 1;
	CreateWindow (TEXT ("BUTTON"), TEXT ("WWW"), WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		x, y, 3*wb, hb, hWnd, (HMENU) CMD_WEB, hInstance, NULL);
	return 0;
}
/*=======================================*/
/* ��������� ��������� WM_COMMAND.       */
/* �����: hWnd - ����������� ����,       */
/*        wCode - ��� ���������,         */
/*        wId - ������������� ���������, */
/*        hWndCtl - ����������� �������� */
/*              ����������.              */
/* �������: ��� ��������� ���������.     */
/*=======================================*/
static LRESULT WMCommand (HWND hWnd, WORD wCode, WORD wId, HWND hWndCtl)
{
	LRESULT lRes = 0;
	TCHAR msg[256];
	if (!wCode)
		switch (wId) {
			case CMD_QUIT:
				CmdQuit (hWnd);
				break;
			case CMD_HELP:
				CmdHelp (hWnd);
				break;
			case CMD_WEB:
				CmdWeb (hWnd);
				break;
			default:
				wsprintf (msg, TEXT ("Code:\t%u\nId:\t%u\nHWND:\t%u"), wCode, wId, hWndCtl);
				MessageBox (hWnd, msg, TEXT ("iactl.c - WMCommand"), MB_OK | MB_ICONINFORMATION);
		}
	return lRes;
}
/*====================================================*/
/* ��������� ������� �� ���������� ������ ����������. */
/* �����: hWnd - ����������� ���� ����� ����������.   */
/*====================================================*/
static void CmdQuit (HWND hWnd)
{
	HWND hWndParent;
	hWndParent = GetParent (hWnd);
	SendMessage (hWndParent, WM_CLOSE, 0, 0);
}
/*====================================================*/
/* ��������� ������� �� ������ ���������� ����������. */
/* �����: hWnd - ����������� ���� ����� ����������.   */
/*====================================================*/
static void CmdHelp (HWND hWnd)
{
	LANGID lang;
	static LPCTSTR s[] = {
		TEXT ("���������"),
		TEXT ("���������� ������:\nCtrl+Q\t- ����� �� ���������,\nCtrl+C\t- ����������� �������� � ����� ������,\nCtrl+V\t- ������� �������� �� ������ ������,\nCtrl+Alt+C\t- ������\376����� �� ������ ��������� � ������������ � �������.\n\n������� � ������ ������������:\nEnter\t- ���������� �������� [=],\nEsc\t- ������ ��� [CE], ������ - [ON/C],\n\"<-\" (�����)\t- �������� ��������� ��������� �����,\n\"~\" (������)\t- ��������� ����� ����������� �����.\n\n����� ���������: ����� ���������, 2019 �."),
		TEXT ("Help"),
		TEXT ("Keyboard shortcuts:\nCtrl+Q\t- close the Application,\nCtrl+C\t- copy the value on the clipboard,\nCtrl+V\t- paste the value from the clipboard,\nCtrl+Alt+C\t- switch from another application to the calculator and back.\n\nKeys in the calculator mode:\nEnter\t- execute equation [=],\nEsc\t- first time [CE], second time - [ON/C],\n\"<-\" (backspace)\t- remove the last entered digit,\n\"~\" (tilde)\t- invert the sign of the value.\n\nThe application developer: Ihar Areshchankau, 2019.")
	};
	LPCTSTR msg, cap;
	int klang;
	lang = GetUserDefaultUILanguage ();
	klang = lang == 0x0419 || lang == 0x0422 || lang == 0x0423? 0: 1;
	cap = s[2 * klang];
	msg = s[2 * klang + 1];
	MessageBox (hWnd, msg, cap, MB_OK | MB_ICONINFORMATION);
}
/*====================================================*/
/* ��������� ������� �� �������� Web-����� ���������. */
/* �����: hWnd - ����������� ���� ����� ����������.   */
/*====================================================*/
static void CmdWeb (HWND hWnd)
{
	LANGID lang;
	TCHAR url[256];
	lang = GetUserDefaultUILanguage ();
	wsprintf (url, TEXT ("https://iharsw.login.by/sidetool/?lang=%x"), lang);
	ShellExecute (hWnd, NULL, url, NULL, NULL, SW_SHOWNORMAL);
}