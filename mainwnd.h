#ifndef MAINWND_H
#define MAINWND_H 1
/*--------------------------------------------------------------*/
/* ������������� ���������� ������ ��� ��������� �������� ����. */
/*--------------------------------------------------------------*/
#define HKACTIVATE 0
/*-------------------------------------------------------------*/
/* ���������� �������, ����������� � �������� ���� ����������. */
/*-------------------------------------------------------------*/
LRESULT CALLBACK MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ATOM MainWndRegisterClass (HINSTANCE hInstance);
HWND MainWndCreate (HINSTANCE hInstance, ATOM wcx, LPCTSTR lpTitle);
#endif